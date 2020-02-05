#pragma once

#include <map>
#include <string>
#include <tasklib/base_task.h>
#include <common/aliases.h>

namespace drjuke::service
{
    enum class TaskId;

    class TaskBuilder
    {
        static std::map<std::string, TaskId> m_ids;

        tasklib::BaseTaskPtr getTask(const Json& message);
    };
}