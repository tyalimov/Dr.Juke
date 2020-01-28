#include "log_stream.h"
#include "string_conv.h"

namespace drjuke::loglib
{
    LogStream& LogStream::operator<< (char ch)
    {
        m_stream << ch;
        return *this;
    }

    LogStream& LogStream::operator<< (wchar_t ch)
    {
        m_stream << ch;
        return *this;
    }

    LogStream& LogStream::operator<< (unsigned char ch)
    {
        m_stream << ch;
        return *this;
    }

    LogStream& LogStream::operator<< (char* val)
    {
        if (val == nullptr)
        {
            m_stream << L"NULL";
            return *this;
        }

        const std::wstring ws = ToWstring(val);
        m_stream << ws;
        return *this;
    }

    LogStream& LogStream::operator<< (const char* val)
    {
        if (val == nullptr)
        {
            m_stream << L"NULL";
            return *this;
        }

        const std::wstring ws = ToWstring(val);
        m_stream << ws;
        return *this;
    }

    LogStream& LogStream::operator<< (wchar_t* val)
    {
        if (val == nullptr)
        {
            m_stream << L"NULL";
            return *this;
        }

        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (const wchar_t* val)
    {
        if (val == nullptr)
        {
            m_stream << L"NULL";
            return *this;
        }

        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (void* val)
    {
        if (val == nullptr)
        {
            m_stream << L"NULL";
            return *this;
        }

        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (const std::string& val)
    {
        return *this << val.c_str();
    }

    LogStream& LogStream::operator<< (const std::wstring& val)
    {
        return *this << val.c_str();
    }

    LogStream& LogStream::operator<< (detail::ios_base_manip val)
    {
        (*val)(m_stream);
        return *this;
    }

    LogStream& LogStream::operator<< (bool val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (short val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (unsigned short val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (int val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (unsigned int val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (long val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (unsigned long val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (__int64 val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (unsigned __int64 val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (float val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (double val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (long double val)
    {
        m_stream << val;
        return *this;
    }

    LogStream& LogStream::operator<< (const std::_Fillobj<char>& _Manip)
    {
        const char str[] = { _Manip._Fill, 0 };
        const wchar_t wch = ToWstring(str)[0];

        m_stream.fill(wch);
        return *this;
    }

    LogStream& LogStream::operator<< (const std::_Fillobj<wchar_t>& _Manip)
    {
        m_stream.fill(_Manip._Fill);
        return* this;
    }

    LogStream& LogStream::operator<< (LogStream& (__CLRCALL_OR_CDECL* fp)(LogStream&))
    {
        return fp(*this);
    }

    void LogStream::endl()
    {
        m_stream << std::endl;
    }

    void LogStream::ends()
    {
        m_stream << std::ends;
    }

    std::wstring LogStream::str() const
    {
        return m_stream.str();
    }
}
namespace std 
{
    drjuke::loglib::LogStream& __CLRCALL_OR_CDECL endl(drjuke::loglib::LogStream& f)
    {
        f.endl();
        return f;
    }

    drjuke::loglib::LogStream& __CLRCALL_OR_CDECL ends(drjuke::loglib::LogStream& f)
    {
        f.ends();
        return f;
    }
}