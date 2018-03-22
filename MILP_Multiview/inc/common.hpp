#pragma once

#include <limits>
#include <string>
#include <sstream>

namespace IMT {
    constexpr int D_MAX = 1;
    constexpr float STALL_COUNT_IN_SECOND = 0.25;
    constexpr float transmissionDelay = 0.020;
    constexpr float INVALIDE_FLOAT_VALUE = -42.42;
    //constexpr float NOT_DOWNLOADED_DISTORTION = 256*256*100;
    //constexpr float NOT_DOWNLOADED_DISTORTION = 256*100;
    constexpr float NOT_DOWNLOADED_DISTORTION = 255*255/4.f;
    constexpr float STALL_QoE_IMPACT = 20*5;//2*0.0008*NOT_DOWNLOADED_DISTORTION;
    //constexpr float STALL_QoE_IMPACT = 2*0.000008*NOT_DOWNLOADED_DISTORTION;
    //constexpr float STALL_QoE_IMPACT = 0.015*NOT_DOWNLOADED_DISTORTION;
    //constexpr float LAG_COST = STALL_QoE_IMPACT*0.75/5.f;
    constexpr float LAG_COST = 2*5;//2*0.0001*NOT_DOWNLOADED_DISTORTION;
    //constexpr float LAG_COST = STALL_QoE_IMPACT*0.5/5.f;
    constexpr float EPSILON = 2*std::numeric_limits<float>::epsilon();

    template<typename Stream, typename T>
    void print_time(Stream& out, T duration_in_ms)
    {
        auto const msecs = duration_in_ms % 1000;
        duration_in_ms /= 1000;
        auto const secs = duration_in_ms % 60;
        duration_in_ms /= 60;
        auto const mins = duration_in_ms % 60;
        duration_in_ms /= 60;
        auto const hours = duration_in_ms % 24;
        duration_in_ms /= 24;
        auto const days = duration_in_ms;
    
        bool printed_earlier = false;
        if (days >= 1) {
            printed_earlier = true;
            out << days << (1 != days ? " days" : " day") << ' ';
        }
        if (printed_earlier || hours >= 1) {
            printed_earlier = true;
            out << hours << (1 != hours ? " hours" : " hour") << ' ';
        }
        if (printed_earlier || mins >= 1) {
            printed_earlier = true;
            out << mins << (1 != mins ? " minutes" : " minute") << ' ';
        }
        if (printed_earlier || secs >= 1) {
            printed_earlier = true;
            out << secs << (1 != secs ? " seconds" : " second") << ' ';
        }
        if (printed_earlier || msecs >= 1) {
            printed_earlier = true;
            out << msecs << (1 != msecs ? " milliseconds" : " millisecond");
        }
    }

    template<typename T>
    std::string print_time(T duration_in_ms)
    {
        std::ostringstream ss;
        print_time(ss, duration_in_ms);
        return ss.str();
    }

}
