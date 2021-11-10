#include "test_single_blossom.h"

#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/logger.h>

using namespace Kitsunemimi::Sakura;

TestSingle_Blossom::TestSingle_Blossom()
    : Blossom()
{
    registerInputField("test_input", true);
    registerOutputField("test_output", true);
}

bool
TestSingle_Blossom::runTask(BlossomLeaf &blossomLeaf,
                            uint64_t &status,
                            std::string &)
{
    const std::string input = blossomLeaf.input.getStringByKey("test_input");
    const std::string output = input + "_output";
    LOG_INFO("TestBlossom with intput: " + input);

    blossomLeaf.output.insert("test_output", new Kitsunemimi::DataValue(output));

    return true;
}
