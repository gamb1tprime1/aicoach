#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Task {
    std::string id, exam, section, difficulty, prompt;
    nlohmann::json meta;
};

struct UserProfile {
    std::string user_id;
    std::string target_exam = "IELTS";
    double target_score = 7.0;
    std::string native_language, current_level, major;
    std::vector<std::string> universities;
    int weeks_available = 12;
    double hours_per_week = 6.0;
    std::string lang = "en";
};

struct EssayDraft {
    std::string user_id, university, prompt, draft, essay_type;
    std::string lang = "en";
};

struct StudyPlan {
    std::vector<std::string> weekly_goals, daily_tasks, resources;
    std::string rationale;
};

struct PortfolioPlan {
    std::vector<std::string> sections, action_items;
    nlohmann::json university_requirements;
};