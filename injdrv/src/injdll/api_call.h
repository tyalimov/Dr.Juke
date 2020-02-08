#pragma once

enum class CallId
{
	ntdll_LdrLoadDll,
	ntdll_LdrGetDllHandle,
	ntdll_NtWriteProcessMemory,
	ntdll_NtReadProcessMemory,
	ws2_32_connect,
	ws2_32_recv,
	ws2_32_send,
	ws2_32_socket,
	ws2_32_closesocket,
};

class ApiCall
{

public:

	using arg_t = uintptr_t;

private:

	static const int N = 16;

	unsigned m_num_args = 0;
	bool m_is_pre = true;
	bool m_skip_call = false;

	CallId m_call_id;
	arg_t m_ret_val = 0;
	arg_t m_args[N];

	template <int i, typename T = void>
	constexpr void setArg() {}

	template <int i, typename T, typename ... Types>
	constexpr void setArg(T arg, Types... rest)
	{
		m_args[i] = (arg_t)arg;
		setArg<i + 1, Types...>(rest...);
	}

public:

	template <typename ...Types>
	ApiCall(CallId id, bool is_pre, Types... args)
	{
		static_assert(sizeof...(args) <= N);
		setArg<0, Types...>(args...);
		m_num_args = sizeof...(args);
		m_call_id = id;
		m_is_pre = is_pre;
	}

	~ApiCall() = default;

	arg_t getArgument(unsigned i) const {
		return m_args[i];
	}

	arg_t getReturnValue() const {
		return m_ret_val;
	}

	void setReturnValue(arg_t value) {
		m_ret_val = value;
	}

	void skipCall() {
		m_skip_call = true;
	}

	bool isCallSkipped() const {
		return m_skip_call;
	}

	CallId getCallId() {
		return m_call_id;
	}

	bool isPre() const {
		return m_is_pre;
	}

	bool isPost() const {
		return !m_is_pre;
	}

	unsigned getArgumentCnt() const {
		return m_num_args;
	}
};

typedef void(*api_call_t)(ApiCall*);
