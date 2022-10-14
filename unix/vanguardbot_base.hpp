#pragma once

#include <string>
#include <functional>
#include <chrono>


namespace vanguard {

    using namespace std;

    class vanguardbot_base {
    public:
        vanguardbot_base() {}

        virtual ~vanguardbot_base();

        void connect(const std::string &inHostname, int inPortNumber, std::function<void()> inReadyToRunHandler);

        void send_message(std::string inMessage);

        void run();

        void perform_after(chrono::minutes delay, bool repeat, function<void()> handler);

    protected:
        struct TimerEntry {
            chrono::steady_clock::time_point mNextFireTime;
            function<void()> mHandler;
            bool mRepeat;
            chrono::seconds mDelay;
        };

        virtual void process_full_lines() = 0;

        virtual void log_in(const string &userName, const string &password, const string &channelName, const string &channelPassword) = 0;
        virtual void log_out() = 0;

        chrono::seconds next_timer_fire_interval();
        void fire_expired_timers();

        bool mKeepRunning = true;
        std::string mMessageBuffer;
        int mSocket = -1;
        std::vector<TimerEntry> mTimers;
    };

} /* vanguard */