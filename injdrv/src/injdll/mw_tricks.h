#pragma once

#include "string.h"
#include "list.h"
#include "synch.h"

using namespace ownstl;

namespace mwtricks
{
	class FunctionCall
	{
	public:

		using arg_t = uintptr_t;

	private:

		static const int N = 16;
		unsigned m_num_args;
		bool m_is_pre = true;
		wstring m_function_name;
		arg_t m_ret_val = NULL;
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
		FunctionCall(wstring name, bool is_pre, arg_t ret_val, Types... args)
		{
			static_assert(sizeof...(args) <= N);
			setArg<0, Types...>(args...);
			m_num_args = sizeof...(args);
			m_function_name = name;
			m_ret_val = ret_val;
			m_is_pre = is_pre;
		}

		~FunctionCall() = default;

		arg_t getArgument(unsigned i) const	{
			return m_args[i];
		}

		arg_t getReturnValue() const {
			return m_ret_val;
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

		const wstring& getName() const {
			return m_function_name;
		}
	};

	class MalwareTrick
	{
	public:

		using CheckCallback = bool (*)(const FunctionCall& call);

	private:

		list<CheckCallback> m_trick;
		wstring m_trick_name;
		bool m_triggered = false;

	public:

		MalwareTrick(const wchar_t* trick_name) {
			m_trick_name = trick_name;
		}

		MalwareTrick() = default;
		~MalwareTrick() = default;

		MalwareTrick(const MalwareTrick& other)
		{
			m_trick_name = other.m_trick_name;
			m_triggered = other.m_triggered;
			m_trick = other.m_trick;
		}

		void addCheck(CheckCallback cb) {
			m_trick.push_back(cb);
		}

		bool updateCurrentStage(const FunctionCall& target)
		{
			if (m_triggered)
				return false;

			CheckCallback cb = m_trick.get_first();
			if (cb(target))
				m_trick.pop_first();

			m_triggered = m_trick.size() == 0;
			return m_triggered;
		}

		const wstring& getName() {
			return m_trick_name;
		}
	};

	class MalwareTrickChain
	{
	public:

		using AlertCallback = void (*)(const wstring& trick_name);

	private:

		list<MalwareTrick> m_tricks;
		AlertCallback m_alert_cb = nullptr;
		CriticalSection m_crit;

	public:

		void addTrick(const MalwareTrick& trick) {
			m_tricks.push_back(trick);
		}

		void onMalwareDetected(AlertCallback callback) {
			m_alert_cb = callback;
		}

		void updateCurrentStage(FunctionCall& target)
		{
			for (auto& trick : m_tricks)
			{
				m_crit.acquire();
				bool status = trick.updateCurrentStage(target);
				m_crit.release();

				if (status)
				{
					if (m_alert_cb != nullptr)
						m_alert_cb(trick.getName());
				}
			}
		}

	};
}

