#pragma once

#include "common.h"

#define ETHERNET_ADDRESS_SIZE 6

#define IPV4_ADDRESS_SIZE  4
#define IPV6_ADDRESS_SIZE 16

#define IPV4 4
#define IPV6 6

#define IPV4_HEADER_MIN_SIZE 20
#define IPV6_HEADER_MIN_SIZE 40
#define ICMP_HEADER_MIN_SIZE  8
#define TCP_HEADER_MIN_SIZE  20
#define UDP_HEADER_MIN_SIZE   8

#define ICMPV4  1
#define TCP     6
#define UDP    17
#define ICMPV6 58

#define htonl(l)                  \
   ((((l) & 0xFF000000) >> 24) | \
   (((l) & 0x00FF0000) >> 8)  |  \
   (((l) & 0x0000FF00) << 8)  |  \
   (((l) & 0x000000FF) << 24))
 
#define htons(s) \
   ((((s) >> 8) & 0x00FF) | \
   (((s) << 8) & 0xFF00))
 
#define ntohl(l)                   \
   ((((l) >> 24) & 0x000000FFL) | \
   (((l) >>  8) & 0x0000FF00L) |  \
   (((l) <<  8) & 0x00FF0000L) |  \
   (((l) << 24) & 0xFF000000L))
 
#define ntohs(s)                     \
   ((USHORT)((((s) & 0x00ff) << 8) | \
   (((s) & 0xff00) >> 8)))



class HexDeserializer
{
	using u8 = unsigned char;
    char* mBuffer = nullptr;
    ULONG mLength = 0;

    inline char substitute(u8 i) {
        return (i <= 9 ? '0' + i : 'A' - 10 + i);
    }

    void int2hex(u8 in, char* out)
    {
		out[0] = substitute((in & 0xF0) >> 4);   
		out[1] = substitute(in & 0x0F);
    }

public:

    HexDeserializer(const char* buffer, ULONG length);

    ~HexDeserializer();

    PCHAR getHexText() {
        return mBuffer;
    }

    ULONG getLength() {
        return mLength;
    }
};

class HexSerializer
{
	using u8 = unsigned char;
    char* mBuffer = nullptr;
    ULONG mLength = 0;

    inline char substitute(u8 i) {
        return (i <= '9' ? i - '0' : i + 10 - 'A');
    }

    u8 hex2int(const char* in)
    {
        return (substitute(in[0]) << 4)
            + substitute(in[1]);
    }

public:

    HexSerializer(const char* buffer, ULONG length);

    ~HexSerializer();

    PCHAR getBinaryData() {
        return mBuffer;
    }

    ULONG getLength() {
        return mLength;
    }
};

class NetFilterRule
{
    using u32 = unsigned int;
    using u16 = unsigned short;
    using u8 = unsigned char;

    bool m_allow = true;
    u8 m_protocol = 0;
    u16 m_priority = 0;
    
    u16 m_local_port = 0;
    u16 m_remote_port = 0;

    u32 m_local_ip = 0;
    u32 m_remote_ip = 0;

    u16 m_offset = 0;
    eastl::string m_content;

    static const u16 mc_port_any = 0;
    static const u16 mc_max_offset = (u16)-1;

public:

    NetFilterRule() = default;
    ~NetFilterRule() = default;

    NetFilterRule(const NetFilterRule& other) = default;

    bool isAllowPerm() {
        return m_allow;
    }

    bool isProtocolMatch(u8 protocol) const {
        return m_protocol == protocol; // || m_protocol == IP
    }

    bool isLocalIpMatch(u32 ip) const {
        return m_local_ip == ip;
    }

    bool isRemoteIpMatch(u32 ip) const {
        return m_remote_ip == ip;
    }

    bool isLocalPortMatch(u32 port) const
    {
        return m_local_port == port
            || m_local_port == mc_port_any;
    }

    bool isRemotePortMatch(u32 port) const
    {
        return m_remote_port == port
            || m_remote_port == mc_port_any;
    }

    bool isContentMatch(const char* buffer, size_t length) const
    {
        if (!m_content.length() == 0)
            return true;

        size_t content_length = m_content.length();
        if (length > m_offset)
        {
            if (length - m_offset > content_length)
            {
                return !m_content.compare(m_offset,
                    content_length, buffer);
            }
        }

        return false;
    }

    void setAllowPerm(bool allow) {
        m_allow = allow;
    }

    void setProtocol(u8 protocol) {
        m_protocol = protocol;
    }

    void setLocalIp(u32 local_ip) {
        m_local_ip = local_ip;
    }

    void setRemoteIp(u32 remote_ip) {
        m_remote_ip = remote_ip;
    }

    void setLocalPort(u16 local_port) {
        m_local_port = local_port;
    }

    void setRemotePort(u16 remote_port) {
        m_remote_port = remote_port;
    }

    void setContent(const char* content, size_t len, u16 offset) 
    {
        m_content = eastl::string(content, len);
        m_offset = offset;
    }

    void setPriority(u16 priority) {
        m_priority = priority;
    }

    u16 getPriority() {
        return m_priority;
    }

};


NTSTATUS parse_rule(const eastl::wstring& rule, NetFilterRule* ruleObj);
