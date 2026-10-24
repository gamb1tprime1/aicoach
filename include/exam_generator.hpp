#pragma once
#include <string>
#include <nlohmann/json.hpp>
#include "llm_client.hpp"

class ExamGenerator {
public:
    ExamGenerator(const LLMClient& llm) : llm_(llm) {}

    nlohmann::json generateEssay(const std::string& exam,
                                 const std::string& section,
                                 const std::string& difficulty,
                                 const std::string& lang) const;

    nlohmann::json generateReading(const std::string& exam,
                                   const std::string& section,
                                   const std::string& difficulty,
                                   const std::string& lang) const;

    nlohmann::json generateSpeaking(const std::string& exam,
                                    const std::string& section,
                                    const std::string& difficulty,
                                    const std::string& lang) const;

private:
    const LLMClient& llm_;
};
