#pragma once
#include <vector>
#include <common/aliases.h>
#include "netlib/netlib.h"

using namespace drjuke;

void DownloaderThread(const std::map<std::string, std::pair<std::string, uint32_t>>& filenames, 
                      const Path& destination, 
                      netlib::LoadingProgressPtr progress_bar);

void ProgressBarThread(netlib::LoadingProgressPtr progress_bar);

