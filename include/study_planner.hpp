#pragma once
#include "models.hpp"
#include "llm_client.hpp"
#include <vector>

class StudyPlanner {
public:
    StudyPlanner(const LLMClient& llm) : llm_(llm) {}
    StudyPlan make_plan(const UserProfile& u, const std::vector<Task>& weak) const;
private:
    const LLMClient& llm_;
};
