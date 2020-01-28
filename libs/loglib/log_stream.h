#pragma once

#include <sstream>
#include <iomanip>
#include <vector>
#include <set>

#include <boost/format/format_fwd.hpp>

namespace drjuke::loglib
{
    namespace detail 
    {
        typedef std::ios_base& (__CLRCALL_OR_CDECL* ios_base_manip)(std::ios_base&);
    }

class LogStream
{
public:
    LogStream& operator << (char ch);
    LogStream& operator << (wchar_t ch);
    LogStream& operator << (unsigned char ch);
    LogStream& operator << (char* val);
    LogStream& operator << (const char* val);
    LogStream& operator << (wchar_t* val);
    LogStream& operator << (const wchar_t* val);
    LogStream& operator << (void* val);
    LogStream& operator << (const std::string& val);
    LogStream& operator << (const std::wstring& val);
    LogStream& operator << (detail::ios_base_manip val);
    LogStream& operator << (bool val);
    LogStream& operator << (short val);
    LogStream& operator << (unsigned short val);
    LogStream& operator << (int val);
    LogStream& operator << (unsigned int val);
    LogStream& operator << (long val);
    LogStream& operator << (unsigned long val);
    LogStream& operator << (__int64 val);
    LogStream& operator << (unsigned __int64 val);
    LogStream& operator << (float val);
    LogStream& operator << (double val);
    LogStream& operator << (long double val);

    template <typename CharT, typename Tr, typename _Ax>
    LogStream& operator<< (const boost::basic_format<CharT, Tr, _Ax>& fmt)
    {
        return *this << fmt.str();
    }	

    template <typename _Elem>
    LogStream& operator<< (const std::_Smanip<_Elem>& val)
    {
        (*val._Pfun)(m_stream, val._Manarg);
        return *this;
    }

    LogStream& operator << (const std::_Fillobj<char>& _Manip);
    LogStream& operator << (const std::_Fillobj<wchar_t>& _Manip);
    LogStream& operator << (LogStream& (__CLRCALL_OR_CDECL* fp)(LogStream&));

    template <typename Ty>
    LogStream& operator<< (const std::vector<Ty>& v)
    {
        using std::operator<<;

        for(auto& element: v)
            *this << element << L" ";
        return *this;
    }

    template <typename Ty>
    LogStream& operator<< (const std::set<Ty>& v)
    {
        using std::operator<<;

        for(auto& element: v)
            *this << element << L" ";
        return *this;
    }

    template <typename Ty1, typename Ty2>
    LogStream& operator<< (const std::pair<Ty1, Ty2>& v)
    {
        using std::operator<<;

        ((*this << v.first) << L" " ) << v.second;
        return *this;
    }

    template <typename Ty>
    LogStream& operator<< (const Ty& v)
    {
        using std::operator<<;

        m_stream << v;
        return *this;
    }


    void endl();
    void ends();

    [[nodiscard]] std::wstring str() const;

protected:
    std::wstringstream m_stream;
};
}

namespace std 
{
    drjuke::loglib::LogStream& __CLRCALL_OR_CDECL endl(drjuke::loglib::LogStream& f);
    drjuke::loglib::LogStream& __CLRCALL_OR_CDECL ends(drjuke::loglib::LogStream& f);
}