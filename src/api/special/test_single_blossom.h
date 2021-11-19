#ifndef TEST_SINGLE_BLOSSOM_H
#define TEST_SINGLE_BLOSSOM_H

#include <libKitsunemimiSakuraLang/blossom.h>

class TestSingle_Blossom
        : public Kitsunemimi::Sakura::Blossom
{
public:
    TestSingle_Blossom();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 Kitsunemimi::Sakura::BlossomStatus &status,
                 Kitsunemimi::ErrorContainer &error);
};

#endif // TEST_LIST_BLOSSOM_H
