#include "task_builder.h"

namespace drjuke::service
{
    tasklib::BaseTaskPtr TaskBuilder::getTask(const Json& /*message*/)
    {
        //auto name = message["task"].get<std::string>();
        //auto id = m_ids.at(name);
        return std::make_shared<tasklib::EndTask>();
        //switch (id)
        //{
        //    //case TaskId::Scan : return std::make_shared<ScanTask>(message); break;
        //}
    }
}
