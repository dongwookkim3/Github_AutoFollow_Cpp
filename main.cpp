#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class GithubAPIBot {
public:
    std::string username;
    std::string token;
    int sleepSecondsActionMin;
    int sleepSecondsActionMax;
    int sleepSecondsLimitedMin;
    int sleepSecondsLimitedMax;
    int maxFollow;
    std::vector<std::string> usersToAction;

    // 응답 데이터를 누적할 버퍼
    std::string responseBuffer;

    GithubAPIBot(const std::string& user, const std::string& token,
                 int sleepMin, int sleepMax, int sleepMinLimited,
                 int sleepMaxLimited, int maxFollow)
        : username(user), token(token), sleepSecondsActionMin(sleepMin),
          sleepSecondsActionMax(sleepMax), sleepSecondsLimitedMin(sleepMinLimited),
          sleepSecondsLimitedMax(sleepMaxLimited), maxFollow(maxFollow) {}

    // GitHub 사용자 팔로워를 가져오는 함수
    void getFollowers(const std::string& user) {
        std::string url = "https://api.github.com/users/" + user + "/followers";
        CURL* curl;
        CURLcode res;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();

        if(curl) {
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, ("Authorization: token " + token).c_str());
            headers = curl_slist_append(headers, "User-Agent: C++-Bot");

            // 응답 버퍼 초기화
            responseBuffer.clear();

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);  // 객체 포인터 전달
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);

            res = curl_easy_perform(curl);
            if(res != CURLE_OK) {
                std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            } else {
                // 응답 데이터를 모두 누적한 후, 파싱 시도
                // (디버깅을 위해 전체 응답을 출력)
                std::cout << "Full Response: " << responseBuffer << std::endl;
                try {
                    auto jsonData = nlohmann::json::parse(responseBuffer);
                    for (const auto& follower : jsonData) {
                        std::string login = follower["login"];
                        usersToAction.push_back(login);
                    }
                } catch (const std::exception& e) {
                    std::cerr << "JSON Parsing Error: " << e.what() << std::endl;
                }
            }

            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
        }
        curl_global_cleanup();
    }

    // 응답 데이터를 누적할 콜백 함수
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        GithubAPIBot* bot = static_cast<GithubAPIBot*>(userp);
        bot->responseBuffer.append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    // 사용자 팔로우
    void follow() {
        for (const auto& user : usersToAction) {
            std::cout << "Following " << user << "..." << std::endl;
            // 실제 팔로우 로직 구현 가능
        }
    }
};

// 환경 변수 로딩
std::string getEnvVar(const std::string& key) {
    const char* val = std::getenv(key.c_str());
    if (val == nullptr) {
        throw std::runtime_error("Environment variable not found: " + key);
    }
    return std::string(val);
}

int main(int argc, char* argv[]) {
    std::string user = getEnvVar("GITHUB_USER");
    std::string token = getEnvVar("TOKEN");

    int sleepMin = 20, sleepMax = 120, sleepMinLimited = 600, sleepMaxLimited = 1500, maxFollow = 100;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-t" && i + 1 < argc) {
            std::string targetUser = argv[i + 1];
            GithubAPIBot bot(user, token, sleepMin, sleepMax, sleepMinLimited, sleepMaxLimited, maxFollow);
            bot.getFollowers(targetUser);
            bot.follow();
            i++; // 다음 인수 건너뛰기
        }
    }

    return 0;
}
