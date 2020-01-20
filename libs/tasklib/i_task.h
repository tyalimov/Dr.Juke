#pragma once

#include <json/json.hpp>
#include <common/aliases.h>

namespace drjuke::tasklib
{
    class BaseTask
    {
    protected:
        Json m_input;
    public:
        explicit BaseTask(const Json &input)
            : m_input(input)
        {}

        virtual void verifyInput() = 0;
        virtual void execute()     = 0;
        virtual ~BaseTask()           = default;
    };

    class StubTask final 
        : public BaseTask
    {
    public:

        explicit StubTask()
            : BaseTask(Json{ { {"str", "qwerty"} } })
        {}

        void execute() override {}
        void verifyInput() override {}
    };

    using BaseTaskPtr = std::shared_ptr<BaseTask>;
}