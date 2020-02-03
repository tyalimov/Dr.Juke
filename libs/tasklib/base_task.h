#pragma once

#pragma warning (push, 0)
#include <json/json.hpp>
#pragma warning (pop)

#include <common/aliases.h>

namespace drjuke::tasklib
{
    class BaseTask
    {
    protected:
        Json m_input;

    public:
        explicit BaseTask(const Json& input)
            : m_input(input)
        {}

        virtual void execute()   = 0;
        virtual bool isEndTask() = 0;
        virtual ~BaseTask()      = default;
    };

    using BaseTaskPtr = std::shared_ptr<BaseTask>;

    // Необходим для сообщения о том, что очередь
    // задач прекратила свою работу.
    class EndTask final
        : public BaseTask
    {
    public:

        explicit EndTask()
            : BaseTask(Json{})
        {}

        void execute() override;
        bool isEndTask() override;
    };
}