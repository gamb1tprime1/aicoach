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
    std::string system;
    
    // === ФОРМИРУЕМ ПРОМПТ ПОД КОНКРЕТНЫЙ ЭКЗАМЕН ===
    if (exam == "IELTS") {
        if (lang == "ru") {
            system = 
                "Ты — создатель заданий для IELTS Reading. "
                "Придумай ОДИН академический текст (400-600 слов) на английском языке на любую научную или социальную тему. "
                "К тексту придумай 5 вопросов (True/False/Not Given или Multiple Choice). "
                "Отвечай строго в JSON формате: "
                "{\"passage\":\"текст\", \"questions\":[{\"q\":\"вопрос\", \"type\":\"multiple_choice\", \"options\":[\"A\",\"B\",\"C\",\"D\"], \"answer\":\"A\"}], \"tips\":[\"совет на русском\"]}. "
                "Весь текст и вопросы — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
            system = 
                "You are an IELTS Reading task creator. "
                "Invent ONE academic passage (400-600 words) in English on a scientific or social topic. "
                "Create 5 questions (True/False/Not Given or Multiple Choice). "
                "Respond ONLY with valid JSON: "
                "{\"passage\":\"text\", \"questions\":[{\"q\":\"question\", \"type\":\"multiple_choice\", \"options\":[\"A\",\"B\",\"C\",\"D\"], \"answer\":\"A\"}], \"tips\":[\"tip in English\"]}. "
                "All text and questions in English.";
        }
    } else if (exam == "TOEFL") {
        if (lang == "ru") {
            system = 
                "Ты — создатель заданий для TOEFL Reading. "
                "Придумай ОДИН университетский текст (500-700 слов) на английском языке на тему биологии, истории или астрономии. "
                "К тексту придумай 5 вопросов (Multiple Choice). "
                "Отвечай строго в JSON формате: "
                "{\"passage\":\"текст\", \"questions\":[{\"q\":\"вопрос\", \"type\":\"multiple_choice\", \"options\":[\"A\",\"B\",\"C\",\"D\"], \"answer\":\"A\"}], \"tips\":[\"совет на русском\"]}. "
                "Весь текст и вопросы — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
            system = 
                "You are a TOEFL Reading task creator. "
                "Invent ONE university-level passage (500-700 words) in English on biology, history, or astronomy. "
                "Create 5 Multiple Choice questions. "
                "Respond ONLY with valid JSON: "
                "{\"passage\":\"text\", \"questions\":[{\"q\":\"question\", \"type\":\"multiple_choice\", \"options\":[\"A\",\"B\",\"C\",\"D\"], \"answer\":\"A\"}], \"tips\":[\"tip in English\"]}. "
                "All text and questions in English.";
        }
    } else if (exam == "SAT") {
        if (lang == "ru") {
            system = 
                "Ты — создатель заданий для Digital SAT Reading & Writing. "
                "Придумай ОДИН короткий текст (100-150 слов) на английском языке на тему литературы, истории или науки. "
                "К тексту придумай 1 вопрос с 4 вариантами ответа. "
                "Отвечай строго в JSON формате: "
                "{\"passage\":\"текст\", \"questions\":[{\"q\":\"вопрос\", \"type\":\"multiple_choice\", \"options\":[\"A\",\"B\",\"C\",\"D\"], \"answer\":\"A\"}], \"tips\":[\"совет на русском\"]}. "
                "Весь текст и вопросы — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
            system = 
                "You are a Digital SAT Reading & Writing task creator. "
                "Invent ONE short passage (100-150 words) in English on literature, history, or science. "
                "Create 1 Multiple Choice question with 4 options. "
                "Respond ONLY with valid JSON: "
                "{\"passage\":\"text\", \"questions\":[{\"q\":\"question\", \"type\":\"multiple_choice\", \"options\":[\"A\",\"B\",\"C\",\"D\"], \"answer\":\"A\"}], \"tips\":[\"tip in English\"]}. "
                "All text and questions in English.";
        }
    } else {
        // Fallback
        system = "You are an English exam task creator. Respond ONLY with valid JSON: {\"passage\":\"text\", \"questions\":[], \"tips\":[]}.";
    }

    std::ostringstream user;
    user << "Exam: " << exam << "\n";
    user << "Section: " << section << "\n";
    user << "Difficulty: " << difficulty << "\n";
    user << "\nGenerate ONE reading exercise matching the format above. Return ONLY the JSON object.";

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
json ExamGenerator::generateListening(const std::string& exam,
                                      const std::string& section,
                                      const std::string& difficulty,
                                      const std::string& lang) const {
    std::string system;
    
    if (exam == "IELTS") {
        if (lang == "ru") {
            system = 
                "Ты — создатель заданий для IELTS Listening. "
                "Придумай ОДИН короткий диалог (100-150 слов) на английском языке на бытовую тему. "
                "К диалогу придумай 3 вопроса (Multiple Choice). "
                "Отвечай строго в JSON формате. Начинай с '{' и заканчивай '}'. "
                "Пример: {\"transcript\":\"A: Hello... B: Hi...\", \"questions\":[{\"q\":\"What is the man booking?\", \"options\":[\"A hotel\",\"A flight\"], \"answer\":\"A hotel\"}], \"time_minutes\":20, \"tips\":[\"Слушай ключевые слова\"]}. "
                "Весь текст и вопросы — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
            system = 
                "You are an IELTS Listening task creator. "
                "Invent ONE short dialogue (100-150 words) in English on an everyday topic. "
                "Create 3 Multiple Choice questions. "
                "Respond ONLY with valid JSON. Start with '{' and end with '}'. "
                "Example: {\"transcript\":\"A: Hello... B: Hi...\", \"questions\":[{\"q\":\"What is the man booking?\", \"options\":[\"A hotel\",\"A flight\"], \"answer\":\"A hotel\"}], \"time_minutes\":20, \"tips\":[\"Listen for key words\"]}. "
                "All text and questions in English.";
        }
    } else if (exam == "TOEFL") {
        if (lang == "ru") {
            system = 
                "Ты — создатель заданий для TOEFL Listening. "
                "Придумай ОДНУ короткую лекцию (150-200 слов) на английском языке на тему биологии или истории. "
                "К лекции придумай 3 вопроса (Multiple Choice). "
                "Отвечай строго в JSON формате. Начинай с '{' и заканчивай '}'. "
                "Пример: {\"transcript\":\"Today we will discuss...\", \"questions\":[{\"q\":\"What is the main topic?\", \"options\":[\"Biology\",\"History\"], \"answer\":\"Biology\"}], \"time_minutes\":20, \"tips\":[\"Записывай ключевые термины\"]}. "
                "Весь текст и вопросы — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
            system = 
                "You are a TOEFL Listening task creator. "
                "Invent ONE short lecture (150-200 words) in English on biology or history. "
                "Create 3 Multiple Choice questions. "
                "Respond ONLY with valid JSON. Start with '{' and end with '}'. "
                "Example: {\"transcript\":\"Today we will discuss...\", \"questions\":[{\"q\":\"What is the main topic?\", \"options\":[\"Biology\",\"History\"], \"answer\":\"Biology\"}], \"time_minutes\":20, \"tips\":[\"Note key terms\"]}. "
                "All text and questions in English.";
        }
    } else {
        system = "You are an English exam task creator. Respond ONLY with valid JSON: {\"transcript\":\"text\", \"questions\":[], \"tips\":[]}.";
    }

    std::ostringstream user;
    user << "Exam: " << exam << "\n";
    user << "Section: " << section << "\n";
    user << "Difficulty: " << difficulty << "\n";
    user << "\nGenerate ONE listening exercise matching the format above. Return ONLY the JSON object.";

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

    // === УМНЫЙ ПАРСЕР ===
    auto j = json::parse(raw, nullptr, false);
    
    // Если строгий парсер упал, пробуем починить частые ошибки LLM
    if (j.is_discarded()) {
        std::string fixed = raw;
        // 1. Удаляем висячие запятые перед } или ]
        size_t pos;
        while ((pos = fixed.find(",}")) != std::string::npos) fixed.replace(pos, 2, "}");
        while ((pos = fixed.find(",]")) != std::string::npos) fixed.replace(pos, 2, "]");
        
        j = json::parse(fixed, nullptr, false);
    }

    if (j.is_discarded() || !j.is_object()) {
        return json{{"error", "JSON parse failed"}, {"raw", raw.substr(0, 500)}};
    }
    // ====================

    json result;
    result["type"]         = "listening";
    result["exam"]         = exam;
    result["section"]      = section;
    result["difficulty"]   = difficulty;
    result["transcript"]   = j.value("transcript", "");
    result["questions"]    = j.value("questions", json::array());
    result["time_minutes"] = j.value("time_minutes", 20);
    result["tips"]         = j.value("tips", json::array());
    return result;
}