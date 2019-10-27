#include <string>

// Предназначено для целочисленных констант.
#define BEGIN_NUMERIC_SPACE(Space) \
    namespace Space { enum {

#define END_NUMERIC_SPACE() \
    }; }

// Предназначено для строковых констант.
#define BEGIN_STRING_SPACE(Space) \
    namespace Space { 

#define END_STRING_SPACE() \
    } 

namespace drjuke::constants
{
    BEGIN_NUMERIC_SPACE(tasklib)
        kMaxTaskQueueSize = 1000
    END_NUMERIC_SPACE()

    BEGIN_STRING_SPACE(tasklib)
        static const std::string kJsonTaskId = "task_id";
    END_STRING_SPACE()

    BEGIN_STRING_SPACE(scansvc)
        // TODO: Каким-то хером задать относительный путь. Скорее всего 
        // в будущем ты будешь знать расположение папки с ресурсами 
        // относительно себя
        static const std::string kYaraRulesLocation = R"(D:\Dr.Juke_resources\CVE_Rules)";
    END_STRING_SPACE()
}