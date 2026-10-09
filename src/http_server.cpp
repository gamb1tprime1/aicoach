#include "http_server.hpp"
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

ApiServer::ApiServer(const std::string& host, int port,
                     const std::string& llm_url,
                     const std::string& llm_key,
                     const std::string& model,
                     const std::string& data_dir,
                     const std::string& frontend_dir)
    : host_(host), frontend_dir_(frontend_dir), port_(port),
      llm_(llm_url, llm_key, model),
      essay_(llm_), portfolio_(llm_), planner_(llm_) {
    tasks_.load_from_dir(data_dir + "/tasks");
}

static void send_json(httplib::Response& res, const json& j, int status = 200) {
    res.status = status;
    res.set_content(j.dump(), "application/json");
}

void ApiServer::run() {
    httplib::Server svr;

    svr.set_post_routing_handler([](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    });
    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    svr.set_mount_point("/", frontend_dir_);

    svr.Post("/api/tasks", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) return send_json(res, {{"error", "bad json"}}, 400);

        auto list = tasks_.pick(body.value("exam", "IELTS"),
                                body.value("section", ""),
                                body.value("difficulty", ""),
                                body.value("count", 5));

        json out = json::array();
        for (auto& t : list) {
            out.push_back({
                {"id", t.id},
                {"section", t.section},
                {"difficulty", t.difficulty},
                {"prompt", t.prompt},
                {"meta", t.meta}
            });
        }
        send_json(res, {{"tasks", out}});
    });

    svr.Post("/api/essay/coach", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) return send_json(res, {{"error", "bad json"}}, 400);

        EssayDraft e;
        e.user_id    = body.value("user_id", "");
        e.university = body.value("university", "");
        e.prompt     = body.value("prompt", "");
        e.draft      = body.value("draft", "");
        e.essay_type = body.value("essay_type", "Personal Statement");

        try {
            send_json(res, essay_.analyze(e));
        } catch (std::exception& ex) {
            send_json(res, {{"error", ex.what()}}, 500);
        }
    });

    svr.Post("/api/portfolio", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) return send_json(res, {{"error", "bad json"}}, 400);

        UserProfile u;
        u.user_id         = body.value("user_id", "");
        u.target_exam     = body.value("target_exam", "IELTS");
        u.target_score    = body.value("target_score", 7.0);
        u.native_language = body.value("native_language", "");
        u.current_level   = body.value("current_level", "");
        u.major           = body.value("major", "");
        u.weeks_available = body.value("weeks_available", 12);
        u.hours_per_week  = body.value("hours_per_week", 6.0);
        if (body.contains("universities"))
            for (auto& x : body["universities"]) u.universities.push_back(x);

        json uni_req = body.value("university_requirements", json::object());

        try {
            auto p = portfolio_.build(u, uni_req);
            send_json(res, {
                {"sections", p.sections},
                {"action_items", p.action_items},
                {"university_requirements", p.university_requirements}
            });
        } catch (std::exception& ex) {
            send_json(res, {{"error", ex.what()}}, 500);
        }
    });

    svr.Post("/api/study-plan", [&](const httplib::Request& req, httplib::Response& res) {
        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) return send_json(res, {{"error", "bad json"}}, 400);

        UserProfile u;
        u.user_id         = body.value("user_id", "");
        u.target_exam     = body.value("target_exam", "IELTS");
        u.target_score    = body.value("target_score", 7.0);
        u.native_language = body.value("native_language", "");
        u.current_level   = body.value("current_level", "");
        u.weeks_available = body.value("weeks_available", 12);
        u.hours_per_week  = body.value("hours_per_week", 6.0);

        std::vector<Task> weak;
        if (body.contains("weak_spots")) {
            for (auto& id : body["weak_spots"]) {
                Task t;
                t.id = id;
                weak.push_back(t);
            }
        }

        try {
            auto plan = planner_.make_plan(u, weak);
            send_json(res, {
                {"weekly_goals", plan.weekly_goals},
                {"daily_tasks", plan.daily_tasks},
                {"resources", plan.resources},
                {"rationale", plan.rationale}
            });
        } catch (std::exception& ex) {
            send_json(res, {{"error", ex.what()}}, 500);
        }
    });

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

    std::cout << "AI Coach running on http://" << host_ << ":" << port_ << "\n";
    svr.listen(host_.c_str(), port_);
}
