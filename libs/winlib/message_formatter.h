#pragma once

#include <cstdint>
#include <string>

namespace drjuke::winlib
{
    class MessageFormatter
    {
    private:
        uint32_t    m_error_code;
        std::string m_formatted_error_code;
        std::string m_library_message;
        std::string m_stacktrace;
        std::string m_formatted_message;

        void formatErrorCode();
        void addLibraryMessage();
        void addFormattedErrorCode();
        void addStackTrace();

    public:

        MessageFormatter(const std::string& library_message,
                         uint32_t errror_code);

        explicit MessageFormatter(const std::string& stacktrace);

        [[nodiscard]]std::string getMessage() const;
    };
}
