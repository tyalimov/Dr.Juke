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
		arg_t m_args[N];

		template <int i, typename T = void>
		constexpr void setArg() {}

		template <int i, typename T, typename ... Types>
		constexpr void setArg(T arg, Types... rest)
		{
			m_args[i] = (arg_t)arg;
			setArg<i + 1, Types...>(rest...);
		}

		string m_function_name;

	public:

		template <typename ...Types>
		FunctionCall(string name, Types... args)
		{
			static_assert(sizeof...(args) <= N);
			setArg<0, Types...>(args...);
			m_num_args = sizeof...(args);
			m_function_name = name;
		}

		~FunctionCall() = default;

		arg_t getArgument(unsigned i) const
		{
			return m_args[i];
		}

		unsigned getArgumentCnt() const {
			return m_num_args;
		}

		const string& getName() const {
			return m_function_name;
		}
	};

	class MalwareTrick
	{
	public:

		using CheckCallback = bool (*)(const FunctionCall& call);

	private:

		list<CheckCallback> m_trick;
		CriticalSection m_crit;
		string m_trick_name;
		bool m_triggered = false;

	public:

		MalwareTrick(const char* trick_name) {
			m_trick_name = trick_name;
		}

		MalwareTrick() = default;
		~MalwareTrick() = default;

		void addCheck(CheckCallback cb) {
			m_trick.push_back(cb);
		}

		bool updateCurrentStage(const FunctionCall& target)
		{
			if (m_triggered)
				return false;

			m_crit.acquire();

			CheckCallback cb = m_trick.get_first();
			if (cb(target))
				m_trick.pop_first();

			m_crit.release();

			m_triggered = m_trick.size() == 0;
			return m_triggered;
		}

		const string& getName() {
			return m_trick_name;
		}
	};

	class MalwareTricks
	{
	public:

		using AlertCallback = void (*)(const string& trick_name);

	private:

		list<MalwareTrick> m_tricks;
		AlertCallback m_alert_cb = nullptr;

	public:

		void addNewTrick(MalwareTrick trick) {
			m_tricks.push_back(trick);
		}

		void onMalwareDetected(AlertCallback callback) {
			m_alert_cb = callback;
		}

		void updateCurrentStage(FunctionCall& target)
		{
			for (auto& trick : m_tricks)
			{
				if (trick.updateCurrentStage(target))
				{
					if (m_alert_cb != nullptr)
						m_alert_cb(trick.getName());
				}
			}
		}

	};
}

