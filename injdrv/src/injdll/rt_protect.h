#pragma once

#include "string.h"
#include "list.h"

using namespace mstl;

namespace mwtricks
{
	using uint = unsigned int;

	class FunctionCall
	{

	private:
		string m_function_name;
		uint m_num_args;
		uint m_num_args_required;
		Argument* m_args;

	public:

		void* getArgument(uint i)
		{
			if (i < m_num_args)
				return m_args[i].value;

			return nullptr;
		}

		bool isArgumentRequired(uint i)
		{
			if (i < m_num_args)
				return m_args[i].isRequired;

			return nullptr;
		}

		bool getArgumentRequiredCnt() {
			return m_num_args_required;
		}

		const string& getFunctionName() {
			return m_function_name;
		}

		uint getArgumentCnt() {
			return m_num_args;
		}

		void setArgument(void* arg, uint i, bool isRequired = false)
		{
			if (i < m_num_args)
			{
				m_args[i].isRequired = isRequired;
				m_args[i].value = arg;
			}
		}

		FunctionCall(const string& function_name, uint num_args)
		{
			m_num_args = num_args;
			m_function_name = function_name;

			m_args = new Argument[num_args];
			for (uint i = 0; i < m_num_args; i++)
			{
				m_args[i].value = 0;
				m_args[i].isRequired = false;
			}
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
		list<FunctionCall> m_trick;

		void addFunctionCall(const FunctionCall& call)
		{
			m_trick.push_back(call);
		}

		bool isMalwareTrick(FunctionCall& target, FunctionCall& hooked)
		{
			if (target.getFunctionName() == hooked.getFunctionName())
			{
				uint num_args = target.getArgumentCnt();
				if (num_args == hooked.getArgumentCnt())
				{
					for (uint i = 0; i < num_args; i++)
					{
						if (target.isArgumentRequired(i))
						{

						}
					}
				}
			}
		}

		void updateCurrentStage(const FunctionCall& call)
		{

		}
	};

	class MalwareTricks
	{
		using MalwareTrick = list<FunctionCall>;
		list<MalwareTrick> m_tricks;



	};
}

