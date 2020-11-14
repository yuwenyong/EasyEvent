//
// Created by yuwenyong on 2020/10/26.
//

#ifndef EASYEVENT_COMMON_ERRORS_H
#define EASYEVENT_COMMON_ERRORS_H

#include "EasyEvent/Common/Config.h"

namespace EasyEvent {

    enum class CommonErrc {
        InvalidArgument = 1,
        NotFound = 2,
        AlreadyRegistered = 3,
    };

    class EASY_EVENT_API CommonErrCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    EASY_EVENT_API const std::error_category& getCommonErrCategory();


    inline std::error_code make_error_code(CommonErrc err) {
        return {static_cast<int>(err), getCommonErrCategory()};
    }
}

namespace std {
    template <>
    struct is_error_code_enum<EasyEvent::CommonErrc>: public true_type {};

}

#endif //EASYEVENT_COMMON_ERRORS_H
