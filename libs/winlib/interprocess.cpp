#include "interprocess.h"

#include <map>

namespace drjuke::winlib
{
    namespace
    {
        std::map<Destination, std::string> g_Adresses
        {
            { Destination::kScanService,   R"(\\.\pipe\ScanService)"     },
            { Destination::kUpdateService, R"(\\.\pipe\UpdateService)"   }
        };
    }
}
