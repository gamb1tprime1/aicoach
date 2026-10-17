#include "history_db.hpp"
#include <sqlite3.h>
#include <stdexcept>
#include <ctime>

HistoryDB::HistoryDB(const std::string& db_path) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("cannot open db: " + db_path);
    }
    db_ = db;

    const char* schema =
        "CREATE TABLE IF NOT EXISTS history ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  user_id TEXT NOT NULL,"
        "  endpoint TEXT NOT NULL,"
        "  request TEXT NOT NULL,"
        "  response TEXT NOT NULL,"
        "  created_at INTEGER NOT NULL"
        ");"
        "CREATE INDEX IF NOT EXISTS idx_user ON history(user_id);";

    char* err = nullptr;
    if (sqlite3_exec(db, schema, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("schema error: " + msg);
    }
}

HistoryDB::~HistoryDB() {
    if (db_) sqlite3_close(static_cast<sqlite3*>(db_));
}

void HistoryDB::log(const std::string& user_id,
                    const std::string& endpoint,
                    const nlohmann::json& request,
                    const nlohmann::json& response) {
    if (!db_) return;

    sqlite3* db = static_cast<sqlite3*>(db_);
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "INSERT INTO history (user_id, endpoint, request, response, created_at) "
        "VALUES (?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    std::string req_s  = request.dump();
    std::string resp_s = response.dump();
    int64_t now = static_cast<int64_t>(std::time(nullptr));

    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, endpoint.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, req_s.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, resp_s.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 5, now);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<nlohmann::json> HistoryDB::recent(const std::string& user_id, int limit) {
    std::vector<nlohmann::json> out;
    if (!db_) return out;

    sqlite3* db = static_cast<sqlite3*>(db_);
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT endpoint, request, response, created_at FROM history "
        "WHERE user_id = ? ORDER BY id DESC LIMIT ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return out;

    sqlite3_bind_text(stmt, 1, user_id.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, limit);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        nlohmann::json item;
        item["endpoint"]   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        item["request"]    = nlohmann::json::parse(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)), nullptr, false);
        item["response"]   = nlohmann::json::parse(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)), nullptr, false);
        item["created_at"] = sqlite3_column_int64(stmt, 3);
        out.push_back(std::move(item));
    }
    sqlite3_finalize(stmt);
    return out;
}