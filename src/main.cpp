/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <common.h>

#include <kyouko_root.h>
#include <args.h>
#include <config.h>

#include <libKitsunemimiArgs/arg_parser.h>
#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiConfig/config_handler.h>

int
main(int argc, char *argv[])
{
    Kitsunemimi::Persistence::initConsoleLogger(true);

    // create and init argument-parser
    Kitsunemimi::Args::ArgParser argParser;
    registerArguments(argParser);

    // parse cli-input
    if(argParser.parse(argc, argv) == false) {
        return 1;
    }

    // init logging
    const bool enableDebug = argParser.wasSet("debug");
    Kitsunemimi::Persistence::initConsoleLogger(enableDebug);
    Kitsunemimi::Persistence::initFileLogger("/var/log/",
                                             "KyoukoMind",
                                             enableDebug);

    // create server
    KyoukoRoot* rootObject = new KyoukoRoot();
    rootObject->start();

    int a = 0;
    std::cin >> a;
    return 0;
}
