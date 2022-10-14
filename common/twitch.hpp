#pragma once

#include "../json11/json11.hpp"
#include <curl/curl.h>
#include <string>

namespace vanguard {

    using namespace json11;
    using namespace std;

    class twitch {
    public:
        struct ChannelInfo {
            string mTitle;
            string mGameName;
            string mGameID;
        };

        static void init();

        twitch(const string &username, const string &password);
        ~twitch();

        string user_id();
        ChannelInfo channel_info(const string &userID);

        Json get_request(const string& url);

    protected:
        CURL *mCurlHandle{nullptr};
        string mUserName;
        string mPassword;
    };

} // vanguardbot
