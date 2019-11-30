#include "vector.h"

int Kek(const stl::Vector<int>& lol)
{
    return lol;
}

int main(int argc, const char *argv[])
{
    stl::Vector<int> karvalol;

    for (auto it : karvalol)
    {
        it += 1;
    }

    return 0;
}