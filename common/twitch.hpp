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
        string game_id(const std::string &userID, const string &game);

        void set_game(const std::string &userID, const string &gameID);
        void set_stream_title(const std::string &userID, const string &title);

        Json send_request(const string &url, const string &body = "", const char *method = nullptr);

    protected:
        CURL *mCurlHandle{nullptr};
        string mUserName;
        string mPassword;
    };

} // vanguardbot
