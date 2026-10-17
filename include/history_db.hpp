#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class HistoryDB {
public:
    HistoryDB(const std::string& db_path);
    ~HistoryDB();

    void log(const std::string& user_id,
             const std::string& endpoint,
             const nlohmann::json& request,
             const nlohmann::json& response);

    std::vector<nlohmann::json> recent(const std::string& user_id, int limit = 20);

private:
    void* db_ = nullptr;
};