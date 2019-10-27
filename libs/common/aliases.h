// Задаем forward declaration для того, 
// чтобы не инклудить хедер для каждого типа,
// тем самым привнося с собой половину стандартной
// библиотеки. Иначе компилятор начнет гадить кирпичами
// при каждой сборке.

#include <nlohmann/json_fwd.hpp>

namespace std::filesystem
{
    class path;
    class recursive_directory_iterator;
}

// Непосредственно более лаконичные алиасы
namespace drjuke
{
    using Json        = nlohmann::json;
    using Path        = std::filesystem::path;
    using DirIterator = std::filesystem::recursive_directory_iterator;
}