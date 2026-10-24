#pragma once
#include "llm_client.hpp"
#include "tasks_repo.hpp"
#include "essay_coach.hpp"
#include "portfolio_builder.hpp"
#include "study_planner.hpp"
#include "history_db.hpp"
#include "task_generator.hpp"
#include "exam_generator.hpp"

class ApiServer {
public:
    ApiServer(const std::string& host, int port,
              const std::string& llm_url,
              const std::string& llm_key,
              const std::string& model,
              const std::string& data_dir,
              const std::string& frontend_dir);
    void run();
private:
    std::string host_;
    std::string frontend_dir_;
    int port_;
    LLMClient llm_;
    TasksRepo tasks_;
    EssayCoach essay_;
    PortfolioBuilder portfolio_;
        StudyPlanner planner_;
    HistoryDB history_;
    TaskGenerator generator_;
    ExamGenerator exam_gen_;
};