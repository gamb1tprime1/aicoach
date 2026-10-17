cmake_minimum_required(VERSION 3.16)
project(AICoach LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(aicoach
    main.cpp
    src/llm_client.cpp
    src/tasks_repo.cpp
    src/essay_coach.cpp
    src/portfolio_builder.cpp
    src/study_planner.cpp
    src/http_server.cpp
    src/history_db.cpp
)
target_link_libraries(aicoach PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    SQLite::SQLite3
    ws2_32
)
target_include_directories(aicoach PRIVATE include)