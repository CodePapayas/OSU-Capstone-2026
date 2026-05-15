/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462
*/

#include "../include/save_manager.h"
#include <stdexcept>
#include <ctime>
#include <cstring>
#include <sstream>
#include <algorithm>
#include <cassert>

#ifdef ALIFE_USE_ZLIB
  #include <zlib.h>
#endif

using namespace std;

SaveManager::SaveManager(shared_ptr<DBConnector> db)
    : m_db(move(db))
{
    if (!m_db) throw invalid_argument("SaveManager: null DBConnector");
}

void SaveManager::initSchema(const string& schemaFilePath) {
    if (!m_db->tableExists("simulation_saves"))
        m_db->applySchemaFile(schemaFilePath);
}

bool SaveManager::compressionEnabled() {
#ifdef ALIFE_USE_ZLIB
    return true;
#else
    return false;
#endif
}

vector<uint8_t> SaveManager::compressBytes(const vector<uint8_t>& data) {
    if (data.empty()) return {};

#ifdef ALIFE_USE_ZLIB
    uLongf destLen = compressBound(static_cast<uLong>(data.size()));  // zlib upper-bound
    vector<uint8_t> out(destLen);

    int rc = compress2(
        out.data(), &destLen,
        reinterpret_cast<const Bytef*>(data.data()),
        static_cast<uLong>(data.size()),
        Z_BEST_COMPRESSION
    );
    if (rc != Z_OK)
        throw runtime_error("SaveManager::compressBytes: zlib error " + to_string(rc));

    out.resize(destLen);
    return out;
#else
    return data;  // No zlib — store uncompressed
#endif
}

vector<uint8_t> SaveManager::decompressBytes(const vector<uint8_t>& data,
                                               size_t expectedUncompressedSize) {
    if (data.empty()) return {};

#ifdef ALIFE_USE_ZLIB
    // Start at the hint size, or 4x compressed size if no hint was given
    uLongf destLen = static_cast<uLongf>(
        expectedUncompressedSize > 0 ? expectedUncompressedSize : data.size() * 4);

    vector<uint8_t> out;
    while (true) {
        out.resize(destLen);
        int rc = uncompress(
            out.data(), &destLen,
            reinterpret_cast<const Bytef*>(data.data()),
            static_cast<uLong>(data.size())
        );
        if (rc == Z_OK) {
            out.resize(destLen);
            return out;
        } else if (rc == Z_BUF_ERROR) {
            destLen *= 2;   // Buffer too small — double and retry
        } else {
            throw runtime_error("SaveManager::decompressBytes: zlib error " +
                                to_string(rc));
        }
    }
#else
    return data;
#endif
}

// ResourceType <-> int conversions used when reading/writing resource rows
string SaveManager::resourceTypeToString(ResourceType t) {
    switch (t) {
        case ResourceType::FOOD:    return "0";
        case ResourceType::WATER:   return "1";
        case ResourceType::MINERAL: return "2";
        case ResourceType::PLANT:   return "3";
        case ResourceType::CUSTOM:  return "4";
        default:                    return "0";
    }
}

ResourceType SaveManager::resourceTypeFromInt(int v) {
    switch (v) {
        case 0: return ResourceType::FOOD;
        case 1: return ResourceType::WATER;
        case 2: return ResourceType::MINERAL;
        case 3: return ResourceType::PLANT;
        case 4: return ResourceType::CUSTOM;
        default: return ResourceType::FOOD;
    }
}

int SaveManager::save(const SimulationSavePayload& payload) {
    return saveInternal(payload, /*isAutoSave=*/false);
}

int SaveManager::saveInternal(const SimulationSavePayload& payload, bool isAutoSave) {
    m_db->beginTransaction();
    try {
        int saveId = upsertSaveSlot(payload, isAutoSave);

        saveAgents   (saveId, payload.agents);
        saveResources(saveId, payload.resources);
        saveEnvironment(saveId, payload);

        if (payload.stateHistory)
            saveHistory(saveId, payload.stateHistory);

        m_db->commitTransaction();
        return saveId;
    } catch (...) {
        m_db->rollbackTransaction();
        throw;
    }
}

int SaveManager::upsertSaveSlot(const SimulationSavePayload& payload, bool isAutoSave) {
    // Tally up fitness and energy totals for the summary row
    double totalEnergy = 0.0;
    double totalFitness = 0.0;
    for (const auto& a : payload.agents) {
        totalFitness += a.fitness;
    }
    double avgFitness = payload.agents.empty() ? 0.0
                       : totalFitness / static_cast<double>(payload.agents.size());

    for (const auto* r : payload.resources) {
        if (r) totalEnergy += r->getEnergyValue();
    }

    // Delete any existing slot with this name (FK cascade removes all child rows)
    { PGResultGuard del(m_db->execParams("DELETE FROM simulation_saves WHERE slot_name = $1", {payload.slotName})); }

    PGResultGuard ins(m_db->execParams(
        "INSERT INTO simulation_saves "
        "(slot_name, description, tick, real_timestamp, agent_count, resource_count, "
        " total_energy, average_fitness, is_auto_save, compressed) "
        "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10) "
        "RETURNING id",
        {
            payload.slotName,
            payload.description,
            to_string(payload.tick),
            to_string(payload.realTimestamp),
            to_string(payload.agents.size()),
            to_string(payload.resources.size()),
            to_string(totalEnergy),
            to_string(avgFitness),
            isAutoSave ? "true" : "false",
            compressionEnabled() ? "true" : "false"
        }
    ));

    return stoi(ins.val(0, 0));
}

void SaveManager::saveAgents(int saveId, const vector<AgentSaveData>& agents) {
    if (agents.empty()) return;

    const string sql =
        "INSERT INTO simulation_agent_states "
        "(save_id, agent_id, pos_x, pos_y, energy, max_energy, age, "
        " energy_gained, energy_spent, offspring, fitness, genome_data, genome_length) "
        "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13)";

    for (const auto& a : agents) {
        // Compress genome before storing; track uncompressed size for decompression hint
        vector<uint8_t> compressed;
        size_t uncompressedLen = 0;
        if (!a.genomeBytes.empty()) {
            compressed     = compressBytes(a.genomeBytes);
            uncompressedLen = a.genomeBytes.size();
        }

        // $12 (genome_data) is sent as binary BYTEA, everything else is text
        const char* paramVals[13];
        int         paramLens[13];
        int         paramFmts[13];

        string p1  = to_string(saveId);         string p2  = to_string(a.agentId);
        string p3  = to_string(a.posX);         string p4  = to_string(a.posY);
        string p5  = to_string(a.energy);       string p6  = to_string(a.maxEnergy);
        string p7  = to_string(a.age);          string p8  = to_string(a.energyGained);
        string p9  = to_string(a.energySpent);  string p10 = to_string(a.offspring);
        string p11 = to_string(a.fitness);      string p13 = to_string(uncompressedLen);

        paramVals[0]=p1.c_str();  paramLens[0]=0; paramFmts[0]=0;
        paramVals[1]=p2.c_str();  paramLens[1]=0; paramFmts[1]=0;
        paramVals[2]=p3.c_str();  paramLens[2]=0; paramFmts[2]=0;
        paramVals[3]=p4.c_str();  paramLens[3]=0; paramFmts[3]=0;
        paramVals[4]=p5.c_str();  paramLens[4]=0; paramFmts[4]=0;
        paramVals[5]=p6.c_str();  paramLens[5]=0; paramFmts[5]=0;
        paramVals[6]=p7.c_str();  paramLens[6]=0; paramFmts[6]=0;
        paramVals[7]=p8.c_str();  paramLens[7]=0; paramFmts[7]=0;
        paramVals[8]=p9.c_str();  paramLens[8]=0; paramFmts[8]=0;
        paramVals[9]=p10.c_str(); paramLens[9]=0; paramFmts[9]=0;
        paramVals[10]=p11.c_str();paramLens[10]=0;paramFmts[10]=0;

        if (!compressed.empty()) {
            paramVals[11] = reinterpret_cast<const char*>(compressed.data());
            paramLens[11] = static_cast<int>(compressed.size());
            paramFmts[11] = 1;  // binary
        } else {
            paramVals[11] = nullptr; paramLens[11] = 0; paramFmts[11] = 1;
        }

        paramVals[12]=p13.c_str();paramLens[12]=0;paramFmts[12]=0;

        PGResultGuard r(m_db->execParamsBinary(sql, 13, paramVals, paramLens, paramFmts));
    }
}

void SaveManager::saveResources(int saveId, const vector<ResourceNode*>& resources) {
    if (resources.empty()) return;

    const string sql =
        "INSERT INTO simulation_resource_states "
        "(save_id, resource_id, pos_x, pos_y, resource_type, "
        " current_energy, max_energy, renewable, regen_rate) "
        "VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9)";

    for (const auto* r : resources) {
        if (!r) continue;
        Position pos = r->getPosition();
        PGResultGuard ins(m_db->execParams(sql, {
            to_string(saveId),
            to_string(r->getID()),
            to_string(pos.x),
            to_string(pos.y),
            resourceTypeToString(r->getType()),
            to_string(r->getEnergyValue()),
            to_string(r->getMaxEnergy()),
            r->isRenewable() ? "true" : "false",
            "0.0"   // regen_rate — placeholder until ResourceNode exposes it
        }));
    }
}

void SaveManager::saveEnvironment(int saveId, const SimulationSavePayload& payload) {
    const string sql =
        "INSERT INTO simulation_environment_state "
        "(save_id, world_width, world_height, total_energy, extra_data) "
        "VALUES ($1,$2,$3,$4,$5)";

    PGResultGuard ins(m_db->execParams(sql, {
        to_string(saveId),
        to_string(payload.worldWidth),
        to_string(payload.worldHeight),
        to_string(payload.totalEnergy),
        "{}"   // extensible JSON for future env fields
    }));
}

void SaveManager::saveHistory(int saveId, const CircularBuffer<SimulationState>* hist) {
    if (!hist || hist->empty()) return;

    const string sql =
        "INSERT INTO simulation_state_history "
        "(save_id, tick, real_timestamp, agent_count, total_energy, "
        " total_resources, avg_agent_energy, avg_fitness) "
        "VALUES ($1,$2,$3,$4,$5,$6,$7,$8)";

    for (size_t i = 0; i < hist->size(); ++i) {
        const SimulationState& s = hist->get(i);
        PGResultGuard ins(m_db->execParams(sql, {
            to_string(saveId),
            to_string(s.tick),
            to_string(s.timestamp),
            to_string(s.agentCount),
            to_string(s.totalEnergy),
            to_string(s.totalResources),
            to_string(s.averageAgentEnergy),
            to_string(s.averageFitness)
        }));
    }
}

bool SaveManager::load(const string& slotName, SimulationSavePayload& out) {
    PGResultGuard meta(m_db->execParams(
        "SELECT id, slot_name, description, tick, real_timestamp, "
        "       agent_count, resource_count, total_energy "
        "FROM   simulation_saves "
        "WHERE  slot_name = $1",
        {slotName}
    ));

    if (meta.rows() == 0) return false;  // Slot doesn't exist

    int saveId         = stoi  (meta.val(0, 0));
    out.slotName       = meta.val(0, 1);
    out.description    = meta.val(0, 2);
    out.tick           = stoull(meta.val(0, 3));
    out.realTimestamp  = stod  (meta.val(0, 4));
    out.totalEnergy    = stod  (meta.val(0, 7));

    {
        PGResultGuard env(m_db->execParams(
            "SELECT world_width, world_height, total_energy "
            "FROM   simulation_environment_state WHERE save_id = $1",
            {to_string(saveId)}
        ));
        if (env.rows() > 0) {
            out.worldWidth  = stoi(env.val(0, 0));
            out.worldHeight = stoi(env.val(0, 1));
            out.totalEnergy = stod(env.val(0, 2));
        }
    }

    loadAgents(saveId, out.agents);
    loadResources(saveId, out);
    return true;
}

// PostgreSQL text-mode BYTEA comes back as "\xdeadbeef" — decode to raw bytes
static vector<uint8_t> pgHexDecode(const char* p, int textLen) {
    vector<uint8_t> out;
    if (!p || textLen < 2 || p[0] != '\\' || p[1] != 'x') return out;
    out.reserve((textLen - 2) / 2);
    auto h = [](char c) -> uint8_t {
        if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<uint8_t>(c - 'a' + 10);
        if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
        return 0;
    };
    for (int i = 2; i + 1 < textLen; i += 2)
        out.push_back(static_cast<uint8_t>((h(p[i]) << 4) | h(p[i + 1])));
    return out;
}

void SaveManager::loadAgents(int saveId, vector<AgentSaveData>& outAgents) {
    PGResultGuard res(m_db->execParams(
        "SELECT agent_id, pos_x, pos_y, energy, max_energy, age, "
        "       energy_gained, energy_spent, offspring, fitness, "
        "       genome_data, genome_length "
        "FROM   simulation_agent_states "
        "WHERE  save_id = $1 ORDER BY agent_id",
        {to_string(saveId)}
    ));

    outAgents.reserve(static_cast<size_t>(res.rows()));
    for (int row = 0; row < res.rows(); ++row) {
        AgentSaveData a;
        a.agentId      = stoull(res.val(row, 0));
        a.posX         = stoi  (res.val(row, 1));
        a.posY         = stoi  (res.val(row, 2));
        a.energy       = stod  (res.val(row, 3));
        a.maxEnergy    = stod  (res.val(row, 4));
        a.age          = stoull(res.val(row, 5));
        a.energyGained = stod  (res.val(row, 6));
        a.energySpent  = stod  (res.val(row, 7));
        a.offspring    = stoul (res.val(row, 8));
        a.fitness      = stod  (res.val(row, 9));

        // Decode hex BYTEA text, then decompress
        int blobLen = res.byteLen(row, 10);
        if (blobLen > 0 && res.rawBytes(row, 10)) {
            size_t hint = 0;
            string lenStr = res.val(row, 11);
            if (!lenStr.empty()) hint = stoull(lenStr);

            vector<uint8_t> compressed = pgHexDecode(res.rawBytes(row, 10), blobLen);
            if (!compressed.empty())
                a.genomeBytes = decompressBytes(compressed, hint);
        }

        outAgents.push_back(move(a));
    }
}

void SaveManager::loadResources(int saveId, SimulationSavePayload& out) {
    PGResultGuard res(m_db->execParams(
        "SELECT resource_id, pos_x, pos_y, resource_type, "
        "       current_energy, max_energy, renewable "
        "FROM   simulation_resource_states "
        "WHERE  save_id = $1 ORDER BY resource_id",
        {to_string(saveId)}
    ));

    // Heap-allocate each ResourceNode; caller owns these (pass to ResourceManager)
    for (int row = 0; row < res.rows(); ++row) {
        Position pos(stoi(res.val(row, 1)), stoi(res.val(row, 2)));
        ResourceType type = resourceTypeFromInt(stoi(res.val(row, 3)));
        double curEnergy  = stod(res.val(row, 4));
        double maxEnergy  = stod(res.val(row, 5));
        bool renewable    = (res.val(row, 6) == "t" || res.val(row, 6) == "true");

        // Ctor sets energy to max, so drain down to the saved level
        auto* node = new ResourceNode(pos, type, maxEnergy, renewable);
        if (curEnergy < maxEnergy)
            node->consume(maxEnergy - curEnergy);
        out.resources.push_back(node);
    }
}

bool SaveManager::deleteSave(const string& slotName) {
    PGResultGuard r(m_db->execParams(
        "DELETE FROM simulation_saves WHERE slot_name = $1",
        {slotName}
    ));
    return PQcmdTuples(r) && string(PQcmdTuples(r)) != "0";
}

// Helper to map a result row into a SaveSlotInfo struct
static SaveSlotInfo rowToSlotInfo(const PGResultGuard& r, int row) {
    SaveSlotInfo info;
    info.id            = stoi  (r.val(row, 0));
    info.slotName      = r.val (row, 1);
    info.description   = r.val (row, 2);
    info.tick          = stoull(r.val(row, 3));
    info.realTimestamp = stod  (r.val(row, 4));
    info.agentCount    = stoi  (r.val(row, 5));
    info.resourceCount = stoi  (r.val(row, 6));
    info.totalEnergy   = stod  (r.val(row, 7));
    info.avgFitness    = stod  (r.val(row, 8));
    info.isAutoSave    = (r.val(row, 9) == "t" || r.val(row, 9) == "true");
    info.createdAt     = r.val (row, 10);
    return info;
}

static const char* kListSql =
    "SELECT id, slot_name, description, tick, real_timestamp, "
    "       agent_count, resource_count, total_energy, average_fitness, "
    "       is_auto_save, created_at::text "
    "FROM   simulation_saves "
    "WHERE  is_auto_save = $1 "
    "ORDER  BY created_at DESC";

vector<SaveSlotInfo> SaveManager::listSaves() const {
    PGResultGuard r(m_db->execParams(
        "SELECT id, slot_name, description, tick, real_timestamp, "
        "       agent_count, resource_count, total_energy, average_fitness, "
        "       is_auto_save, created_at::text "
        "FROM   simulation_saves ORDER BY created_at DESC",
        {}
    ));
    vector<SaveSlotInfo> out;
    out.reserve(static_cast<size_t>(r.rows()));
    for (int i = 0; i < r.rows(); ++i)
        out.push_back(rowToSlotInfo(r, i));
    return out;
}

vector<SaveSlotInfo> SaveManager::listAutoSaves() const {
    PGResultGuard r(m_db->execParams(kListSql, {"true"}));
    vector<SaveSlotInfo> out;
    out.reserve(static_cast<size_t>(r.rows()));
    for (int i = 0; i < r.rows(); ++i)
        out.push_back(rowToSlotInfo(r, i));
    return out;
}

bool SaveManager::slotExists(const string& slotName) const {
    PGResultGuard r(m_db->execParams(
        "SELECT 1 FROM simulation_saves WHERE slot_name = $1",
        {slotName}
    ));
    return r.rows() > 0;
}
