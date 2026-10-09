#pragma once
#include "models.hpp"
#include <vector>

class TasksRepo {
public:
    void load_from_dir(const std::string& dir);
    std::vector<Task> pick(const std::string& exam,
                           const std::string& section,
                           const std::string& difficulty,
                           int count) const;
private:
    std::vector<Task> tasks_;
};
