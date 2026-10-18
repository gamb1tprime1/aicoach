#include "llm_client.hpp"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

static size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* s = static_cast<std::string*>(userdata);
    s->append(ptr, size * nmemb);
    return size * nmemb;
}

LLMClient::LLMClient(std::string url, std::string key, std::string model)
    : api_url_(std::move(url)), api_key_(std::move(key)), model_(std::move(model)) {}

std::string LLMClient::chat(const std::string& system, const std::string& user) const {
            json body = {
        {"model", model_},
        {"messages", json::array({
            {{"role","system"},{"content",system}},
            {{"role","user"},  {"content",user}}
        })},
        {"temperature", 0.3},
        {"max_tokens", 4000}
    };

    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("curl init failed");

    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key_).c_str());

    std::string payload = body.dump();
    curl_easy_setopt(curl, CURLOPT_URL, api_url_.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) throw std::runtime_error("curl error");

    auto j = json::parse(response, nullptr, false);
    if (j.is_discarded() || !j.contains("choices")) {
        throw std::runtime_error("LLM error: " + response.substr(0, 500));
    }
    return j["choices"][0]["message"]["content"].get<std::string>();
}