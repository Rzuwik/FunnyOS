#ifndef FUNNYOS_STDLIB_HEADERS_FUNNYOS_STDLIB_SYSTEM_HPP
#define FUNNYOS_STDLIB_HEADERS_FUNNYOS_STDLIB_SYSTEM_HPP

#include <FunnyOS/Stdlib/Config.hpp>
#include "Compiler.hpp"
#include "Platform.hpp"

// Copy and move constructor macros
#define NON_COPYABLE(name)      \
    name(const name&) = delete; \
    name& operator=(const name&) = delete

#define NON_MOVEABLE(name)          \
    name(name&&) noexcept = delete; \
    name& operator=(name&&) noexcept = delete

#define TRIVIALLY_COPYABLE(name) \
    name(const name&) = default; \
    name& operator=(const name&) = default

#define TRIVIALLY_MOVEABLE(name)     \
    name(name&&) noexcept = default; \
    name& operator=(name&&) noexcept = default

#define COPYABLE(name) \
    name(const name&); \
    name& operator=(const name&)

#define MOVEABLE(name)     \
    name(name&&) noexcept; \
    name& operator=(name&&) noexcept

#define INTERFACE(name) \
   protected:           \
    name() = default;   \
                        \
   public:              \
    NON_COPYABLE(name); \
    NON_MOVEABLE(name); \
    virtual ~name() = default;

//
// Assertion macros
//
#define F_FAIL_ASSERT(message)                                                    \
    do {                                                                          \
        F_ERROR_WITH_MESSAGE(FunnyOS::Stdlib::System::AssertionFailure, message); \
    } while (0)

#define F_ASSERT(condition, message) \
    do {                             \
        if (!(condition)) {          \
            F_FAIL_ASSERT(message);  \
        }                            \
    } while (0)

//
// Exception macros
//
#define F_TRIVIAL_EXCEPTION(name, message)                                      \
    class name : public FunnyOS::Stdlib::System::Exception {                    \
       public:                                                                  \
        name() noexcept  = default;                                             \
        ~name() override = default;                                             \
        TRIVIALLY_COPYABLE(name);                                               \
        TRIVIALLY_MOVEABLE(name);                                               \
                                                                                \
        [[nodiscard]] inline const char* GetMessage() const noexcept override { \
            return message;                                                     \
        }                                                                       \
    }

#define F_TRIVIAL_EXCEPTION_WITH_MESSAGE(name)                                  \
    class name : public FunnyOS::Stdlib::System::Exception {                    \
       public:                                                                  \
        name(const char* message) noexcept : m_message(message) {}              \
        ~name() override = default;                                             \
        TRIVIALLY_COPYABLE(name);                                               \
        TRIVIALLY_MOVEABLE(name);                                               \
                                                                                \
        [[nodiscard]] inline const char* GetMessage() const noexcept override { \
            return m_message;                                                   \
        }                                                                       \
                                                                                \
       private:                                                                 \
        const char* m_message;                                                  \
    }

#ifndef F_LL
#define F_ERROR(exception_class) \
    do {                         \
        throw exception_class(); \
    } while (0)

#define F_ERROR_WITH_MESSAGE(exception_class, message) \
    do {                                               \
        throw exception_class(message);                \
    } while (0)
#else

#define F_ERROR(exception_class)                                                    \
    do {                                                                            \
        using namespace FunnyOS::Stdlib::System;                                    \
        Exception* exception = _Exception::SetActiveException(new exception_class); \
        Terminate(exception->GetMessage());                                         \
    } while (0)

#define F_ERROR_WITH_MESSAGE(exception_class, message)                                       \
    do {                                                                                     \
        using namespace FunnyOS::Stdlib::System;                                             \
        Exception* exception = _Exception::SetActiveException(new exception_class(message)); \
        Terminate(exception->GetMessage());                                                  \
    } while (0)

#endif

// Buffer macros
#define F_SIZEOF_BUFFER(buffer) (sizeof(buffer) / sizeof(buffer[0]))  // NOLINT(bugprone-sizeof-expression)

namespace FunnyOS::Stdlib::System {

    /**
     * Aborts the current process execution.
     */
    [[noreturn]] void Abort();

    /**
     * Terminates the current process with the supplied reason.
     */
    [[noreturn]] void Terminate(const char* reason);

    /**
     * Reports error to the standard process error stream.
     * String is formatted via String::Format
     *
     * @param error error format
     * @param ... format parameters
     */
    void ReportError(const char* error, ...);

    /**
     * Base class for all exception types.
     */
    class Exception {
       public:
        Exception() noexcept = default;
        virtual ~Exception() noexcept;

        Exception(const Exception&) = default;
        Exception& operator=(const Exception&) = default;

        Exception(Exception&&) = default;
        Exception& operator=(Exception&&) = default;

        /**
         * Get the message of the exception.
         */
        [[nodiscard]] virtual const char* GetMessage() const noexcept;
    };

    /**
     * Exception thrown by dynamic_cast when a bad reference cast happen.
     */
    F_TRIVIAL_EXCEPTION(BadCastException, "FunnyOS::Stdlib::System::Exception");

    /**
     * An exception of this type is thrown when a typeid operator is applied to a dereferenced null pointer value.
     */
    F_TRIVIAL_EXCEPTION(BadTypeidException, "FunnyOS::Stdlib::System::BadTypeidException");

    /**
     * Exception thrown by the allocation functions to report failure to allocate storage.
     */
    F_TRIVIAL_EXCEPTION(BadAllocation, "FunnyOS::Stdlib::System::BadAllocation");

    /**
     * Thrown by F_ASSERT if an assertion fails and F_ASSERTIONS_THROW_EXCEPTIONS is defined.
     */
    F_TRIVIAL_EXCEPTION_WITH_MESSAGE(AssertionFailure);

#ifdef F_LL
    namespace _Exception {
        Exception* GetAndClearActiveException();

        Exception* SetActiveException(Exception* ptr);
    }  // namespace _Exception
#endif
}  // namespace FunnyOS::Stdlib::System

#endif  // FUNNYOS_STDLIB_HEADERS_FUNNYOS_STDLIB_SYSTEM_HPP
