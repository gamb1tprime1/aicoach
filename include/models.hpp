#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Task {
    std::string id;
    std::string exam;
    std::string section;
    std::string difficulty;
    std::string prompt;
    nlohmann::json meta;
};

struct UserProfile {
    std::string user_id;
    std::string target_exam;
    double target_score = 7.0;
    std::string native_language;
    std::string current_level;
    std::vector<std::string> universities;
    std::string major;
    int weeks_available = 12;
    double hours_per_week = 6.0;
};

struct EssayDraft {
    std::string user_id;
    std::string university;
    std::string prompt;
    std::string draft;
    std::string essay_type;
};

struct StudyPlan {
    std::vector<std::string> weekly_goals;
    std::vector<std::string> daily_tasks;
    std::vector<std::string> resources;
    std::string rationale;
};

struct PortfolioPlan {
    std::vector<std::string> sections;
    std::vector<std::string> action_items;
    nlohmann::json university_requirements;
};
