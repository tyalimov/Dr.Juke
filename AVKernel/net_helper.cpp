#include "net_helper.h"
#include "EASTL/list.h"
#include "util.h"

using namespace eastl;

using u32 = unsigned int;
using u16 = unsigned short;
using u8 = unsigned char;


    HexDeserializer::HexDeserializer(const char* buffer, ULONG length) 
    {
        mLength = length * 2;
		mBuffer = new char[mLength + 1];

		if (mBuffer)
		{
			for (ULONG i = 0, j = 0; i < length; i++, j += 2)
				int2hex(buffer[i], &mBuffer[j]);

			mBuffer[mLength] = '\0';
		}
    }

    HexDeserializer::~HexDeserializer()
    {
        if (mBuffer)
        {
            delete[] mBuffer;
            mBuffer = nullptr;
        }
    }

    HexSerializer::HexSerializer(const char* buffer, ULONG length)
    {
        mLength = length / 2;
		mBuffer = new char[mLength + 1];
		
        if (mBuffer)
		{
			for (ULONG i = 0, j = 0; i < mLength; i++, j += 2)
				mBuffer[i] = hex2int(&buffer[j]);

			mBuffer[mLength] = '\0';
		}
    }

    HexSerializer::~HexSerializer()
    {
        if (mBuffer)
        {
            delete[] mBuffer;
            mBuffer = nullptr;
        }
    }


	// deny tcp 0.0.0.0:12 0.0.0.0:13 priority=2 offset=13 |00123456|
    // const wchar_t* r = L"deny tcp 255.255.255.250:65535 123.11.1.0:13 priority=7800 offset=65535 |0123456789ABCDEFabcdef|";

    using u32 = unsigned int;
    using u16 = unsigned short;
    using u8 = unsigned char;

	u8 wchar2dec(wchar_t c)
	{
        if (c >= '\u0030' && c <= '\u0039')
            return (u8)(c - '\u0030');

        return 0xFF;
	}

	list<wstring> split(const wstring& s, wchar_t spl)
	{
		size_t j = 0, i = 0;
		list<wstring> chunks;
		wstring sub;

		for (j = s.find(spl, 0); j != wstring::npos; j = s.find(spl, j + 1))
		{
			sub = s.substr(i, j - i);
			chunks.push_back(move(sub));
			i = j + 1;
		}

		sub = s.substr(i, s.length() - i);
		chunks.push_back(move(sub));

		return chunks;
	}

    // EF FF FF FA -> 239.255.255.250
    int parse_ip(const wstring& ip_string, u32* ip)
    {
        int i = 24;
        *ip = 0;

        auto chunks = split(ip_string, L'.');
        if (chunks.size() != 4)
            return -1;

        u8 d1, d2, d3, octet;
        for (const auto& ip_octet_s: chunks)
        {
            auto length = ip_octet_s.length();
            if (length == 1)
            {
                octet = wchar2dec(ip_octet_s[0]);
                if (octet == 0xFF)
                    return -1;
            }
            else if (length == 2)
            {
                d1 = wchar2dec(ip_octet_s[0]);
                d2 = wchar2dec(ip_octet_s[1]);

                if (d1 == 0xFF || d2 == 0xFF)
                    return -1;

                if ((int)d1 * 10 + (int)d2 > 0xFF)
                    return -1;

                octet = d1 * 10 + d2;
            }
            else if (length == 3)
            {
                d1 = wchar2dec(ip_octet_s[0]);
                d2 = wchar2dec(ip_octet_s[1]);
                d3 = wchar2dec(ip_octet_s[2]);

                if (d1 == 0xFF || d2 == 0xFF || d3 == 0xFF)
                    return -1;

                if ((int)d1 * 100 + (int)d2 * 10 + (int)d3 > 0xFF)
                    return -1;
                
                octet = d1 * 100 + d2 * 10 + d3;
            }
            else
                return -1;

            *ip = *ip + (octet << i);
            i -= 8;
        }

        return 0;
    }

    int parse_ip_port(const wstring& ip_port, u32* ip, u16* port)
    {
        auto chunks = split(ip_port, L':');
        if (chunks.size() != 2)
            return -1;

        wstring ip_str = chunks.front();
        if (ip_str.length() > 0)
        {
            if (ip_str == L"any")
                *ip = 0;
            else
            {
                if (parse_ip(ip_str, ip) == -1)
                    return -1;
            }
        }
        else
            return -1;

        chunks.pop_front();
        wstring port_str = chunks.front();

        if (port_str.length() > 0)
        {
            if (port_str == L"any")
                *port = 0;
            else
            {
                int p = _wtoi(port_str.c_str());

                if (p > 0 && p < 65536)
                    *port = (u16)p;
                else
                    return -1;
            }
        }

        return 0;
    }

    int parse_eq_str(const wstring& pri_str, const wstring& name, u16* out)
    {
        auto chunks = split(pri_str, '=');
        if (chunks.size() != 2)
            return -1;
        else
        {
            wstring value = chunks.front();
            if (value != name)
                return -1;
            else
            {
                chunks.pop_front();
                value = chunks.front();

                int x = _wtoi(value.c_str());
                if (x > 0 && x <= (u16)-1)
                    *out = (u16)x;
                else
                    return -1;

            }
        }

        return 0;
    }

    NTSTATUS parse_rule(const wstring& rule, NetFilterRule* ruleObj)
    {
        wstring value;
        u32 ip;
        u16 port, priority;
        size_t size;
        int res;

        auto chunks = split(rule, L' ');
        size = chunks.size();

        if (size != 5 && size != 7)
            return STATUS_UNSUCCESSFUL;

        // 1. permission allow / deny
        value = chunks.front();
        if (value == L"allow")
            ruleObj->setAllowPerm(true);
        else if (value == L"deny")
            ruleObj->setAllowPerm(false);
        else
            return STATUS_UNSUCCESSFUL;

        // 2. protocol tcp/udp/icmp/ip
        chunks.pop_front();
        value = chunks.front();
        if (value == L"tcp")
            ruleObj->setProtocol(TCP);
        else if (value == L"udp")
            ruleObj->setProtocol(UDP);
        else if (value == L"icmp")
            ruleObj->setProtocol(ICMPV4);
        else if (value == L"ip")
            ruleObj->setProtocol(IPV4);
        else
            return STATUS_UNSUCCESSFUL;

        // 3 ip:port from
        chunks.pop_front();
        value = chunks.front();

        res = parse_ip_port(value, &ip, &port);
        if (res)
            return STATUS_UNSUCCESSFUL;
        else
        {
            ruleObj->setLocalIp(ip);
            ruleObj->setLocalPort(port);
        }

        // 4 ip:port to
        chunks.pop_front();
        value = chunks.front();

        res = parse_ip_port(value, &ip, &port);
        if (res)
            return STATUS_UNSUCCESSFUL;
        else
        {
            ruleObj->setRemoteIp(ip);
            ruleObj->setRemotePort(port);
        }

        // 5 priority
        chunks.pop_front();
        value = chunks.front();
        res = parse_eq_str(value, L"priority", &priority);
        if (res)
            return STATUS_UNSUCCESSFUL;
        else
            ruleObj->setPriority(priority);

        if (size == 7)
        {
            // 6 offset
            u16 offset;
            wstring ws_content;
            string content;

            chunks.pop_front();
            value = chunks.front();
            res = parse_eq_str(value, L"offset", &offset);
            if (res)
                return STATUS_UNSUCCESSFUL;

            // 7 content |001100|
            chunks.pop_front();
            value = chunks.front();

            if (value.length() >= 4)
            {
                if (value.front() == L'|' && value.back() == L'|')
                    ws_content = wstring(value.begin() + 1, value.end() - 1);

                content = str_util::ToString(ws_content);

                if (content.length() % 2 != 0)
                    return STATUS_UNSUCCESSFUL;

                bool ok;
                for (const char& x : content)
                {
                    ok = (x >= '0' && x <= '9'
                        || x >= 'A' && x <= 'F'
                        || x >= 'a' && x <= 'f');

                    if (!ok)
                        return STATUS_UNSUCCESSFUL;
                }


                HexSerializer serializer(content.c_str(), (ULONG)content.length());

                ruleObj->setContent(serializer.getBinaryData(),
                    serializer.getLength(), offset);
            }
            else
                return STATUS_UNSUCCESSFUL;
        }

        return STATUS_SUCCESS;
    }

