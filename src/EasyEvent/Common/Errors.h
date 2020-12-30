//
// Created by yuwenyong on 2020/10/26.
//

#ifndef EASYEVENT_COMMON_ERRORS_H
#define EASYEVENT_COMMON_ERRORS_H

#include "EasyEvent/Common/Config.h"

namespace EasyEvent {

    enum class UserErrors {
        InvalidArgument = 1,
        NotFound = 2,
        NotSupported = 3,
        OperationCanceled = 4,
        NotConvertible = 5,
        OutOfRange = 6,
        UnexpectedBehaviour = 7,
    };

    class EASY_EVENT_API UserErrorCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getUserErrorCategory();


    inline std::error_code make_error_code(UserErrors err) {
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
    struct is_error_code_enum<EasyEvent::UserErrors>: public true_type {};

}

#endif //EASYEVENT_COMMON_ERRORS_H
