//
// Created by yuwenyong on 2020/10/30.
//

#ifndef EASYEVENT_COMMON_TIME_H
#define EASYEVENT_COMMON_TIME_H

#include "EasyEvent/Common/Config.h"

namespace EasyEvent {

    class EASY_EVENT_API Time {
    public:
        Time() = default;

        int64 seconds() const {
            return _usec / 1000000;
        }

        int64 milliSeconds() const {
            return _usec / 1000;
        }

        int64 microSeconds() const {
            return _usec;
        }

        double secondsAsDouble() const {
            return static_cast<double>(_usec) / 1000000.0;
        }

        double milliSecondsAsDouble() const {
            return static_cast<double>(_usec) / 1000.0;
        }

        double microSecondsAsDouble() const {
            return static_cast<double>(_usec);
        }

        struct tm utcTime() const;

        struct tm localTime() const;

        std::string toDateTimeString() const;

        std::string toDurationString() const;

        std::string toString(const char *format) const;

        Time operator-() const {
            return Time(_usec);
        }

        Time operator+(const Time& rhs) const {
            return Time(_usec + rhs._usec);
        }

        Time operator-(const Time& rhs) const {
            return Time(_usec - rhs._usec);
        }

        Time& operator+=(const Time& rhs) {
            _usec += rhs._usec;
            return * this;
        }

        Time& operator-=(const Time& rhs) {
            _usec -= rhs._usec;
            return *this;
        }

        bool operator<(const Time& rhs) const {
            return _usec < rhs._usec;
        }

        bool operator<=(const Time& rhs) const {
            return _usec <= rhs._usec;
        }

        bool operator>(const Time& rhs) const {
            return _usec > rhs._usec;
        }

        bool operator>=(const Time& rhs) const {
            return _usec >= rhs._usec;
        }

        bool operator==(const Time& rhs) const {
            return _usec == rhs._usec;
        }

        bool operator!=(const Time& rhs) const {
            return _usec != rhs._usec;
        }

        double operator/(const Time& rhs) const {
            return static_cast<double>(_usec) / static_cast<double>(rhs._usec);
        }

        Time& operator*=(int32 rhs) {
            _usec *= rhs;
            return *this;
        }

        Time operator*(int32 rhs) const {
            Time t;
            t._usec = _usec * rhs;
            return t;
        }

        Time& operator*=(int64 rhs) {
            _usec *= rhs;
            return *this;
        }
        
        Time operator*(int64 rhs) const {
            Time t;
            t._usec = _usec * rhs;
            return t;
        }

        Time& operator*=(double rhs) {
            _usec *= static_cast<int64>(static_cast<double>(_usec) * rhs);
            return *this;
        }
        
        Time operator*(double rhs) const {
            Time t;
            t._usec = static_cast<int64>(static_cast<double>(_usec) * rhs);
            return t;
        }

        Time& operator/=(int32 rhs) {
            _usec /= rhs;
            return *this;
        }

        Time operator/(int32 rhs) const {
            Time t;
            t._usec = _usec / rhs;
            return t;
        }

        Time& operator/=(int64 rhs) {
            _usec /= rhs;
            return *this;
        }

        Time operator/(int64 rhs) const {
            Time t;
            t._usec = _usec / rhs;
            return t;
        }

        Time& operator/=(double rhs) {
            _usec *= static_cast<int64>(static_cast<double>(_usec) / rhs);
            return *this;
        }

        Time operator/(double rhs) const {
            Time t;
            t._usec = static_cast<int64>(static_cast<double>(_usec) / rhs);
            return t;
        }

        static Time now();

        static Time makeTime(struct tm &tm);

        static Time makeTime(int year, int mon, int day, int hour=0, int min=0, int sec=0, int isdst=0);

        static Time seconds(int64 t) {
            return Time(t * INT64(1000000));
        }
        
        static Time milliSeconds(int64 t) {
            return Time(t * INT64(1000));
        }

        static Time microSeconds(int64 t) {
            return Time(t);
        }

        static Time seconds(double t) {
            return Time(static_cast<int64>(t * 1000000));
        }

        static Time milliSeconds(double t) {
            return Time(static_cast<int64>(t * 1000));
        }

        static Time microSeconds(double t) {
            return Time(static_cast<int64>(t));
        }

    protected:
        explicit Time(int64 usec)
            : _usec(usec) {

        }

        int64 _usec{0};
    };

    inline std::ostream& operator<<(std::ostream& out, const Time& tm) {
        return out << tm.secondsAsDouble();
    }
}

#endif //EASYEVENT_COMMON_TIME_H
