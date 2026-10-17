#include "essay_coach.hpp"

using json = nlohmann::json;

json EssayCoach::analyze(const EssayDraft& e) const {
    std::string system;
    if (e.lang == "ru") {
        system =
            "Ты — наставник по академическому письму и admissions-эссе. "
            "Ты НИКОГДА не пишешь эссе за пользователя и не переписываешь его текст. "
            "Сохраняй индивидуальный голос автора. "
            "Дай: (1) план, (2) 5-8 вопросов для рефлексии, "
            "(3) чек-лист под конкретный университет, "
            "(4) 3 слабых места черновика БЕЗ переписывания, "
            "(5) ресурсы. "
            "Отвечай ТОЛЬКО валидным JSON, без markdown. Ответ начинается с '{' и заканчивается на '}'. "
            "Используй ТОЛЬКО эти ключи: "
            "{\"outline\":[строки], \"reflection_questions\":[строки], "
            "\"university_checklist\":[строки], \"weak_points\":[строки], "
            "\"word_budget\":{}, \"tone_notes\":[строки], \"resources\":[строки]}. "
            "Все элементы массивов — обычные строки, не объекты. "
            "Содержимое пиши ПО-РУССКИ.";
    } else {
        system =
            "You are an academic writing and admissions essay coach. "
            "You NEVER write the essay for the user and NEVER rewrite their text. "
            "Preserve the author's individual voice. "
            "Give: (1) an outline, (2) 5-8 reflection questions, "
            "(3) a university-specific checklist, "
            "(4) 3 concrete weak points of the draft WITHOUT rewriting text for the author, "
            "(5) resources for self-improvement. "
            "You MUST respond ONLY with valid JSON. No markdown, no backticks, no commentary. "
            "Your response must start with '{' and end with '}'. "
            "Use ONLY these English keys exactly as written: "
            "{\"outline\":[strings], \"reflection_questions\":[strings], "
            "\"university_checklist\":[strings], \"weak_points\":[strings], "
            "\"word_budget\":{}, \"tone_notes\":[strings], \"resources\":[strings]}. "
            "All array items must be plain strings, not objects.";
    }

    const std::string user =
        "University: " + e.university + "\n" +
        "Essay type: " + e.essay_type + "\n" +
        "Prompt: " + e.prompt + "\n" +
        "User's draft:\n\"\"\"\n" + e.draft + "\n\"\"\"\n\n" +
        "Return ONLY JSON with the exact keys specified. No other text.";

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
    if (j.is_discarded() || !j.is_object()) {
        return json{
            {"error", "LLM returned non-JSON"},
            {"raw", raw.substr(0, 1500)}
        };
    }
    return j;
}