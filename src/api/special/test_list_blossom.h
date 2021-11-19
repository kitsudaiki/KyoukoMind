#ifndef TEST_LIST_BLOSSOM_H
#define TEST_LIST_BLOSSOM_H

#include <libKitsunemimiSakuraLang/blossom.h>

class TestList_Blossom
        : public Kitsunemimi::Sakura::Blossom
{
public:
    TestList_Blossom();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 Kitsunemimi::Sakura::BlossomStatus &status,
                 Kitsunemimi::ErrorContainer &error);
};

#endif // TEST_LIST_BLOSSOM_H
