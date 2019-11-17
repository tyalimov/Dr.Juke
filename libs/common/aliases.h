// Задаем forward declaration для того, 
// чтобы не инклудить хедер для каждого типа,
// тем самым привнося с собой половину стандартной
// библиотеки. Иначе компилятор начнет гадить кирпичами
// при каждой сборке.

#pragma warning ( push, 0 )
#   include "nlohmann/json_fwd.hpp"
#pragma warning ( pop )

namespace std::filesystem
{
    class path;
    class recursive_directory_iterator;
}

namespace drjuke
{
    using Json        = nlohmann::json;
    using Path        = std::filesystem::path;
    using DirIterator = std::filesystem::recursive_directory_iterator;
}

namespace fs = std::filesystem;