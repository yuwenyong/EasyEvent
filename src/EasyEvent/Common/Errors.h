//
// Created by yuwenyong on 2020/10/26.
//

#ifndef EASYEVENT_COMMON_ERRORS_H
#define EASYEVENT_COMMON_ERRORS_H

#include "EasyEvent/Common/Config.h"

namespace EasyEvent {

    enum class UserErrorCode {
        InvalidArgument = 1,
        NotFound = 2,
        AlreadyRegistered = 3,
    };

    class EASY_EVENT_API UserErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getUserErrorCategory();


    inline std::error_code make_error_code(UserErrorCode err) {
        return {static_cast<int>(err), getUserErrorCategory()};
    }

    EASY_EVENT_API void doThrowError(const std::error_code& err);

    EASY_EVENT_API void doThrowError(const std::error_code& err, const char* location);

    inline void throwError(const std::error_code& err) {
        if (err) {
            doThrowError(err);
        }
    }

    inline void throwError(const std::error_code& err, const char* location) {
        if (err) {
            doThrowError(err, location);
        }
    }
}

namespace std {

    template <>
    struct is_error_code_enum<EasyEvent::UserErrorCode>: public true_type {};

}

#endif //EASYEVENT_COMMON_ERRORS_H
