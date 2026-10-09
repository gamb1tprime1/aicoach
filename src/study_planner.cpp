#include "study_planner.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

StudyPlan StudyPlanner::make_plan(const UserProfile& u, const std::vector<Task>& weak) const {
    const std::string system =
        "You are an IELTS/SAT preparation methodologist. Create a weekly plan "
        "that brings the student from their current level to the target score "
        "(up to 9.0 IELTS). Account for native language, hours per week, and weak spots. "
        "You MUST respond ONLY with valid JSON. No markdown, no backticks, no commentary. "
        "Your response must start with '{' and end with '}'. "
        "Use ONLY these English keys exactly: "
        "{\"weekly_goals\":[strings], \"daily_tasks\":[strings], "
        "\"resources\":[strings], \"rationale\":string}. "
        "All array items must be plain strings, not objects.";

    json weak_json = json::array();
    for (auto& t : weak) weak_json.push_back({{"id", t.id}, {"section", t.section}});

    const std::string user =
        "Exam: " + u.target_exam + "\n" +
        "Target: " + std::to_string(u.target_score) + "\n" +
        "Current level: " + u.current_level + "\n" +
        "Native language: " + u.native_language + "\n" +
        "Weeks: " + std::to_string(u.weeks_available) + "\n" +
        "Hours per week: " + std::to_string(u.hours_per_week) + "\n" +
        "Weak sections: " + weak_json.dump();

    std::string raw = llm_.chat(system, user);

    auto obj_start = raw.find('{');
    auto arr_start = raw.find('[');
    std::size_t start = std::string::npos;
    std::size_t end   = std::string::npos;

    if (obj_start != std::string::npos && (arr_start == std::string::npos || obj_start < arr_start)) {
        start = obj_start;
        end   = raw.rfind('}');
    } else if (arr_start != std::string::npos) {
        start = arr_start;
        end   = raw.rfind(']');
    }
    if (start != std::string::npos && end != std::string::npos && end > start) {
        raw = raw.substr(start, end - start + 1);
    }

    auto j = json::parse(raw, nullptr, false);

    StudyPlan p;
    if (j.is_discarded() || !j.is_object()) {
        p.rationale = "LLM returned non-JSON";
        p.weekly_goals.push_back(raw.substr(0, 1000));
        return p;
    }

    auto push_string = [](std::vector<std::string>& v, const json& item) {
        if (item.is_string()) {
            v.push_back(item.get<std::string>());
        } else if (item.is_object()) {
            for (auto it = item.begin(); it != item.end(); ++it) {
                if (it.value().is_string()) {
                    v.push_back(it.value().get<std::string>());
                    break;
                }
            }
        }
    };

    if (j.contains("weekly_goals") && j["weekly_goals"].is_array()) {
        for (auto& s : j["weekly_goals"]) push_string(p.weekly_goals, s);
    }
    if (j.contains("daily_tasks") && j["daily_tasks"].is_array()) {
        for (auto& s : j["daily_tasks"]) push_string(p.daily_tasks, s);
    }
    if (j.contains("resources") && j["resources"].is_array()) {
        for (auto& s : j["resources"]) push_string(p.resources, s);
    }
    if (j.contains("rationale")) {
        if (j["rationale"].is_string()) p.rationale = j["rationale"].get<std::string>();
        else if (j["rationale"].is_object()) {
            for (auto it = j["rationale"].begin(); it != j["rationale"].end(); ++it) {
                if (it.value().is_string()) { p.rationale = it.value().get<std::string>(); break; }
            }
        }
    }

    return p;
}