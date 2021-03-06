//
// Created by yuwenyong on 2020/12/20.
//

#ifndef EASYEVENT_COMMON_UTILITY_H
#define EASYEVENT_COMMON_UTILITY_H

#include "EasyEvent/Common/Config.h"

namespace EasyEvent {

    class NonCopyable {
    public:
        NonCopyable() {}
        ~NonCopyable() {}
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
    };


    template <typename T> struct FailType: std::false_type {};

    template <typename VariantT, typename CompT>
    class VariantComparer: NonCopyable {
    public:
        explicit VariantComparer(const VariantT& lhs) noexcept
            : _lhs(lhs) {

        }

        template<typename T>
        bool operator()(const T& rhsContent) const {
            const T& lhsContent = std::get<T>(_lhs);
            return CompT()(lhsContent, rhsContent);
        }
    private:
        const VariantT& _lhs;
    };

    struct VariantEqualComp {
        template<typename T>
        bool operator()(const T& lhs, const T& rhs) const {
            return lhs == rhs;
        }
    };

    struct VariantLessComp {
        template<typename T>
        bool operator()(const T& lhs, const T& rhs) const {
            return lhs < rhs;
        }
    };


    template <typename ValueT>
    class Holder {
    public:
        explicit Holder(const std::shared_ptr<ValueT>& val, bool takeover=false)
                : _val(val.get())
                , _ptr(takeover ? val : nullptr) {

        }

        std::shared_ptr<ValueT> takeover() {
            Assert(!_ptr);
            _ptr = _val->shared_from_this();
            return _ptr;
        }

        std::shared_ptr<ValueT> release() {
            Assert(_ptr);
            auto ptr = std::move(_ptr);
            return ptr;
        }

        std::shared_ptr<const ValueT> shared() const {
            if (_ptr) {
                return _ptr;
            }
            return _val->shared_from_this();
        }

        std::shared_ptr<ValueT> shared() {
            if (_ptr) {
                return _ptr;
            }
            return _val->shared_from_this();
        }

        const ValueT* get() const {
            return _val;
        }

        ValueT* get() {
            return _val;
        }

        const ValueT* operator->() const {
            return _val;
        }

        ValueT* operator->() {
            return _val;
        }

        const ValueT& operator*() const {
            return *_val;
        }

        ValueT& operator*() {
            return *_val;
        }
    private:
        ValueT* _val;
        std::shared_ptr<ValueT> _ptr;
    };

    template <typename ValueT>
    bool operator<(const Holder<ValueT>& lhs, const Holder<ValueT>& rhs) {
        return lhs.get() < rhs.get();
    }

}


#endif //EASYEVENT_COMMON_UTILITY_H
