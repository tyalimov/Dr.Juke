#pragma once

#include <exception>
#include <string>
#include <vector>

namespace drjuke::winlib
{
    struct WindowsStackFrame
    {
        WindowsStackFrame()
            : m_name("")
            , m_address(0)
        {}

        WindowsStackFrame(const std::string& name,
                          const std::string& module,
                          std::uint64_t      address)
            : m_name(name)
            , m_module(module)
            , m_address(address)
        {}

        std::string m_name;
        std::string m_module;
        std::uint64_t m_address;
    };

    class WindowsException final
        : public std::exception
    {
    private:
        std::string                    m_error_message;
        std::vector<WindowsStackFrame> m_stack_trace;
        std::uint32_t                  m_error_code;

        void initializeMessage(const char *message);
        void initializeStackTrace();

    public:
        WindowsException(const char *message);
        ~WindowsException() override = default;
        const char* what() const noexcept override;
        std::string dumpStackTrace() const;
    };
}
