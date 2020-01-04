#include <string>

namespace drjuke::constants
{
    namespace tasklib
    {
        constexpr uint32_t kMaxTaskQueueSize = 1000;
    }

    namespace scanlib
    {
        // TODO: Перенести это в settingslib
        // TODO: Также убрать хардкод и делать через получение путей из реестра
        constexpr std::string_view kYaraRulesLocation   = R"(D:\Dr.Juke_resources\CVE_Rules)";
        constexpr std::string_view kClamAvRulesLocation = R"(D:\Dr.Juke_resources\ClamAV_Rules)";
        constexpr std::string_view kPeidRulesLocation   = R"(D:\Dr.Juke_resources\PEiD_Rules)";
    }
}