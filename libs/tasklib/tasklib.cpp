#include "tasklib.h"

namespace drjuke::tasklib
{
    TaskQueuePtr Factory::getTaskQueue()
    {
        return std::make_shared<TaskQueue>();
    }
}
