/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462
*/
#pragma once

#include <string>
#include <stdexcept>
#include <vector>
#include <libpq-fe.h>

using namespace std;

// Thrown when we can't connect or the connection drops
struct DBConnectionError : public runtime_error {
    explicit DBConnectionError(const string& msg)
        : runtime_error("DBConnectionError: " + msg) {}
};

// Thrown when a query or statement fails
struct DBQueryError : public runtime_error {
    explicit DBQueryError(const string& msg)
        : runtime_error("DBQueryError: " + msg) {}
};

/**
 * PGResultGuard - RAII wrapper for PGresult*, calls PQclear() on scope exit
 * Stops us from leaking query results
 */
struct PGResultGuard {
    PGresult* res = nullptr;

    explicit PGResultGuard(PGresult* r) : res(r) {}
    ~PGResultGuard() { if (res) PQclear(res); }

    PGResultGuard(const PGResultGuard&) = delete;
    PGResultGuard& operator=(const PGResultGuard&) = delete;

    PGResultGuard(PGResultGuard&& o) noexcept : res(o.res) { o.res = nullptr; }
    PGResultGuard& operator=(PGResultGuard&& o) noexcept {
        if (this != &o) { if (res) PQclear(res); res = o.res; o.res = nullptr; }
        return *this;
    }

    operator PGresult*() const { return res; }
    PGresult* operator->() const { return res; }

    int rows() const { return res ? PQntuples(res) : 0; }   // Rows from a SELECT
    int cols() const { return res ? PQnfields(res) : 0; }   // Column count

    // Text value at (row, col), returns "" for SQL NULL
    string val(int row, int col) const {
        if (!res || PQgetisnull(res, row, col)) return "";
        return PQgetvalue(res, row, col);
    }

    const char* rawBytes(int row, int col) const {          // Raw pointer for BYTEA
        return res ? PQgetvalue(res, row, col) : nullptr;
    }
    int byteLen(int row, int col) const {                   // Byte length for BYTEA
        return res ? PQgetlength(res, row, col) : 0;
    }
};

// Connection details - reads from env vars or uses sensible defaults
struct DBConnectionParams {
    string host     = "localhost";
    string port     = "5432";
    string dbname   = "alife_sim";
    string user     = "postgres";
    string password = "";

    string toConnectionString() const;  // Build the libpq keyword=value string

    // Pull values from ALIFE_DB_HOST / PORT / NAME / USER / PASS
    // Falls back to the defaults above if a var isn't set
    static DBConnectionParams fromEnv();
};

/**
 * DBConnector - One psql connection, opens on construction, closes on destruction
 * Everything else (SaveManager, AutoSave) borrows a shared_ptr to this
 */
class DBConnector {
public:
    explicit DBConnector(const DBConnectionParams& params);
    explicit DBConnector(const string& connectionString);
    ~DBConnector();

    DBConnector(const DBConnector&) = delete;
    DBConnector& operator=(const DBConnector&) = delete;

    DBConnector(DBConnector&&) noexcept;
    DBConnector& operator=(DBConnector&&) noexcept;

    bool isConnected() const;
    void reconnect();   // Try to re-establish a lost connection

    // Run a plain SQL string — wrap result in PGResultGuard immediately
    PGresult* exec(const string& sql);

    // Parameterised query with $1..$N placeholders, all params as text
    PGresult* execParams(const string& sql, const vector<string>& params);

    // Binary params — needed for BYTEA genome data
    PGresult* execParamsBinary(const string& sql,
                                int nParams,
                                const char* const* paramValues,
                                const int* paramLengths,
                                const int* paramFormats);

    void beginTransaction();
    void commitTransaction();
    void rollbackTransaction();   // Best-effort, won't throw

    void applySchema(const string& schemaSql);          // Run a raw SQL schema string
    void applySchemaFile(const string& filePath);       // Read a .sql file and run it
    bool tableExists(const string& tableName);          // Check if table exists in public schema

    PGconn* raw() const { return m_conn; }              // Direct handle, use sparingly

private:
    PGconn*            m_conn = nullptr;
    DBConnectionParams m_params;
    string             m_connStr;

    void connect();
    void checkResult(PGresult* res, const string& context) const;
};
