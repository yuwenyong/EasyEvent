//
// Created by yuwenyong on 2020/11/20.
//

#ifndef EASYEVENT_COMMON_TASK_H
#define EASYEVENT_COMMON_TASK_H

#include "EasyEvent/Common/Config.h"


namespace EasyEvent {

    template <typename ResT, typename... ArgsT>
    struct MaybeUnaryOrBinaryFunction {

    };

    template <typename ResT, typename T1>
    struct MaybeUnaryOrBinaryFunction<ResT, T1>: std::unary_function<T1, ResT> {

    };

    template <typename ResT, typename T1, typename T2>
    struct MaybeUnaryOrBinaryFunction<ResT, T1, T2>: std::binary_function<T1, T2, ResT> {

    };

    class EASY_EVENT_API BadTaskCall: public std::exception {
    public:
        ~BadTaskCall() noexcept override;

        const char* what() const noexcept override;
    };

    template <typename Tp>
    struct IsLocationInvariant: std::is_trivially_copyable<Tp>::type {

    };

    class _UndefinedClass;

    union _NocopyTypes {
        void* _object;
        const void* _constObject;
        void (*_functionPointer)();
        void (_UndefinedClass::*_memberPointer)();
    };

    union _AnyData {

        void* access() {
            return &_data[0];
        }

        const void* access() const {
            return &_data[0];
        }

        template<typename Tp>
        Tp& access() {
            return *static_cast<Tp*>(access());
        }

        template<typename Tp>
        const Tp& access() const {
            return *static_cast<const Tp*>(access());
        }

        _NocopyTypes _unused;
        char _data[sizeof(_NocopyTypes)];
    };

    enum _ManagerOperation {
        _GetFunctorPtr,
        _DestroyFunctor
    };

    template <typename Tp>
    struct _SimpleTypeWrapper {
        _SimpleTypeWrapper(Tp value): _value(value) {}
        Tp _value;
    };

    template <typename Tp>
    struct IsLocationInvariant<_SimpleTypeWrapper<Tp>>: IsLocationInvariant<Tp> {

    };

    template <typename SignatureT>
    class Task;

    class TaskBase {
    public:
        static const std::size_t MaxSize = sizeof(_NocopyTypes);
        static const std::size_t MaxAlign = alignof(_NocopyTypes);

        template<typename FunctorT>
        class BaseManager {
        protected:
            static const bool StoredLocally = (
                    IsLocationInvariant<FunctorT>::value &&
                    sizeof(FunctorT) <= MaxSize &&
                    alignof(FunctorT) <= MaxAlign &&
                    (MaxAlign % alignof(FunctorT) == 0));

            typedef std::integral_constant<bool, StoredLocally> LocalStorage;

            static FunctorT* getPointer(const _AnyData& source) {
                const FunctorT* ptr = StoredLocally ?
                        std::addressof(source.access<FunctorT>()) :
                        source.access<FunctorT*>();
                return const_cast<FunctorT*>(ptr);
            }

            static void destroy(_AnyData& victim, std::true_type) {
                victim.access<FunctorT>().~FunctorT();
            }

            static void destroy(_AnyData& victim, std::false_type) {
                delete victim.access<FunctorT*>();
            }

        public:
            static bool manager(_AnyData& dest, const _AnyData& source, _ManagerOperation op) {
                switch (op) {
                    case _GetFunctorPtr:
                        dest.access<FunctorT*>() = getPointer(source);
                        break;
                    case _DestroyFunctor:
                        destroy(dest, LocalStorage());
                        break;
                }
                return false;
            }

            static void initFunctor(_AnyData& functor, FunctorT&& f) {
                initFunctor(functor, std::move(f), LocalStorage());
            }

            template<typename SignatureT>
            static bool notEmpty(const Task<SignatureT>& t) {
                return static_cast<bool>(t);
            }

            template<typename Tp>
            static bool notEmpty(Tp* fp) {
                return fp != nullptr;
            }

            template<typename ClassT, typename Tp>
            static bool notEmpty(Tp ClassT::*mp) {
                return mp != nullptr;
            }

            template<typename Tp>
            static bool notEmpty(const Tp&) {
                return true;
            }

        private:
            static void initFunctor(_AnyData& functor, FunctorT&& f, std::true_type) {
                ::new (functor.access()) FunctorT(std::move(f));
            }

            static void initFunctor(_AnyData& functor, FunctorT&& f, std::false_type) {
                functor.access<FunctorT*>() = new FunctorT(std::move(f));
            }
        };

        TaskBase(): _manager(nullptr) {}

        ~TaskBase() {
            if (_manager) {
                _manager(_functor, _functor, _DestroyFunctor);
            }
        }

        bool empty() const { return !_manager; }

        typedef bool (*ManagerType)(_AnyData&, const _AnyData&, _ManagerOperation);

        _AnyData _functor;
        ManagerType _manager;
    };

    template <typename SignatureT, typename FunctorT>
    class TaskHandler;

    template <typename ResT, typename FunctorT, typename... ArgsT>
    class TaskHandler<ResT(ArgsT...), FunctorT>: public TaskBase::BaseManager<FunctorT> {
        typedef TaskBase::BaseManager<FunctorT> BaseT;
    public:
        static ResT invoke(const _AnyData& functor, ArgsT&&... args) {
            return (*BaseT::getPointer(functor))(std::forward<ArgsT>(args)...);
        }
    };

    template <typename FunctorT, typename... ArgsT>
    class TaskHandler<void(ArgsT...), FunctorT>: public TaskBase::BaseManager<FunctorT> {
        typedef TaskBase::BaseManager<FunctorT> BaseT;

    public:
        static void invoke(const _AnyData& functor, ArgsT&&... args) {
            (*BaseT::getPointer(functor))(std::forward<ArgsT>(args)...);
        }
    };

    template<typename ClassT, typename MemberT, typename ResT, typename... ArgsT>
    class TaskHandler<ResT(ArgsT...), MemberT ClassT::*>: public TaskHandler<void(ArgsT...), MemberT ClassT::*> {
        typedef TaskHandler<void(ArgsT...), MemberT ClassT::*> BaseT;

    public:
        static ResT invoke(const _AnyData& functor, ArgsT&&... args) {
            return std::invoke(BaseT::getPointer(functor)->_value, std::forward<ArgsT>(args)...);
        }
    };

    template<typename ClassT, typename MemberT, typename... ArgsT>
    class TaskHandler<void(ArgsT...), MemberT ClassT::*>
        : public TaskBase::BaseManager<_SimpleTypeWrapper<MemberT ClassT::*>> {
        typedef MemberT ClassT::* FunctorT;
        typedef _SimpleTypeWrapper<FunctorT > WrapperT;
        typedef TaskBase::BaseManager<WrapperT> BaseT;

    public:
        static bool manager(_AnyData& dest, const _AnyData& source, _ManagerOperation op) {
            switch (op) {
                case _GetFunctorPtr:
                    dest.access<FunctorT*>() = &BaseT::getPointer(source)->_value;
                    break;
                default:
                    BaseT::manager(dest, source, op);
            }
            return false;
        }

        static void invoke(const _AnyData& functor, ArgsT&&... args) {
            std::invoke(BaseT::getPointer(functor)->_value, std::forward<ArgsT>(args)...);
        }
    };

    template <typename FromT, typename ToT>
    using CheckFuncReturnType = std::conditional<
            std::is_void_v<ToT>, std::true_type,
            std::conditional<std::is_same_v<FromT, ToT>, std::true_type,
            std::is_convertible<FromT, ToT>>>;

    template <typename ResT, typename... ArgsT>
    class Task<ResT(ArgsT...)>: public MaybeUnaryOrBinaryFunction<ResT, ArgsT...>, private TaskBase {
        template<typename FuncT, typename Res2T = typename std::invoke_result<FuncT&, ArgsT...>::type>
        struct Callable: CheckFuncReturnType<Res2T, ResT> {

        };

        template<typename CondT, typename Tp>
        using Requires = typename std::enable_if<CondT::value, Tp>::type;

        template<typename CondT, typename Tp>
        using RequiresNot = typename std::enable_if<!(CondT::value), Tp>::type;

    public:
        typedef ResT result_type;

        Task() noexcept
        : TaskBase() {}

        Task(nullptr_t) noexcept
        : TaskBase() {}

        Task(const Task&) = delete;

        Task(Task&& rhs) noexcept: TaskBase() {
            rhs.swap(*this);
        }

        template<typename FunctorT,
                typename = RequiresNot<std::is_same<FunctorT, Task>, void>,
                typename = Requires<Callable<FunctorT>, void>>
        Task(FunctorT func): TaskBase() {
            typedef TaskHandler<ResT(ArgsT...), FunctorT> HandlerType;

            if (HandlerType::notEmpty(func)) {
                HandlerType::initFunctor(_functor, std::move(func));
                _invoker = &HandlerType::invoke;
                _manager = &HandlerType::manager;
            }
        }

        Task& operator=(const Task&) = delete;

        Task& operator=(Task&& rhs) noexcept {
            Task(std::move(rhs)).swap(*this);
            return *this;
        }

        Task& operator=(nullptr_t) noexcept {
            if (_manager) {
                _manager(_functor, _functor, _DestroyFunctor);
                _manager = nullptr;
                _invoker = nullptr;
            }
            return *this;
        }

        template<typename FunctorT>
        Requires<Callable<typename std::decay<FunctorT>::type>, Task&>
        operator=(FunctorT&& rhs) {
            Task(std::forward<FunctorT>(rhs)).swap(*this);
            return *this;
        }

        template<typename FunctorT>
        Task& operator=(std::reference_wrapper<FunctorT> rhs) noexcept {
            Task(rhs).swap(*this);
            return *this;
        }

        void swap(Task& rhs) noexcept {
            std::swap(_functor, rhs._functor);
            std::swap(_manager, rhs._manager);
            std::swap(_invoker, rhs._invoker);
        }

        explicit operator bool() const noexcept {
            return !empty();
        }

        ResT operator()(ArgsT... args) const;

    private:
        using InvokerType = ResT (*)(const _AnyData&, ArgsT&&...);
        InvokerType  _invoker;
    };

//    template <typename ResT, typename... ArgsT>
//    template <typename FunctorT,  typename>
//    Task<ResT(ArgsT...)>::Task(FunctorT func)
//        :TaskBase() {
//        typedef TaskHandler<ResT(ArgsT...), FunctorT> HandlerType;
//
//        if (HandlerType::notEmpty(func)) {
//            HandlerType::initFunctor(_functor, std::move(func));
//            _invoker = &HandlerType::invoke;
//            _manager = &HandlerType::manager;
//        }
//    }

    template <typename ResT, typename... ArgsT>
    ResT Task<ResT(ArgsT...)>::operator()(ArgsT... args) const {
        if (empty()) {
            throw BadTaskCall();
        }
        return _invoker(_functor, std::forward<ArgsT>(args)...);
    }


    template <typename ResT, typename... ArgsT>
    inline bool operator==(const Task<ResT(ArgsT...)>& t, nullptr_t) noexcept {
        return !static_cast<bool>(t);
    }

    template <typename ResT, typename... ArgsT>
    inline bool operator==(nullptr_t, const Task<ResT(ArgsT...)>& t) noexcept {
        return !static_cast<bool>(t);
    }

    template <typename ResT, typename... ArgsT>
    inline bool operator!=(const Task<ResT(ArgsT...)>& t, nullptr_t) noexcept {
        return static_cast<bool>(t);
    }

    template <typename ResT, typename... ArgsT>
    inline bool operator!=(nullptr_t, const Task<ResT(ArgsT...)>& t) noexcept {
        return static_cast<bool>(t);
    }

    template <typename ResT, typename... ArgsT>
    inline void swap(Task<ResT(ArgsT...)>& lhs, Task<ResT(ArgsT...)>& rhs) noexcept {
        lhs.swap(rhs);
    }
}

#endif //EASYEVENT_COMMON_TASK_H
