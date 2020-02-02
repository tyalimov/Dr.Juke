#pragma once

#include <common/aliases.h>
#include "netlib/netlib.h"

using namespace drjuke;

void DownloaderThread(const std::map<std::string, std::pair<std::string, uint32_t>>& filenames, 
                      const Path& destination);

void ProgressBarThread();

