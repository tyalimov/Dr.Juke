#pragma once

#include "base_task.h"
#include "task_queue.h"

namespace drjuke::tasklib
{
    class Factory
    {
    public:
        static TaskQueuePtr getTaskQueue();
    };
}