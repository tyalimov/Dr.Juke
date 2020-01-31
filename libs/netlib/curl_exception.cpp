#include "curl_exception.h"

namespace drjuke::netlib
{
    CurlException::CurlException(CURLcode error_code)
        : m_message("")
        , m_error_code(error_code)
    {
        m_message += "Curl failed with: ";
        m_message += std::to_string(error_code);
        m_message += " | ";
        m_message += curl_easy_strerror(m_error_code);
    }

    const char *CurlException::what() const noexcept
    {
        return m_message.c_str();
    }
}
