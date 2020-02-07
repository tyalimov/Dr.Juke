#include "net_helper.h"
#include <EASTL/list.h>
#include "util/preferences.h"
#include "common.h"

using namespace eastl;

using u32 = unsigned int;
using u16 = unsigned short;
using u8 = unsigned char;


    HexDeserializer::HexDeserializer(const char* buffer, ULONG length) 
    {
        if (length == 0)
            return;

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
        if (length == 0)
            return;

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

    // EF FF FF FA <- 239.255.255.250
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
                if (x >= 0 && x <= (u16)-1)
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
            string content;

            chunks.pop_front();
            value = chunks.front();
            res = parse_eq_str(value, L"offset", &offset);
            if (res)
                return STATUS_UNSUCCESSFUL;

            // 7 content |001100|
            chunks.pop_front();
            value = chunks.front();

            // skip terminating char
            value.pop_back();
            
            auto length = value.length();
            if ((length >= 4) && (length % 2 == 0))
            {
                if (value.front() == L'|' && value.back() == L'|')
                    value = wstring(value.begin() + 1, value.end() - 1);
                else
                    return STATUS_UNSUCCESSFUL;

                content = str_util::ToString(value);

                bool ok;
                for (const char& x : content)
                {
                    ok = (x >= '0' && x <= '9'
                        || x >= 'A' && x <= 'F');

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

    void NetFilterRuleList::regReadConfiguration() 
    {
		regReadRules([this](PWCH Name,
			ULONG NameLen, PVOID Data, ULONG DataLen, ULONG Type)
			{
				modifyRule(Name, NameLen, Data, DataLen, Type,
					[this](const wstring& rule_name, NetFilterRule* rule_obj) {

                        if (rule_obj != nullptr)
						    this->addRule(rule_name, rule_obj);

					});
			});

		if (mRuleMap.size() == 0)
			kprintf(TRACE_NETFILTER_WARN, "Configuration: No filtering rules found!");
    }

    void NetFilterRuleList::onRegKeyChange(const wstring& KeyPath,
        PREG_SET_VALUE_KEY_INFORMATION PreInfo, BOOLEAN bDeleted)
    {
        PUNICODE_STRING ValueName = PreInfo->ValueName;
        kprintf(TRACE_NETFILTER_INFO, "Key path = %ws", KeyPath.c_str());
        if (KeyPath == mKeyBase + mKeyRules)
        {
            if (bDeleted)
            {
                modifyRule(ValueName->Buffer, ValueName->Length,
                    PreInfo->Data, PreInfo->DataSize, PreInfo->Type,
                    [this](const wstring& rule_name, NetFilterRule* rule) {

                        UNREFERENCED_PARAMETER(rule);
                        this->removeRule(rule_name);

                    });
            }
            else
            {
                modifyRule(ValueName->Buffer, ValueName->Length,
                    PreInfo->Data, PreInfo->DataSize, PreInfo->Type,
                    [this](const wstring& rule_name, NetFilterRule* rule) {

                        if (rule != nullptr)
                            this->addRule(rule_name, rule);

                    });
            }
        }
    }

    void NetFilterRuleList::addRule(const wstring& name, NetFilterRule* rule)
    {
        rule->setName(name);
        mRuleLock.acquire();

        // insert priority
        priority_t pri = rule->getPriority();
		auto result = mNamePriorityMap.try_emplace(name, pri);
		bool inserted = result.second;
		auto it = result.first;

        // if priority has changed in existing rule
        // we must remove record from priority map
        if (!inserted)
        {
            mRuleMap.erase(it->second);
            it->second = pri;
        }

        // insert rule
		auto result2 = mRuleMap.try_emplace(pri, *rule);
		bool inserted2 = result2.second;
		auto it2 = result2.first;

		if (!inserted2)
			it2->second = *rule;

        mRuleLock.release();

		if (inserted)
		{
			kprintf(TRACE_NETFILTER_INFO, "Added rule <Name=%ws>, <Priority=0x%04X>",
				name.c_str(), pri);
		}
		else
		{
			kprintf(TRACE_NETFILTER_INFO, "Modified <Name=%ws>, <Priority=0x%04X>",
				name.c_str(), pri);
		}

    }

    void NetFilterRuleList::removeRule(const wstring& name)
    {
        eastl_size_t n = 0;

		mRuleLock.acquire();

        auto it = mNamePriorityMap.find(name);
        if (it != mNamePriorityMap.end())
        {
            priority_t pri = it->second;
            n = mRuleMap.erase(pri);
            mNamePriorityMap.erase(it);
        }

		mRuleLock.release();

		if (n > 0)
			kprintf(TRACE_NETFILTER_INFO, "Removed rule <Name=%ws>", name.c_str());
		else
			kprintf(TRACE_NETFILTER_ERROR, "Attempt to remove "
				"non existent rule <Name=%ws>", name.c_str());
    }

	void NetFilterRuleList::modifyRule(PWCH Name, ULONG NameLen, PVOID Data, ULONG DataLen, 
        ULONG Type, function<void(const wstring&, NetFilterRule*)> cb)
	{
		wstring rule_name(Name, NameLen / sizeof(WCHAR));

        if (Type == REG_NONE)
            cb(rule_name, nullptr);
        else if (Type == REG_SZ)
        {
            wstring rule((PWCH)Data, DataLen / sizeof(WCHAR));

            NetFilterRule ruleObj;
            if (NT_SUCCESS(parse_rule(rule, &ruleObj)))
                cb(rule_name, &ruleObj);
            else
            {
				kprintf(TRACE_NETFILTER_ERROR, "Bad format. "
					"Unable to parse rule <Name=%ws>", rule_name.c_str());
            }

        }
		else
		{
			kprintf(TRACE_NETFILTER_ERROR, "Type != REG_SZ. "
				"Nothing to do with record <Name=%ws>", rule_name.c_str());
		}
	}

    void NetFilterRuleList::regReadRules(function<void(PWCH Name, ULONG NameLen,
        PVOID Data, ULONG DataLen, ULONG Type)> cbAddRule)
	{
		NTSTATUS status;
		wstring proc_key = mKeyBase + mKeyRules;

		status = PreferencesReadFull(proc_key.c_str(), cbAddRule);
		if (!NT_SUCCESS(status))
		{
			kprintf(TRACE_NETFILTER_ERROR, "Failed to read " 
                "rules registry key <KeyPath=%ws>", proc_key.c_str());
		}
	}
