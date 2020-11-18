//
// Created by yuwenyong on 2020/10/30.
//

#include "EasyEvent/Common/Time.h"


struct tm EasyEvent::Time::utcTime() const {
    time_t time = static_cast<long>(_usec / 1000000);

    struct tm* t;
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    t = gmtime(&time);
#else
    struct tm tr;
    gmtime_r(&time, &tr);
    t = &tr;
#endif
    return *t;
}

struct tm EasyEvent::Time::localTime() const {
    time_t time = static_cast<long>(_usec / 1000000);

    struct tm* t;
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    t = localtime(&time);
#else
    struct tm tr;
    localtime_r(&time, &tr);
    t = &tr;
#endif
    return *t;
}

std::string EasyEvent::Time::toDateTimeString() const {
    std::ostringstream os;
    os << toString("%Y-%m-%d %H:%M:%S") << ".";
    os.fill('0');
    os.width(3);
    os << static_cast<long>(_usec % 1000000 / 1000);
    return os.str();
}

std::string EasyEvent::Time::toDurationString() const {
    int64 usecs = _usec % 1000000;
    int64 secs = _usec / 1000000 % 60;
    int64 mins = _usec / 1000000 / 60 % 60;
    int64 hours = _usec / 1000000 / 60 / 60 % 24;
    int64 days = _usec / 1000000 / 60 / 60 / 24;

    using namespace std;

    std::ostringstream os;
    if(days != 0)
    {
        os << days << "d ";
    }
    os << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << mins << ":" << std::setw(2) << secs;
    if(usecs != 0)
    {
        os << "." << std::setw(3) << (usecs / 1000);
    }

    return os.str();
}

std::string EasyEvent::Time::toString(const char *format) const {
    struct tm t = localTime();
    char buf[32];
    if(strftime(buf, sizeof(buf), format, &t) == 0)
    {
        return std::string();
    }
    return std::string(buf);
}

EasyEvent::Time EasyEvent::Time::now() {
#if EASY_EVENT_PLATFORM == EASY_EVENT_PLATFORM_WINDOWS
    struct _timeb tb;
    _ftime(&tb);
    return Time(static_cast<int64>(tb.time) * INT64(1000000) + static_cast<int64>(tb.millitm) * 1000);
#else
    struct timeval tv;
    gettimeofday(&tv, 0);
    return Time(tv.tv_sec * INT64(1000000) + tv.tv_usec);
#endif
}

EasyEvent::Time EasyEvent::Time::makeTime(struct tm &tm) {
    time_t t = mktime(&tm);
    return seconds(static_cast<int64>(t));
}

EasyEvent::Time EasyEvent::Time::makeTime(int year, int mon, int day, int hour, int min, int sec, int isdst) {
    struct tm t;
    memset(&t, 0, sizeof(t));
    t.tm_year = year - 1900;
    t.tm_mon = mon - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = isdst;
    return makeTime(t);
}