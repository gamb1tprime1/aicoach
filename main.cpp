#include "http_server.hpp"
#include <cstdlib>
#include <iostream>

int main() {
    const char* llm_url = std::getenv("LLM_API_URL");
    const char* llm_key = std::getenv("LLM_API_KEY");
    const char* model   = std::getenv("LLM_MODEL");
    const char* data    = std::getenv("DATA_DIR");
    const char* front   = std::getenv("FRONTEND_DIR");

    ApiServer server(
        "0.0.0.0", 8080,
        llm_url ? llm_url : "https://api.openai.com/v1/chat/completions",
        llm_key ? llm_key : "",
        model   ? model   : "gpt-4o-mini",
        data    ? data    : "./data",
        front   ? front   : "./frontend"
    );
    server.run();
    return 0;
}
