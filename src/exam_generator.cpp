#include "exam_generator.hpp"
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <atomic>

using json = nlohmann::json;

// Глобальный счетчик для уникального seed при вызове в цикле
static std::atomic<int> g_seed_counter{0};
static int get_unique_seed() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)) + g_seed_counter++);
    return std::rand() % 1000;
}

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
            "Поле tips — строго на русском. НЕ используй китайский.";
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

    auto start = raw.find('{');
    auto end   = raw.rfind('}');
    if (start == std::string::npos || end == std::string::npos || end <= start) {
        return json{{"error", "LLM returned non-JSON"}, {"raw", raw.substr(0, 500)}};
    }
    raw = raw.substr(start, end - start + 1);

    auto j = json::parse(raw, nullptr, false);
    if (j.is_discarded()) {
        std::string fixed = raw;
        size_t pos;
        while ((pos = fixed.find(",}")) != std::string::npos) fixed.replace(pos, 2, "}");
        while ((pos = fixed.find(",]")) != std::string::npos) fixed.replace(pos, 2, "]");
        j = json::parse(fixed, nullptr, false);
    }
    if (j.is_discarded() || !j.is_object()) {
        return json{{"error", "JSON parse failed"}, {"raw", raw.substr(0, 500)}};
    }

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
    int random_seed = get_unique_seed();
    
    std::string system;
    
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
                "Придумай ОДИН университетский текст (400-500 слов) на английском языке. "
                "Выбери СЛУЧАЙНУЮ тему из списка (НЕ астрономию!): биология, история, психология, экономика, литература, социология, геология, лингвистика. "
                "К тексту придумай 5 вопросов (Multiple Choice). "
                "Отвечай строго в JSON формате: "
                "{\"passage\":\"текст\", \"questions\":[{\"q\":\"вопрос\", \"type\":\"multiple_choice\", \"options\":[\"A\",\"B\",\"C\",\"D\"], \"answer\":\"A\"}], \"tips\":[\"совет на русском\"]}. "
                "Весь текст и вопросы — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
            system = 
                "You are a TOEFL Reading task creator. "
                "Invent ONE university-level passage (400-500 words) in English. "
                "Choose a RANDOM topic from this list (NOT astronomy!): biology, history, psychology, economics, literature, sociology, geology, linguistics. "
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
        system = "You are an English exam task creator. Respond ONLY with valid JSON: {\"passage\":\"text\", \"questions\":[], \"tips\":[]}.";
    }
    
    system += " Seed: " + std::to_string(random_seed) + ".";

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
    if (j.is_discarded()) {
        std::string fixed = raw;
        size_t pos;
        while ((pos = fixed.find(",}")) != std::string::npos) fixed.replace(pos, 2, "}");
        while ((pos = fixed.find(",]")) != std::string::npos) fixed.replace(pos, 2, "]");
        j = json::parse(fixed, nullptr, false);
    }
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

    int random_seed = get_unique_seed();

    std::string system;
    if (exam == "IELTS") {
        if (lang == "ru") {
            system =
                "Ты — составитель заданий для устной части IELTS. "
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
                "You are an IELTS speaking task creator. "
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
            " CRITICAL: All content MUST be in English. DO NOT use Chinese, Japanese, or any other language.";
        }
    } else {
        // TOEFL Speaking
        if (lang == "ru") {
                        system =
                "Ты — создатель заданий для устной части TOEFL. "
                "Придумай ОДИН вопрос для устного ответа. "
                "ЗАПРЕЩЕНО использовать тему Education (образование). "
                "Выбери тему из списка: технологии, здоровье, работа, путешествия, хобби, экология, спорт, еда, музыка. "
                "Seed: " + std::to_string(random_seed) + ". "
                "Отвечай ТОЛЬКО валидным JSON-объектом: "
                "{\"cue_card\":\"тема на английском\", \"questions\":[\"вопрос на английском\"], \"time_prep\":15, \"time_speak\":45, \"tips\":[\"совет на русском\"]}. "
                "Весь контент — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
                       system =
                "You are a TOEFL Speaking task creator. "
                "Invent ONE speaking prompt. "
                "DO NOT use the topic Education. "
                "Choose a topic from this list: technology, health, work, travel, hobbies, environment, sports, food, music. "
                "Seed: " + std::to_string(random_seed) + ". "
                "Respond ONLY with valid JSON: "
                "{\"cue_card\":\"topic in English\", \"questions\":[\"question in English\"], \"time_prep\":15, \"time_speak\":45, \"tips\":[\"tip in English\"]}. "
                "All content in English.";        }
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
    if (j.is_discarded()) {
        std::string fixed = raw;
        size_t pos;
        while ((pos = fixed.find(",}")) != std::string::npos) fixed.replace(pos, 2, "}");
        while ((pos = fixed.find(",]")) != std::string::npos) fixed.replace(pos, 2, "]");
        j = json::parse(fixed, nullptr, false);
    }
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
    int random_seed = get_unique_seed();
    
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
                "Придумай ОДНУ короткую лекцию (150-200 слов) на английском языке. "
                "Выбери СЛУЧАЙНУЮ тему из списка: биология, психология, экономика, литература, геология, музыка, астрономия, химия. "
                "К лекции придумай 3 вопроса (Multiple Choice). "
                "Отвечай строго в JSON формате. Начинай с '{' и заканчивай '}'. "
                "Пример: {\"transcript\":\"Today we will discuss...\", \"questions\":[{\"q\":\"What is the main topic?\", \"options\":[\"Biology\",\"History\"], \"answer\":\"Biology\"}], \"time_minutes\":20, \"tips\":[\"Записывай ключевые термины\"]}. "
                "Весь текст и вопросы — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
                        system = 
                "You are a TOEFL Listening task creator. "
                "Invent ONE short lecture (150-200 words) in English. "
                "Choose a RANDOM topic from this list: biology, psychology, economics, literature, geology, music, astronomy, chemistry. "
                "Create 3 Multiple Choice questions. "
                "Respond ONLY with valid JSON. Start with '{' and end with '}'. "
                "Example: {\"transcript\":\"Today we will discuss...\", \"questions\":[{\"q\":\"What is the main topic?\", \"options\":[\"Biology\",\"History\"], \"answer\":\"Biology\"}], \"time_minutes\":20, \"tips\":[\"Note key terms\"]}. "
                "All text and questions in English.";
        }
    } else {
        system = "You are an English exam task creator. Respond ONLY with valid JSON: {\"transcript\":\"text\", \"questions\":[], \"tips\":[]}.";
    }
    
    system += " Seed: " + std::to_string(random_seed) + ".";

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

    auto j = json::parse(raw, nullptr, false);
    if (j.is_discarded()) {
        std::string fixed = raw;
        size_t pos;
        while ((pos = fixed.find(",}")) != std::string::npos) fixed.replace(pos, 2, "}");
        while ((pos = fixed.find(",]")) != std::string::npos) fixed.replace(pos, 2, "]");
        j = json::parse(fixed, nullptr, false);
    }
    if (j.is_discarded() || !j.is_object()) {
        return json{{"error", "JSON parse failed"}, {"raw", raw.substr(0, 500)}};
    }

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

json ExamGenerator::generateMath(const std::string& exam,
                                 const std::string& section,
                                 const std::string& difficulty,
                                 const std::string& lang) const {
    int random_seed = get_unique_seed();

    std::string system;
    
    if (lang == "ru") {
        system = 
            "Ты — создатель заданий по математике для SAT. "
            "Придумай ОДНУ задачу. "
            "Выбери СЛУЧАЙНУЮ тему из списка: алгебра, геометрия, статистика, функции, вероятности, проценты, пропорции, анализ данных. "
            "Отвечай строго по шаблону, БЕЗ JSON, БЕЗ фигурных скобок: "
            "ВОПРОС: [текст задачи на английском] "
            "ВАРИАНТЫ: [A) ..., B) ..., C) ..., D) ...] "
            "ОТВЕТ: [правильная буква] "
            "ОБЪЯСНЕНИЕ: [краткое решение на русском] "
            "ТЕМА: [тема на английском]. "
            "Сгенерируй уникальную задачу с seed: " + std::to_string(random_seed) + ". "
            "Не используй прямоугольники с периметром 36. Придумай что-то новое.";
    } else {
        system = 
            "You are a SAT Math task creator. "
            "Invent ONE math problem. "
            "Choose a RANDOM topic from this list: algebra, geometry, statistics, functions, probability, percentages, ratios, data analysis. "
            "Respond strictly in this format, NO JSON, NO curly braces: "
            "ВОПРОС: [problem text in English] "
            "ВАРИАНТЫ: [A) ..., B) ..., C) ..., D) ...] "
            "ОТВЕТ: [correct letter] "
            "ОБЪЯСНЕНИЕ: [brief solution in English] "
            "ТЕМА: [topic in English]. "
            "Generate a unique problem with seed: " + std::to_string(random_seed) + ". "
            "Do NOT use rectangles with a perimeter of 36. Invent something new.";
    }

    std::ostringstream user;
    user << "Exam: " << exam << "\n";
    user << "Section: " << section << "\n";
    user << "Difficulty: " << difficulty << "\n";
    user << "\nGenerate ONE math problem.";

    std::string raw;
    try {
        raw = llm_.chat(system, user.str());
    } catch (std::exception&) {
        return json{{"error", "LLM request failed"}};
    }

     auto get_field = [&](const std::string& key1, const std::string& key2) -> std::string {
        std::string raw_lower = raw;
        std::transform(raw_lower.begin(), raw_lower.end(), raw_lower.begin(), ::tolower);
        std::string k1_lower = key1;
        std::transform(k1_lower.begin(), k1_lower.end(), k1_lower.begin(), ::tolower);
        
        size_t start = raw_lower.find(k1_lower);
        if (start == std::string::npos) {
            // Ищем альтернативный ключ (английский), если не нашли русский
            std::string k2_lower = key2;
            std::transform(k2_lower.begin(), k2_lower.end(), k2_lower.begin(), ::tolower);
            start = raw_lower.find(k2_lower);
            if (start == std::string::npos) return "";
            start += k2_lower.length();
        } else {
            start += k1_lower.length();
        }
        
        // Пропускаем пробелы и двоеточия
        while (start < raw.length() && (raw[start] == ' ' || raw[start] == ':' || raw[start] == '*' || raw[start] == '\n' || raw[start] == '\r')) {
            start++;
        }
        
        size_t end = std::string::npos;
        const char* next_keys[] = {"вопрос", "варианты", "ответ", "объяснение", "тема", 
                                   "question", "options", "answer", "explanation", "topic"};
        for (const char* nk : next_keys) {
            size_t pos = raw_lower.find(nk, start);
            if (pos != std::string::npos && (end == std::string::npos || pos < end)) {
                end = pos;
            }
        }
        
        std::string res = raw.substr(start, end - start);
        // Очистка от лишних символов в начале и конце
        res.erase(0, res.find_first_not_of(" \t\n\r:{}[]\"'*#"));
        res.erase(res.find_last_not_of(" \t\n\r:{}[]\"'*#") + 1);
        return res;
    };

    std::string question = get_field("ВОПРОС:", "QUESTION:");
    std::string options_str = get_field("ВАРИАНТЫ:", "OPTIONS:");
    std::string answer = get_field("ОТВЕТ:", "ANSWER:");
    std::string explanation = get_field("ОБЪЯСНЕНИЕ:", "EXPLANATION:");
    std::string topic = get_field("ТЕМА:", "TOPIC:");

    json options_arr = json::array();
    std::stringstream ss(options_str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        item.erase(0, item.find_first_not_of(" \t\n\r"));
        item.erase(item.find_last_not_of(" \t\n\r") + 1);
        if (!item.empty()) options_arr.push_back(item);
    }
    // Если модель не смогла выдать задачу по шаблону — возвращаем ошибку,
    // чтобы сервер попробовал сгенерировать её ещё раз (у нас есть 3 попытки в цикле)
    if (question.empty() || options_arr.empty()) {
        return json{{"error", "Empty math problem"}};
    }
    json result;
    result["type"]        = "math";
    result["exam"]        = exam;
    result["section"]     = section;
    result["difficulty"]  = difficulty;
    result["question"]    = question;
    result["options"]     = options_arr;
    result["answer"]      = answer;
    result["explanation"] = explanation;
    result["topic"]       = topic;
    return result;
}

json ExamGenerator::generateWritingTask1(const std::string& exam,
                                         const std::string& section,
                                         const std::string& difficulty,
                                         const std::string& lang) const {
    bool is_general = (section.find("Letter") != std::string::npos);

    std::string system;
    if (is_general) {
        if (lang == "ru") {
            system =
                "Ты — составитель IELTS General Training Writing Task 1 (письмо). "
                "Придумай ОДНО задание на написание письма (150 слов). "
                "Выбери тип: formal, semi-formal или informal. "
                "Отвечай ТОЛЬКО валидным JSON-объектом. Начинай с '{' и заканчивай '}'. "
                "Ключи: "
                "{\"letter_type\":\"formal | semi-formal | informal\", "
                "\"prompt\":\"полный текст задания на английском\", "
                "\"recipient\":\"кому адресовано (на английском)\", "
                "\"purpose\":\"цель письма (на английском)\", "
                "\"min_words\":150, "
                "\"time_minutes\":20, "
                "\"tips\":[\"совет на русском\", \"совет на русском\"]}. "
                "Все поля (prompt, recipient, purpose) — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
            system =
                "You are an IELTS General Training Writing Task 1 (letter) creator. "
                "Invent ONE letter task (150 words). "
                "Choose type: formal, semi-formal, or informal. "
                "Respond ONLY with valid JSON. Start with '{' and end with '}'. "
                "Keys: "
                "{\"letter_type\":\"formal | semi-formal | informal\", "
                "\"prompt\":\"full task text in English\", "
                "\"recipient\":\"addressee (in English)\", "
                "\"purpose\":\"letter purpose (in English)\", "
                "\"min_words\":150, "
                "\"time_minutes\":20, "
                "\"tips\":[\"tip in English\"]}. "
                "All content in English.";
        }
    } else {
        if (lang == "ru") {
            system =
                "Ты — составитель IELTS Academic Writing Task 1 (описание графика). "
                "Придумай ОДНО задание: описание графика, диаграммы, таблицы или схемы (150 слов). "
                "Опиши, что изображено на графике, в текстовой форме (модель не может нарисовать график). "
                "Отвечай ТОЛЬКО валидным JSON-объектом. Начинай с '{' и заканчивай '}'. "
                "Ключи: "
                "{\"chart_type\":\"bar chart | line graph | pie chart | table | map | process diagram\", "
                "\"chart_description\":\"описание данных графика на английском (что показывает, какие цифры)\", "
                "\"prompt\":\"полный текст задания на английском\", "
                "\"min_words\":150, "
                "\"time_minutes\":20, "
                "\"tips\":[\"совет на русском\", \"совет на русском\"]}. "
                "Все поля (chart_type, chart_description, prompt) — на английском. Советы — на русском. НЕ используй китайский.";
        } else {
            system =
                "You are an IELTS Academic Writing Task 1 (chart description) creator. "
                "Invent ONE task: describe a chart, graph, table, or diagram (150 words). "
                "Describe what the chart shows in text (model can't draw images). "
                "Respond ONLY with valid JSON. Start with '{' and end with '}'. "
                "Keys: "
                "{\"chart_type\":\"bar chart | line graph | pie chart | table | map | process diagram\", "
                "\"chart_description\":\"description of chart data in English (what it shows, numbers)\", "
                "\"prompt\":\"full task text in English\", "
                "\"min_words\":150, "
                "\"time_minutes\":20, "
                "\"tips\":[\"tip in English\"]}. "
                "All content in English.";
        }
    }

    std::ostringstream user;
    user << "Exam: " << exam << "\n";
    user << "Section: " << section << "\n";
    user << "Difficulty: " << difficulty << "\n";
    user << "\nGenerate ONE Writing Task 1 task. Return ONLY the JSON object.";

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
    if (j.is_discarded()) {
        std::string fixed = raw;
        size_t pos;
        while ((pos = fixed.find(",}")) != std::string::npos) fixed.replace(pos, 2, "}");
        while ((pos = fixed.find(",]")) != std::string::npos) fixed.replace(pos, 2, "]");
        j = json::parse(fixed, nullptr, false);
    }
    if (j.is_discarded() || !j.is_object()) {
        return json{{"error", "JSON parse failed"}, {"raw", raw.substr(0, 500)}};
    }

    json result;
    result["type"]       = "writing_task1";
    result["exam"]       = exam;
    result["section"]    = section;
    result["difficulty"] = difficulty;
    result["min_words"]  = j.value("min_words", 150);
    result["time_minutes"] = j.value("time_minutes", 20);
    result["prompt"]     = j.value("prompt", "");
    result["tips"]       = j.value("tips", json::array());

    if (is_general) {
        result["letter_type"] = j.value("letter_type", "formal");
        result["recipient"]   = j.value("recipient", "");
        result["purpose"]     = j.value("purpose", "");
    } else {
        result["chart_type"]        = j.value("chart_type", "");
        result["chart_description"] = j.value("chart_description", "");
    }
    return result;
}