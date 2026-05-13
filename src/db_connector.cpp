/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462
*/

#include "../include/db_connector.h"

#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;

// Build the libpq keyword=value connection string
string DBConnectionParams::toConnectionString() const {
    string s;
    if (!host.empty())
        s += "host=" + host + " ";
    s += "port="   + port   + " ";
    s += "dbname=" + dbname + " ";
    s += "user="   + user   + " ";
    if (!password.empty())
        s += "password=" + password + " ";
    s += "connect_timeout=10";
    return s;
}

static string envOr(const char* varName, const string& fallback) {
    const char* val = getenv(varName);
    return val ? string(val) : fallback;  // Use env var if set (even empty), else default
}

DBConnectionParams DBConnectionParams::fromEnv() {
    DBConnectionParams p;
    p.host     = envOr("ALIFE_DB_HOST", p.host);
    p.port     = envOr("ALIFE_DB_PORT", p.port);
    p.dbname   = envOr("ALIFE_DB_NAME", p.dbname);
    p.user     = envOr("ALIFE_DB_USER", p.user);
    p.password = envOr("ALIFE_DB_PASS", p.password);
    return p;
}

// Open the connection on construction
DBConnector::DBConnector(const DBConnectionParams& params)
    : m_params(params)
    , m_connStr(params.toConnectionString())
{
    connect();
}

DBConnector::DBConnector(const string& connectionString)
    : m_connStr(connectionString)
{
    connect();
}

DBConnector::~DBConnector() {
    if (m_conn) {
        PQfinish(m_conn);
        m_conn = nullptr;
    }
}

DBConnector::DBConnector(DBConnector&& o) noexcept
    : m_conn(o.m_conn)
    , m_params(move(o.m_params))
    , m_connStr(move(o.m_connStr))
{
    o.m_conn = nullptr;
}

DBConnector& DBConnector::operator=(DBConnector&& o) noexcept {
    if (this != &o) {
        if (m_conn) PQfinish(m_conn);
        m_conn    = o.m_conn;
        m_params  = move(o.m_params);
        m_connStr = move(o.m_connStr);
        o.m_conn  = nullptr;
    }
    return *this;
}

void DBConnector::connect() {
    m_conn = PQconnectdb(m_connStr.c_str());
    if (!m_conn || PQstatus(m_conn) != CONNECTION_OK) {
        string err = m_conn ? PQerrorMessage(m_conn) : "unknown error";
        if (m_conn) { PQfinish(m_conn); m_conn = nullptr; }
        throw DBConnectionError(err);
    }
}

bool DBConnector::isConnected() const {
    return m_conn && PQstatus(m_conn) == CONNECTION_OK;
}

void DBConnector::reconnect() {
    if (!m_conn) { connect(); return; }
    PQreset(m_conn);   // libpq built-in reconnect
    if (PQstatus(m_conn) != CONNECTION_OK)
        throw DBConnectionError(PQerrorMessage(m_conn));
}

// Validates a PGresult*, throws on error and cleans up
void DBConnector::checkResult(PGresult* res, const string& context) const {
    if (!res)
        throw DBQueryError(context + ": null result");

    ExecStatusType status = PQresultStatus(res);
    if (status != PGRES_COMMAND_OK && status != PGRES_TUPLES_OK) {
        string err = PQresultErrorMessage(res);
        PQclear(res);
        throw DBQueryError(context + ": " + err);
    }
}

PGresult* DBConnector::exec(const string& sql) {
    if (!isConnected()) reconnect();
    PGresult* res = PQexec(m_conn, sql.c_str());
    checkResult(res, sql.substr(0, 80));
    return res;
}

PGresult* DBConnector::execParams(const string& sql, const vector<string>& params) {
    if (!isConnected()) reconnect();

    vector<const char*> vals;
    vals.reserve(params.size());
    for (const auto& p : params) vals.push_back(p.c_str());

    PGresult* res = PQexecParams(
        m_conn,
        sql.c_str(),
        static_cast<int>(vals.size()),
        nullptr,    // server infers types
        vals.data(),
        nullptr,    // lengths (null-terminated text mode)
        nullptr,    // formats (all text)
        0           // result format: text
    );
    checkResult(res, sql.substr(0, 80));
    return res;
}

PGresult* DBConnector::execParamsBinary(const string& sql,
                                         int nParams,
                                         const char* const* paramValues,
                                         const int* paramLengths,
                                         const int* paramFormats) {
    if (!isConnected()) reconnect();
    PGresult* res = PQexecParams(
        m_conn, sql.c_str(), nParams,
        nullptr, paramValues, paramLengths, paramFormats,
        0   // text result format
    );
    checkResult(res, sql.substr(0, 80));
    return res;
}

void DBConnector::beginTransaction()    { PGResultGuard g(exec("BEGIN")); }
void DBConnector::commitTransaction()   { PGResultGuard g(exec("COMMIT")); }

void DBConnector::rollbackTransaction() {
    // Best-effort — don't throw if rollback itself fails
    PGresult* res = PQexec(m_conn, "ROLLBACK");
    if (res) PQclear(res);
}

void DBConnector::applySchema(const string& schemaSql) {
    PGResultGuard g(exec(schemaSql));
}

void DBConnector::applySchemaFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open())
        throw runtime_error("applySchemaFile: cannot open " + filePath);
    ostringstream ss;
    ss << file.rdbuf();
    applySchema(ss.str());
}
