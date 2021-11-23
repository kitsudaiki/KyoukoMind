#include "test_list_blossom.h"

#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/logger.h>

using namespace Kitsunemimi::Sakura;

TestList_Blossom::TestList_Blossom()
    : Blossom()
{
    registerInputField("test_input", true);
    registerOutputField("test_output");
}

bool
TestList_Blossom::runTask(BlossomLeaf &blossomLeaf,
                          const Kitsunemimi::DataMap &,
                          BlossomStatus &status,
                          Kitsunemimi::ErrorContainer &error)
{
    const std::string input = blossomLeaf.input.getStringByKey("test_input");
    const std::string output = input + "_output";
    LOG_INFO("TestBlossom with intput: " + input);

    Kitsunemimi::DataArray* header = new Kitsunemimi::DataArray();
    header->append(new Kitsunemimi::DataValue("key"));
    header->append(new Kitsunemimi::DataValue("value"));

    Kitsunemimi::DataArray* body = new Kitsunemimi::DataArray();

    Kitsunemimi::DataArray* line1 = new Kitsunemimi::DataArray();
    line1->append(new Kitsunemimi::DataValue(input));
    line1->append(new Kitsunemimi::DataValue(output));
    body->append(line1);

    Kitsunemimi::DataArray* line2 = new Kitsunemimi::DataArray();
    line2->append(new Kitsunemimi::DataValue(input + "_line2"));
    line2->append(new Kitsunemimi::DataValue(output + "_line2"));
    body->append(line2);

    blossomLeaf.output.insert("header", header);
    blossomLeaf.output.insert("body", body);

    return true;
}
