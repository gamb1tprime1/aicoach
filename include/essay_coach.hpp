#pragma once
#include "models.hpp"
#include "llm_client.hpp"
#include <nlohmann/json.hpp>

class EssayCoach {
public:
    EssayCoach(const LLMClient& llm) : llm_(llm) {}
    nlohmann::json analyze(const EssayDraft& e) const;
private:
    const LLMClient& llm_;
};
