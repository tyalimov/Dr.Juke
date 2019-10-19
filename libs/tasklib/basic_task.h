#pragma once

#include <json.hpp>

namespace drjuke::threading
{
	class BasicTask
	{
	public:
		using Json = nlohmann::json;
	protected:
		Json m_input;
	public:

        explicit BasicTask(const Json &input)
			: m_input(input)
		{}


		virtual void execute() = 0;
		virtual ~BasicTask() = default;
	};

    class StubTask : public BasicTask
    {
    public:

		explicit StubTask()
			: BasicTask(Json({ {"str", "qwerty"} }))
		{}

		void execute() override {}
    };

	using TaskPtr = std::shared_ptr<BasicTask>;
}