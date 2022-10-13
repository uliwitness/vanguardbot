#include "vanguardbot_base.hpp"
#include <iostream>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace vanguard {

    using namespace std;
    using namespace std::chrono;

    void vanguardbot_base::connect(const std::string &inHostname, int inPortNumber,
                                   std::function<void()> inReadyToRunHandler) {
        // Resolve host name into IP:
        hostent *hostname = gethostbyname(inHostname.c_str());
        if (!hostname) {
            cout << "Function failed with error: " << errno << endl;
            return;
        }
        std::string IPAddress(inet_ntoa(**(in_addr **) hostname->h_addr_list));

        //Fill out the information needed to initialize a socket
        sockaddr_in target; //Socket address information

        target.sin_family = AF_INET;
        target.sin_port = htons(inPortNumber);
        target.sin_addr.s_addr = inet_addr(IPAddress.c_str());

        mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (mSocket == -1) {
            cerr << "Error: Couldn't create socket (" << errno << ")." << endl;
            return;
        }

        if (::connect(mSocket, (sockaddr *) &target, sizeof(target)) == -1) {
            close(mSocket);
            cerr << "Error: Couldn't connect to server." << endl;
            mSocket = -1;
            return;
        }

        inReadyToRunHandler();
    }


    vanguardbot_base::~vanguardbot_base() {
        if (mSocket != -1) {
            close(mSocket);
        }
    }


    void vanguardbot_base::send_message(std::string inMessage) {
        cout << "Sending: " << inMessage << endl;
        std::string messageWithLineBreak(inMessage);
        messageWithLineBreak.append("\r\n");

        ssize_t result = send(mSocket, messageWithLineBreak.c_str(), messageWithLineBreak.length(), 0);
        if (result == -1) {
            cerr << "send failed: " << errno << endl;
        }
    }

    seconds vanguardbot_base::next_timer_fire_interval() {
        steady_clock::time_point nextFireTime = steady_clock::time_point::max();
        for (TimerEntry& currTimer : mTimers) {
            if (currTimer.mNextFireTime < nextFireTime) {
                nextFireTime = currTimer.mNextFireTime;
            }
        }

        auto result = duration_cast<seconds>(nextFireTime - steady_clock::now());
        return (result.count() > 0) ? result : seconds(0);
    }

    void vanguardbot_base::fire_expired_timers() {
        auto now = steady_clock::now();
        for (auto currTimer = mTimers.begin(); currTimer != mTimers.end(); ) {
            if (currTimer->mNextFireTime <= now) {
                cout << "\tRunning timer." << endl;
                currTimer->mHandler();
                if (currTimer->mRepeat) {
                    // TODO: Can cause drift, but at least it guarantees we don't fire again immediately if we expired
                    //  ages ago because the computer went to sleep or whatever and now have 5 hours to catch up.
                    cout << "\t\tScheduling timer for " << currTimer->mDelay.count() << " seconds from now." << endl;
                    currTimer->mNextFireTime = now + currTimer->mDelay;
                    ++currTimer;
                } else {
                    cout << "\t\tRemoved non-repeating timer." << endl;
                    currTimer = mTimers.erase(currTimer);
                }
            } else {
                cout << "\tSkipping future timer." << endl;
                ++currTimer;
            }
        }
    }

    void vanguardbot_base::run() {
        if (mSocket == -1) {
            return;
        }

        mKeepRunning = true;
        struct timeval timeout = { 60, 0 };

        while (mKeepRunning) {
            timeout.tv_sec = next_timer_fire_interval().count();
            if (timeout.tv_sec > 10) {
                timeout.tv_sec = 10;
            }
            timeout.tv_usec = 0;
            //cout << "Timeout to next timer is: " << timeout.tv_sec << endl;

            fd_set fdSet = {};
            FD_ZERO(&fdSet);
            FD_SET(mSocket, &fdSet);
            int selResult = select(mSocket + 1, &fdSet, nullptr, nullptr, &timeout);
            if (selResult == 0) { // time-out
                cout << "Checking for timers." << endl;
                fire_expired_timers();
            } else if (selResult == -1) { // Error
                cerr << "Error waiting for socket: " << strerror(errno) << endl;
                fire_expired_timers();
            } else {
                cout << "Received network data." << endl;
                char buffer[513] = {};
                ssize_t amount = recv(mSocket, buffer, sizeof(buffer) - 1, 0);
                if (amount <= 0) {
                    cerr << "Error reading socket: " << strerror(errno) << endl;
                    break;
                }
                cout << "\t" << amount << " bytes." << endl;

                mMessageBuffer.append(buffer);
                process_full_lines();
                fire_expired_timers();
            }
        }
    }

    void vanguardbot_base::perform_after(chrono::minutes delay, bool repeat, function<void()> handler) {
        cout << "Added timer: " << delay.count() << " minutes" << (repeat ? " (repeating)" : "") << "." << endl;
        mTimers.push_back(TimerEntry{steady_clock::time_point::min(), handler, repeat, duration_cast<seconds>(delay) });
    }

} /* vanguard */