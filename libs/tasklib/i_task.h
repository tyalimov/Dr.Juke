#pragma once

#include <json.hpp>

#include <common/aliases.h>

namespace drjuke::threading
{
    class ITask
    {
    protected:

        Json m_input;
    public:

        explicit ITask(const Json &input)
            : m_input(input)
        {}

        explicit ITask()
            : m_input()
        {}

        void setInput(const Json& input) { m_input = input; }

        virtual void verifyInput() = 0;
        virtual void execute()     = 0;
        virtual ~ITask()           = default;
    };

    class StubTask : public ITask
    {
    public:

        explicit StubTask()
            : ITask(Json({ {"str", "qwerty"} }))
        {}

        void execute() override {}
        void verifyInput() override {}
    };

    using ITaskPtr = std::shared_ptr<ITask>;
}