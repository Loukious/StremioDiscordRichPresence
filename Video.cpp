#include "Video.h"
#include <sstream>
#include <curl/curl.h>
#include <nlohmann/json.hpp> // Include the JSON library
#include "MemoryUtility.h"
#include "fstream"

using json = nlohmann::json;


Video::Video() {
    videoinfo = MemoryUtility::ReadVideoId();
    if (videoinfo.size() > 1) {
        type = "series";
        s = videoinfo[1];
        ep = videoinfo[2];
    }
    else {
        type = "movie";
    }
    id = videoinfo[0];
    setvidinfo();
}

std::vector<std::string> Video::getvidinfo() {
    return MemoryUtility::ReadVideoId();
}

double Video::gettime() {
    return MemoryUtility::ReadVideoTime();
}

void Video::setvidinfo() {
    std::string url = "https://v3-cinemeta.strem.io/meta/" + type + "/" + id + ".json";

    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

    }
    curl_global_cleanup();

    // Parse JSON
    auto json = nlohmann::json::parse(readBuffer);

    poster = json["meta"]["poster"];
    name = json["meta"]["name"];
    year = json["meta"]["year"];

    if (type == "series") {
        for (const auto& video : json["meta"]["videos"]) {
            if (video["id"].get<std::string>() == join(videoinfo, ":")) {
                thumbnail = video["thumbnail"].get<std::string>();
                epname = video["name"].get<std::string>();
                break;
            }
        }
    }
    
}

size_t Video::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s) {
    s->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string Video::join(const std::vector<std::string>& v, const std::string& delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < v.size(); ++i) {
        oss << v[i];
        if (i != v.size() - 1) {
            oss << delimiter;
        }
    }
    return oss.str();
}
