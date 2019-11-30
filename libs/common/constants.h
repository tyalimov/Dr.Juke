#include <string>

namespace drjuke::constants
{
    namespace tasklib
    {
        constexpr uint32_t kMaxTaskQueueSize = 1000;
    }

    namespace scansvc
    {
        // TODO: Каким-то хером задать относительный путь. Скорее всего 
        // в будущем ты будешь знать расположение папки с ресурсами 
        // относительно себя
        constexpr std::string_view kYaraRulesLocation = R"(D:\Dr.Juke_resources\CVE_Rules)";
    }
}