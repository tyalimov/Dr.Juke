// drjuke_pyport.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "drjuke_pyport.h"


// This is an example of an exported variable
DRJUKEPYPORT_API int ndrjukepyport=0;

// This is an example of an exported function.
DRJUKEPYPORT_API int fndrjukepyport(void)
{
    return 0;
}

// This is the constructor of a class that has been exported.
Cdrjukepyport::Cdrjukepyport()
{
    return;
}
