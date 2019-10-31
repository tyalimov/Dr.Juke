#pragma once

#include "string.h"
#include "list.h"
#include "synch.h"

using namespace ownstl;

namespace mwtricks
{
	class FunctionCall
	{

	private:

		using uint = unsigned int;
		uint m_num_args;

		string m_function_name;
		void** m_args;

	public:

		void* getArgument(uint i)
		{
			return m_args[i];
		}

		uint getArgumentCnt() {
			return m_num_args;
		}

		const string& getName() {
			return m_function_name;
		}

		void setArgument(void* arg, uint i)
		{
			if (i < m_num_args)
				m_args[i] = arg;
		}

		FunctionCall(const string& function_name, uint num_args)
		{
			m_num_args = num_args;
			m_function_name = function_name;

			m_args = new void* [num_args];
			memset(m_args, 0, num_args);
		}

		~FunctionCall()
		{
			if (m_args)
			{
				delete[] m_args;
				m_args = nullptr;
			}

			m_num_args = 0;
		}
	};

	class MalwareTrick
	{
	public:

		using CheckCallback = bool (*)(const FunctionCall& call);

	private:

		list<CheckCallback> m_trick;
		CriticalSection crit;
		string m_trick_name;

	public:

		MalwareTrick(const char* trick_name) {
			m_trick_name = trick_name;
		}

		void addNewCheck(CheckCallback cb) {
			m_trick.push_back(cb);
		}

		bool updateCurrentStage(const FunctionCall& target)
		{
			crit.acquire();

			CheckCallback cb = m_trick.get_first();
			if (cb(target))
				m_trick.pop_first();

			crit.release();
			return m_trick.size() == 0;
		}

		const string& getName() {
			return m_trick_name;
		}

		MalwareTrick() = default;
		~MalwareTrick() = default;
	};

	class MalwareTricks
	{
	public:

		using AlertCallback = bool (*)(const string& trick_name);

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

		bool updateCurrentStage(FunctionCall& target)
		{
			for (auto& trick : m_tricks)
			{
				if (trick.updateCurrentStage(target))
					m_alert_cb(trick.getName());
			}
		}

	};
}

