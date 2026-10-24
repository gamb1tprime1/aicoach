#include "exam_generator.hpp"
#include <sstream>

using json = nlohmann::json;

json ExamGenerator::generateEssay(const std::string& exam,
                                  const std::string& section,
                                  const std::string& difficulty,
                                  const std::string& lang) const {
    std::string system;
    if (lang == "ru") {
        system =
            "Ты — составитель заданий для экзамена IELTS/TOEFL/SAT. "
            "Твоя задача — придумать ОДНО новое эссе-задание в стиле официального экзамена. "
            "Отвечай ТОЛЬКО валидным JSON-объектом. Ответ начинается с '{' и заканчивается на '}'. "
            "Без markdown, без ```, без пояснений. "
            "Ключи: "
            "{\"prompt\":\"полный текст задания на английском\", "
            "\"min_words\":число, "
            "\"time_minutes\":число, "
            "\"topic\":\"краткая тема на английском\", "
            "\"tips\":[\"совет1\", \"совет2\", \"совет3\"]}. "
            "Поле prompt — на английском (экзамен на английском). "
            "Поле tips — строго на русском. НЕ используй китайский, японский или другие языки. "
            "Только русский текст в tips.";
    } else {
        system =
            "You are an IELTS/TOEFL/SAT task creator. "
            "Invent ONE new essay prompt in the style of the official exam. "
            "Respond ONLY with a valid JSON object. Start with '{' and end with '}'. "
            "No markdown, no backticks, no commentary. "
            "Keys: "
            "{\"prompt\":\"full task text in English\", "
            "\"min_words\":number, "
            "\"time_minutes\":number, "
            "\"topic\":\"short topic in English\", "
            "\"tips\":[\"tip1\", \"tip2\", \"tip3\"]}.";
            " All text must be in English only. Do NOT use Chinese, Japanese, or any other language.";
    }

    std::ostringstream user;
    user << "Exam: " << exam << "\n";
    user << "Section: " << section << "\n";
    user << "Difficulty: " << difficulty << "\n";
    user << "\nGenerate ONE essay prompt. Return ONLY the JSON object.";

    std::string raw;
    try {
        raw = llm_.chat(system, user.str());
    } catch (std::exception&) {
        return json{{"error", "LLM request failed"}};
    }

    // Найти границы объекта {...}
    auto start = raw.find('{');
    auto end   = raw.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        return json{{"error", "LLM returned non-JSON"}, {"raw", raw.substr(0, 500)}};
    }
    raw = raw.substr(start, end - start + 1);

    auto j = json::parse(raw, nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        return json{{"error", "JSON parse failed"}, {"raw", raw.substr(0, 500)}};
    }

    // Приводим к единому формату
    json result;
    result["type"]       = "essay";
    result["exam"]       = exam;
    result["section"]    = section;
    result["difficulty"] = difficulty;
    result["prompt"]     = j.value("prompt", "");
    result["min_words"]  = j.value("min_words", 250);
    result["time_minutes"] = j.value("time_minutes", 40);
    result["topic"]      = j.value("topic", "");
    result["tips"]       = j.value("tips", json::array());
    return result;
}
json ExamGenerator::generateReading(const std::string& exam,
                                    const std::string& section,
                                    const std::string& difficulty,
                                    const std::string& lang) const {
    bool is_sat = (exam == "SAT");

    std::string system;
    if (lang == "ru") {
        system =
            "Ты — составитель заданий для экзамена IELTS/TOEFL/SAT. "
            "Твоя задача — придумать ОДНО задание на чтение в стиле официального экзамена. "
            "Отвечай ТОЛЬКО валидным JSON-объектом. Ответ начинается с '{' и заканчивается на '}'. "
            "Без markdown, без пояснений. "
            "Ключи: "
            "{\"passage\":\"текст на английском 300-500 слов\", "
            "\"questions\":["
            "{\"q\":\"вопрос на английском\", "
            "\"type\":\"multiple_choice\" или \"true_false_not_given\", "
            "\"options\":[\"A\",\"B\",\"C\",\"D\"], "
            "\"answer\":\"A\"}], "
            "\"tips\":[\"совет на русском\", \"совет на русском\"]}. "
            "Весь текст passage и вопросы — строго на английском. "
            "Поле tips — строго на русском, НЕ используй китайский. "
            "Сделай 5 вопросов.";
    } else {
        system =
            "You are an IELTS/TOEFL/SAT reading task creator. "
            "Invent ONE reading passage with 5 questions in the style of the official exam. "
            "Respond ONLY with a valid JSON object. Start with '{' and end with '}'. "
            "No markdown, no commentary. "
            "Keys: "
            "{\"passage\":\"English text 300-500 words\", "
            "\"questions\":["
            "{\"q\":\"question in English\", "
            "\"type\":\"multiple_choice\" or \"true_false_not_given\", "
            "\"options\":[\"A\",\"B\",\"C\",\"D\"], "
            "\"answer\":\"A\"}], "
            "\"tips\":[\"tip in English\"]}. "
            "All text must be in English. All 5 questions in English.";
    }

    std::ostringstream user;
    user << "Exam: " << exam << "\n";
    user << "Section: " << section << "\n";
    user << "Difficulty: " << difficulty << "\n";
    if (is_sat) {
        user << "Format: short passage (100-200 words) with 2 multiple-choice questions.\n";
    } else {
        user << "Format: long passage (400-600 words) with 5 questions.\n";
    }
    user << "\nGenerate ONE reading exercise. Return ONLY the JSON object.";

    std::string raw;
    try {
        raw = llm_.chat(system, user.str());
    } catch (std::exception&) {
        return json{{"error", "LLM request failed"}};
    }

    auto start = raw.find('{');
    auto end   = raw.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        return json{{"error", "LLM returned non-JSON"}, {"raw", raw.substr(0, 500)}};
    }
    raw = raw.substr(start, end - start + 1);

    auto j = json::parse(raw, nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        return json{{"error", "JSON parse failed"}, {"raw", raw.substr(0, 500)}};
    }

    json result;
    result["type"]       = "reading";
    result["exam"]       = exam;
    result["section"]    = section;
    result["difficulty"] = difficulty;
    result["passage"]    = j.value("passage", "");
    result["questions"]  = j.value("questions", json::array());
    result["tips"]       = j.value("tips", json::array());
    return result;
}
json ExamGenerator::generateSpeaking(const std::string& exam,
                                     const std::string& section,
                                     const std::string& difficulty,
                                     const std::string& lang) const {
    bool is_part3 = (section.find("Part 3") != std::string::npos);
    bool is_part1 = (section.find("Part 1") != std::string::npos);

    std::string system;
    if (lang == "ru") {
        system =
            "Ты — составитель заданий для устной части IELTS/TOEFL. "
            "Отвечай ТОЛЬКО валидным JSON-объектом. Ответ начинается с '{' и заканчивается на '}'. "
            "Без markdown, без пояснений. "
            "Ключи: "
            "{\"cue_card\":\"тема на английском\", "
            "\"bullets\":[\"You should say: пункт 1 на английском\", \"пункт 2\", \"пункт 3\"], "
            "\"questions\":[\"вопрос на английском\"], "
            "\"time_prep\":число_минут, "
            "\"time_speak\":число_минут, "
            "\"tips\":[\"совет на русском\", \"совет на русском\"]}. "
            "Весь контент (cue_card, bullets, questions) — строго на английском. "
            "Поле tips — строго на русском, НЕ используй китайский.";
    } else {
        system =
            "You are an IELTS/TOEFL speaking task creator. "
            "Respond ONLY with a valid JSON object. Start with '{' and end with '}'. "
            "No markdown, no commentary. "
            "Keys: "
            "{\"cue_card\":\"topic in English\", "
            "\"bullets\":[\"You should say: item 1 in English\", \"item 2\", \"item 3\"], "
            "\"questions\":[\"question in English\"], "
            "\"time_prep\":number_minutes, "
            "\"time_speak\":number_minutes, "
            "\"tips\":[\"tip in English\"]}. "
            "All content must be in English.";
    }

    std::ostringstream user;
    user << "Exam: " << exam << "\n";
    user << "Section: " << section << "\n";
    user << "Difficulty: " << difficulty << "\n";

    if (is_part1) {
        user << "Format: IELTS Speaking Part 1. Generate 6 short questions on a simple everyday topic.\n";
    } else if (is_part3) {
        user << "Format: IELTS Speaking Part 3. Generate 4 discussion questions on an abstract topic.\n";
    } else {
        user << "Format: IELTS Speaking Part 2 (cue card). Generate one topic with 3 'You should say:' bullets. "
             << "time_prep = 1, time_speak = 2.\n";
    }

    user << "\nGenerate ONE speaking exercise. Return ONLY the JSON object.";

    std::string raw;
    try {
        raw = llm_.chat(system, user.str());
    } catch (std::exception&) {
        return json{{"error", "LLM request failed"}};
    }

    auto start = raw.find('{');
    auto end   = raw.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        return json{{"error", "LLM returned non-JSON"}, {"raw", raw.substr(0, 500)}};
    }
    raw = raw.substr(start, end - start + 1);

    auto j = json::parse(raw, nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        return json{{"error", "JSON parse failed"}, {"raw", raw.substr(0, 500)}};
    }

    json result;
    result["type"]         = "speaking";
    result["exam"]         = exam;
    result["section"]      = section;
    result["difficulty"]   = difficulty;
    result["cue_card"]     = j.value("cue_card", "");
    result["bullets"]      = j.value("bullets", json::array());
    result["questions"]    = j.value("questions", json::array());
    result["time_prep"]    = j.value("time_prep", 1);
    result["time_speak"]   = j.value("time_speak", 2);
    result["tips"]         = j.value("tips", json::array());
    return result;
}