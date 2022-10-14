#include "twitch.hpp"
#include "client_id.h"
#include "string_utils.hpp"
#include <string>
#include <iostream>

namespace vanguard {

    using namespace json11;
    using namespace std;

    static size_t writeFunction(void *ptr, size_t size, size_t nmemb, string* data) {
        data->append((char*) ptr, size * nmemb);
        return size * nmemb;
    }

    static string url_escaped(const string &target) {
        char *escaped = curl_easy_escape(nullptr, target.c_str(), target.size());
        if (!escaped) {
            return {};
        }
        string result(escaped);
        curl_free(escaped);
        return result;
    }

    /*static*/ void twitch::init() {
        curl_global_init(CURL_GLOBAL_ALL);
    }

    twitch::twitch(const string &username, const string &password)
    : mUserName(username), mPassword(password) {
        mCurlHandle = curl_easy_init();
        if (!mCurlHandle) {
            cerr << "Couldn't open CURL handle." << endl;
        }
    }

    twitch::~twitch() {
        curl_easy_cleanup(mCurlHandle);
    }

    twitch::ChannelInfo twitch::channel_info(const std::string &userID) {
        auto json = send_request(string("https://api.twitch.tv/helix/channels?broadcaster_id=") + userID, "", nullptr);
        return ChannelInfo{json["title"].string_value(), json["game_name"].string_value(), json["game_id"].string_value()};
    }

    string twitch::user_id() {
        auto json = send_request(string("https://api.twitch.tv/helix/users?login=") + mUserName, "", nullptr);
        auto idJson = json["id"];
        if (idJson.is_string()) {
            return idJson.string_value();
        } else {
            return to_string(idJson.int_value());
        }
    }

    void twitch::set_stream_title(const std::string &userID, const string &title) {
        Json bodyJson = Json::object {
                {"title", title}
        };

        auto json = send_request(
                string("https://api.twitch.tv/helix/channels?broadcaster_id=") + userID,
                bodyJson.dump(),
                "PATCH");
    }

    string twitch::game_id(const std::string &userID, const string &game) {
        string escapedGame = url_escaped(game);
        auto json = send_request(string("https://api.twitch.tv/helix/games?name=") + escapedGame);
        auto idJson = json["id"];
        return idJson.string_value();
    }

    void twitch::set_game(const std::string &userID, const string &gameID) {
        Json bodyJson = Json::object {
                {"game_id", gameID}
        };

        auto json = send_request(
                string("https://api.twitch.tv/helix/channels?broadcaster_id=") + userID,
                bodyJson.dump(),
                "PATCH");
    }

    Json twitch::send_request(const string &url, const string &body, const char *method) {
        string responseBody;
        string responseHeaders;
        long httpStatus = 0;
        struct curl_slist *headers = nullptr;

        headers = curl_slist_append(headers, (string("Authorization: Bearer ") + mPassword.substr(6)).c_str());
        headers = curl_slist_append(headers, "Client-Id: " CLIENT_ID);
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(mCurlHandle, CURLOPT_VERBOSE, 1L);
        if (method) {
            cout << "METHOD: " << method << endl;
            curl_easy_setopt(mCurlHandle, CURLOPT_CUSTOMREQUEST, method);
        }
        curl_easy_setopt(mCurlHandle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(mCurlHandle, CURLOPT_URL, url.c_str());
        cout << "URL: " << url << endl;
        if (!body.empty()) {
            cout << "BODY: " << body << endl;
            curl_easy_setopt(mCurlHandle, CURLOPT_POSTFIELDS, body.data());
            curl_easy_setopt(mCurlHandle, CURLOPT_POSTFIELDSIZE, body.size());
        }
        curl_easy_setopt(mCurlHandle, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(mCurlHandle, CURLOPT_WRITEDATA, &responseBody);
        curl_easy_setopt(mCurlHandle, CURLOPT_HEADERDATA, &responseHeaders);

        CURLcode res = curl_easy_perform(mCurlHandle);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_getinfo(mCurlHandle, CURLINFO_RESPONSE_CODE, &httpStatus);

        cout << "\nSTATUS: " << httpStatus << "\n" << responseHeaders << responseBody << "\n" << endl;

        string errMsg;
        auto json = json11::Json::parse(responseBody, errMsg);
        if (!errMsg.empty()) {
            cerr << "Error parsing Twitch reply: " << errMsg << endl;
        }

        auto dataJson = json["data"];
        if (dataJson.is_array()) {
            auto firstDataJson = dataJson[0];
            if (firstDataJson.is_object()) {
                return firstDataJson;
            }
        }
        return {};
    }

} // vanguardbot