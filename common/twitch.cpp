#include "twitch.hpp"
#include <string>
#include <iostream>
#include "client_id.h"

namespace vanguard {

    using namespace json11;
    using namespace std;

    static size_t writeFunction(void *ptr, size_t size, size_t nmemb, string* data) {
        data->append((char*) ptr, size * nmemb);
        return size * nmemb;
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
        auto json = get_request(string("https://api.twitch.tv/helix/channels?broadcaster_id=") + userID);
        return ChannelInfo{json["title"].string_value(), json["game_name"].string_value(), json["game_id"].string_value()};
    }

    string twitch::user_id() {
        auto json = get_request(string("https://api.twitch.tv/helix/users?login=") + mUserName);
        auto idJson = json["id"];
        if (idJson.is_string()) {
            return idJson.string_value();
        } else {
            return to_string(idJson.int_value());
        }
    }

    Json twitch::get_request(const string &url) {
        string responseBody;
        string responseHeaders;
        long httpStatus = 0;
        struct curl_slist *headers = nullptr;

        headers = curl_slist_append(headers, (string("Authorization: Bearer ") + mPassword.substr(6)).c_str());
        headers = curl_slist_append(headers, "Client-Id: " CLIENT_ID);

        //curl_easy_setopt(mCurlHandle, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(mCurlHandle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(mCurlHandle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(mCurlHandle, CURLOPT_WRITEFUNCTION, writeFunction);
        curl_easy_setopt(mCurlHandle, CURLOPT_WRITEDATA, &responseBody);
        curl_easy_setopt(mCurlHandle, CURLOPT_HEADERDATA, &responseHeaders);

        CURLcode res = curl_easy_perform(mCurlHandle);
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
        }
        curl_easy_getinfo(mCurlHandle, CURLINFO_RESPONSE_CODE, &httpStatus);


        //cout << "STATUS: " << httpStatus << "\n" << responseHeaders << responseBody << "\n" << endl;

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