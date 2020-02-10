#pragma once

#include "util/util.h"
#include <EASTL/map.h>

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
    
    u16 m_port_from = 0;
    u16 m_port_to = 0;

    u32 m_ip_from = 0;
    u32 m_ip_to = 0;

    u16 m_offset = 0;
    eastl::string m_content;
    eastl::wstring m_name;

    static const u16 mc_port_any = 0;
    static const u16 mc_ip_any = 0;

public:

    NetFilterRule() = default;
    ~NetFilterRule() = default;

    NetFilterRule(const NetFilterRule& other) = default;

    bool isPacketAllowed() const {
        return m_allow;
    }

    bool isProtocolMatch(u8 protocol) const 
    {
        return m_protocol == protocol 
            || m_protocol == IPV4;
    }

    bool isIpFromMatch(u32 ip) const 
    {
        return m_ip_from == ip 
            || m_ip_from == mc_ip_any;
    }

    bool isIpToMatch(u32 ip) const 
    {
        return m_ip_to == ip 
            || m_ip_to == mc_ip_any;
    }

    bool isPortFromMatch(u32 port) const
    {
        return m_port_from == port
            || m_port_from == mc_port_any;
    }

    bool isPortToMatch(u32 port) const
    {
        return m_port_to == port
            || m_port_to == mc_port_any;
    }

    bool isContentMatch(const char* buffer, size_t length) const
    {
        size_t content_length = m_content.length();

        if (content_length == 0)
            return true;

        if (length > m_offset)
        {
            if (length - m_offset > content_length)
            {
                return !strncmp(&buffer[m_offset], 
                    m_content.c_str(), m_content.length());
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
        m_ip_from = local_ip;
    }

    void setRemoteIp(u32 remote_ip) {
        m_ip_to = remote_ip;
    }

    void setLocalPort(u16 local_port) {
        m_port_from = local_port;
    }

    void setRemotePort(u16 remote_port) {
        m_port_to = remote_port;
    }

    void setContent(const char* content, size_t len, u16 offset) 
    {
        m_content = eastl::string(content, len);
        m_offset = offset;
    }

    void setPriority(u16 priority) {
        m_priority = priority;
    }

    void setName(const wstring& name) {
        m_name = name;
    }

    u16 getPriority() const {
        return m_priority;
    }

    const wchar_t* getName() const {
        return m_name.c_str();
    }
};

class NetFilterRuleList
{

public:

    using priority_t = unsigned short;

private:

	using u32 = unsigned int;
	using u16 = unsigned short;
	using u8 = unsigned char;

    const wstring mKeyRules = L"\\EnabledRules";
    wstring mKeyBase;

    map<wstring, priority_t> mNamePriorityMap;
    map<priority_t, NetFilterRule> mRuleMap;

    SpinLock mRuleLock;

public:

	// Forbid copy/move
    NetFilterRuleList(const NetFilterRuleList&) = delete;
    NetFilterRuleList(const NetFilterRuleList&&) = delete;

    NetFilterRuleList(const wchar_t* BaseKey) 
        : mKeyBase(BaseKey) {}

    ~NetFilterRuleList() = default;

    void regReadConfiguration();

    void onRegKeyChange(const wstring& KeyPath,
        PREG_SET_VALUE_KEY_INFORMATION PreInfo, BOOLEAN bDeleted);

    void addRule(const wstring& name, NetFilterRule* rule);

    void removeRule(const wstring& name);

    void modifyRule(PWCH Name, ULONG NameLen, PVOID Data, ULONG DataLen,
        ULONG Type, function<void(const wstring&, NetFilterRule*)> cb);

    void regReadRules(function<void(PWCH Name, ULONG NameLen,
        PVOID Data, ULONG DataLen, ULONG Type)> cbAddProtectedObject);

    const wchar_t* findMatchingRule(u8 prot, u32 ip_from, u16 port_from, 
        u32 ip_to, u16 port_to, const char* content, size_t length, bool* allowed)
    {
        const wchar_t* rule_name = nullptr;
        bool found;

        mRuleLock.acquire();

        for (const auto& elem : mRuleMap)
        {
            const NetFilterRule& rule = elem.second;

            if (!rule.isProtocolMatch(prot))
                continue;

            if (!rule.isIpFromMatch(ip_from))
                continue;

            if (!rule.isPortFromMatch(port_from))
                continue;

            if (!rule.isIpToMatch(ip_to))
                continue;

            if (!rule.isPortToMatch(port_to))
                continue;

            // isContentMatch returns true if m_content is empty
            found = rule.isContentMatch(content, length);

            if (found)
            {
                *allowed = rule.isPacketAllowed();
                rule_name = rule.getName();
                break;
            }
        }

        mRuleLock.release();

        return rule_name;
    }

};

using PNetFilterRuleList = NetFilterRuleList*;

PNetFilterRuleList NetFilterGetInstancePtr();

NTSTATUS parse_rule(const eastl::wstring& rule, NetFilterRule* ruleObj);
