#pragma once
#include "models.hpp"
#include "llm_client.hpp"
#include <nlohmann/json.hpp>

class PortfolioBuilder {
public:
    PortfolioBuilder(const LLMClient& llm) : llm_(llm) {}
    PortfolioPlan build(const UserProfile& u, const nlohmann::json& uni_req) const;
private:
    const LLMClient& llm_;
};
