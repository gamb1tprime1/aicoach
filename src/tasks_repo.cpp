#include "tasks_repo.hpp"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void TasksRepo::load_from_dir(const std::string& dir) {
    if (!fs::exists(dir)) return;
    for (auto& p : fs::directory_iterator(dir)) {
        if (p.path().extension() != ".json") continue;
        std::ifstream f(p.path());
        nlohmann::json j;
        f >> j;
        for (auto& t : j) {
            Task task;
            task.id         = t.value("id", "");
            task.exam       = t.value("exam", "");
            task.section    = t.value("section", "");
            task.difficulty = t.value("difficulty", "");
            task.prompt     = t.value("prompt", "");
            task.meta       = t.value("meta", nlohmann::json::object());
            tasks_.push_back(std::move(task));
        }
    }
}

std::vector<Task> TasksRepo::pick(const std::string& exam,
                                  const std::string& section,
                                  const std::string& difficulty,
                                  int count) const {
    std::vector<Task> out;
    for (auto& t : tasks_) {
        if (t.exam != exam) continue;
        if (!section.empty() && t.section != section) continue;
        if (!difficulty.empty() && t.difficulty != difficulty) continue;
        out.push_back(t);
        if ((int)out.size() >= count) break;
    }
    return out;
}
