#include "task_generator.hpp"
#include <sstream>
#include <ctime>
#include <cstdio>

using json = nlohmann::json;

json TaskGenerator::generate(const std::string& exam,
                             const std::string& section,
                             const std::string& difficulty,
                             int count,
                             const std::string& lang) const {
    if (count < 1) count = 1;
    if (count > 10) count = 10;

    // Берём 3 примера из репозитория как образец стиля
    auto samples = repo_.pick(exam, section, difficulty, 3);

    std::ostringstream samples_str;
    for (auto& t : samples) {
        samples_str << "- [" << t.section << "] [" << t.difficulty << "] " << t.prompt << "\n";
    }
    if (samples.empty()) {
        samples_str << "(no samples available, invent typical " << exam << " tasks)\n";
    }

    std::string system;
    if (lang == "ru") {
        system =
            "Ты — составитель заданий для экзамена IELTS/SAT. "
            "Твоя задача — придумать НОВЫЕ задания, не повторяя темы из примеров. "
            "Отвечай ТОЛЬКО валидным JSON-массивом. Ответ начинается с '[' и заканчивается на ']'. "
            "Без markdown, без ```, без пояснений. "
            "Каждый элемент — объект с ключами: "
            "{\"section\":строка, \"difficulty\":строка, \"prompt\":строка, \"meta\":{}}. "
            "Поле prompt — на английском (это экзамен IELTS/SAT, они на английском). "
            "meta может содержать topic, min_words, time_minutes. "
            "Пример ответа: [{\"section\":\"Writing Task 2\",\"difficulty\":\"band7\",\"prompt\":\"Some prompt...\",\"meta\":{\"topic\":\"education\"}}].";
    } else {
        system =
            "You are an IELTS/SAT task creator. "
            "Invent NEW tasks that do NOT repeat topics from the examples. "
            "Respond ONLY with a valid JSON array. Start with '[' and end with ']'. "
            "No markdown, no backticks, no commentary. "
            "Each item is an object with keys: "
            "{\"section\":string, \"difficulty\":string, \"prompt\":string, \"meta\":{}}. "
            "The prompt must be in English (IELTS/SAT tasks are in English). "
            "meta may contain topic, min_words, time_minutes. "
            "Example response: [{\"section\":\"Writing Task 2\",\"difficulty\":\"band7\",\"prompt\":\"Some prompt...\",\"meta\":{\"topic\":\"education\"}}].";
    }

    std::ostringstream user;
    user << "Exam: " << exam << "\n";
    if (!section.empty())    user << "Section: " << section << "\n";
    if (!difficulty.empty()) user << "Difficulty: " << difficulty << "\n";
    user << "Generate exactly " << count << " new tasks.\n\n";
    user << "Examples (do NOT repeat these topics):\n" << samples_str.str();
    user << "\nReturn ONLY the JSON array of " << count << " new tasks.";

    std::string raw;
    try {
        raw = llm_.chat(system, user.str());
    } catch (std::exception& ex) {
        // Видно в окне сервера — помогает отладке
        std::fprintf(stderr, "[task_generator] LLM exception: %s\n", ex.what());
        return json::array();
    }

    // Ищем границы массива [...]
    auto arr_start = raw.find('[');
    auto arr_end   = raw.rfind(']');
    if (arr_start == std::string::npos || arr_end == std::string::npos || arr_end <= arr_start) {
        std::fprintf(stderr, "[task_generator] No array brackets in raw: %s\n", raw.substr(0, 300).c_str());
        return json::array();
    }
    raw = raw.substr(arr_start, arr_end - arr_start + 1);

    auto j = json::parse(raw, nullptr, false);
    if (j.is_discarded() || !j.is_array()) {
        std::fprintf(stderr, "[task_generator] JSON parse failed. Raw: %s\n", raw.substr(0, 300).c_str());
        return json::array();
    }

    // Оставляем только валидные элементы, добавляем id
    json result = json::array();
    int idx = 0;
    for (auto& item : j) {
        if (!item.is_object()) continue;
        if (!item.contains("prompt") || !item["prompt"].is_string()) continue;

        json task;
        task["id"]         = "gen-" + std::to_string(std::time(nullptr)) + "-" + std::to_string(idx++);
        task["section"]    = item.value("section", section.empty() ? "Writing" : section);
        task["difficulty"] = item.value("difficulty", difficulty.empty() ? "band7" : difficulty);
        task["prompt"]     = item["prompt"];
        task["meta"]       = item.value("meta", json::object());

        result.push_back(std::move(task));
        if (result.size() >= count) break;
    }
    return result;
}