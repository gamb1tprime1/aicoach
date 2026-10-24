#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "llm_client.hpp"

class ExamGenerator {
public:
    ExamGenerator(const LLMClient& llm) : llm_(llm) {}

    // Writing Task 2 (Essay) — IELTS Academic/General, TOEFL, SAT
    nlohmann::json generateEssay(const std::string& exam,
                                 const std::string& section,
                                 const std::string& difficulty,
                                 const std::string& lang) const;

private:
    const LLMClient& llm_;
};