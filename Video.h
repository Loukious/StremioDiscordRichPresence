#pragma once

#include <string>
#include <vector>

class Video {
public:
    Video();

    void setvidinfo();

    std::vector<std::string> getvidinfo();
    double gettime();

    std::string type, s, ep, id, poster, name, thumbnail, epname, year;


    std::string join(const std::vector<std::string>& v, const std::string& delimiter);

private:
    std::vector<std::string> videoinfo;

    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s);
};
