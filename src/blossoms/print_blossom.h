#ifndef PRINT_BLOSSOM_H
#define PRINT_BLOSSOM_H

#include <libKitsunemimiSakuraLang/blossom.h>

class PrintBlossom
        : public Kitsunemimi::Sakura::Blossom
{
public:
    PrintBlossom();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 std::string &);
};

#endif // PRINT_BLOSSOM_H
