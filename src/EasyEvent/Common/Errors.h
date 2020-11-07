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
    };

    class EASY_EVENT_API CommonErrCategory: public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override;
        [[nodiscard]] std::string message(int ev) const override;
    };

    extern const CommonErrCategory gCommonErrCategory;

    inline std::error_code make_error_code(CommonErrc err) {
        return {static_cast<int>(err), gCommonErrCategory};
    }
}

namespace std {
    template <>
    struct is_error_code_enum<EasyEvent::CommonErrc>: public true_type {};

}

#endif //EASYEVENT_COMMON_ERRORS_H
