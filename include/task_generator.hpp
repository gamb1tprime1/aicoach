#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "llm_client.hpp"
#include "tasks_repo.hpp"

class TaskGenerator {
public:
    TaskGenerator(const LLMClient& llm, const TasksRepo& repo)
        : llm_(llm), repo_(repo) {}

    // Генерирует count новых заданий по образцу existing.
    // Возвращает json::array. При ошибке — пустой массив.
    nlohmann::json generate(const std::string& exam,
                            const std::string& section,
                            const std::string& difficulty,
                            int count,
                            const std::string& lang = "en") const;

private:
    const LLMClient& llm_;
    const TasksRepo& repo_;
};