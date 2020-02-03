#include "base_task.h"

namespace drjuke::tasklib
{
    void EndTask::execute()
    {
    }

    bool EndTask::isEndTask()
    {
        return true;
    }
}
