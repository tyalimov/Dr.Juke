// Задаем forward declaration для того, 
// чтобы не инклудить хедер для каждого типа,
// тем самым привнося с собой половину стандартной
// библиотеки. Иначе компилятор начнет гадить кирпичами
// при каждой сборке.

class std::filesystem::path;
class nlohmann::json;
class std::filesystem::recursive_directory_iterator;

// Непосредственно более лаконичные алиасы
namespace drjuke
{
    using Json        = nlohmann::json;
    using Path        = std::filesystem::path;
    using DirIterator = std::filesystem::recursive_directory_iterator;
}