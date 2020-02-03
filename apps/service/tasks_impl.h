#pragma once

#include <tasklib/base_task.h>

#define TASK_STD_CTOR(class_name)            \
    explicit class_name(const Json& message) \
        : BaseTask(message) {}

#define TASK_NOT_END_FUNCTION                \
    bool isEndTask() { return false; }

#define DECLARE_TASK(class_name)             \
    TASK_STD_CTOR(class_name)                \
    TASK_NOT_END_FUNCTION

using namespace drjuke::tasklib;

namespace drjuke::service
{
    class ScanTask final
        : public BaseTask
    {
    public:

        DECLARE_TASK(ScanTask)

        void execute() override
        {
            
        }
    };
}
