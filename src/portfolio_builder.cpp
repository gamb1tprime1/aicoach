#include "portfolio_builder.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

PortfolioPlan PortfolioBuilder::build(const UserProfile& u, const json& uni_req) const {
        std::string system;
    if (u.lang == "ru") {
        system =
            "Ты — консультант по поступлению в зарубежные вузы. "
            "Составь индивидуальный план портфолио: академические достижения, "
            "внеклассные активности, волонтёрство, олимпиады, research, летние школы, "
            "письма, рекомендации, дедлайны. Учитывай требования конкретных вузов. "
            "Отвечай ТОЛЬКО валидным JSON, без markdown. Начинается с '{', заканчивается на '}'. "
            "Используй ТОЛЬКО эти ключи: "
            "{\"sections\":[строки], \"action_items\":[строки], "
            "\"university_requirements\":{}, \"timeline\":[строки]}. "
            "Все элементы массивов — обычные строки. Содержимое пиши ПО-РУССКИ.";
    } else {
        system =
            "You are a study-abroad admissions consultant. "
            "Build an individual portfolio plan: academic achievements, "
            "extracurricular activities, volunteering, olympiads, research, summer schools, "
            "letters, recommendations, deadlines. Account for specific university requirements. "
            "You MUST respond ONLY with valid JSON. No markdown, no backticks, no commentary. "
            "Your response must start with '{' and end with '}'. "
            "Use ONLY these English keys exactly: "
            "{\"sections\":[strings], \"action_items\":[strings], "
            "\"university_requirements\":{}, \"timeline\":[strings]}. "
            "All array items must be plain strings, not objects.";
    }

    std::string unis;
    for (auto& x : u.universities) unis += "- " + x + "\n";

    const std::string user =
        "Target exam: " + u.target_exam + "\n" +
        "Target score: " + std::to_string(u.target_score) + "\n" +
        "Major: " + u.major + "\n" +
        "Universities:\n" + unis +
        "University requirements (JSON): " + uni_req.dump() + "\n" +
        "Weeks until deadline: " + std::to_string(u.weeks_available);

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

    PortfolioPlan p;
    if (j.is_discarded() || !j.is_object()) {
        p.sections.push_back("LLM returned non-JSON");
        p.action_items.push_back(raw.substr(0, 1000));
        p.university_requirements = uni_req;
        return p;
    }

    if (j.contains("sections")) {
        for (auto& s : j["sections"]) {
            if (s.is_string()) p.sections.push_back(s.get<std::string>());
            else if (s.is_object() && s.contains("name")) p.sections.push_back(s["name"].get<std::string>());
        }
    }
    if (j.contains("action_items")) {
        for (auto& s : j["action_items"]) {
            if (s.is_string()) p.action_items.push_back(s.get<std::string>());
            else if (s.is_object() && s.contains("name")) p.action_items.push_back(s["name"].get<std::string>());
            else if (s.is_object() && s.contains("action")) p.action_items.push_back(s["action"].get<std::string>());
        }
    }

    p.university_requirements = uni_req;
    return p;
}