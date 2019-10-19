#include "basic_executor.h"

namespace drjuke::threading
{
	void Executor(TaskQueue& queue)
	{
		while (true)
		{
            if (queue.isFinalized())
            {
				break;
            }
			else
			{
				queue.popTask()->execute();
			}
		}
	}
}