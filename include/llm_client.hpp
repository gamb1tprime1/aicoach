#pragma once
#include <string>

class LLMClient {
public:
    LLMClient(std::string api_url, std::string api_key, std::string model);
    std::string chat(const std::string& system, const std::string& user) const;
private:
    std::string api_url_;
    std::string api_key_;
    std::string model_;
};
