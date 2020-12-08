/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <common.h>

#include <kyouko_root.h>
#include <args.h>
#include <config.h>
#include <core/callbacks.h>

#include <libKitsunemimiArgs/arg_parser.h>
#include <libKitsunemimiPersistence/logger/logger.h>

#include <libKitsunemimiSakuraMessaging/messaging_controller.h>

using Kitsunemimi::Sakura::MessagingController;

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
    Kitsunemimi::Persistence::initFileLogger("/var/log/", "KyoukoMind", enableDebug);

    // init config
    const std::string configFile = argParser.getStringValue("config");
    if(Kitsunemimi::Config::initConfig(configFile) == false) {
        return 1;
    }
    registerConfigs();

    // create server
    KyoukoRoot* rootObject = new KyoukoRoot();
    rootObject->initBlossoms();
    if(rootObject->initSakuraFiles() == false) {
        return 1;
    }

    std::vector<std::string> groupNames = {};
    if(MessagingController::initializeMessagingController("KyoukoMind",
                                                          groupNames,
                                                          &sessionCreateCallback,
                                                          &sessionCloseCallback) == false)
    {
        return 1;
    }

    rootObject->start();

    int a = 0;
    std::cin >> a;
    return 0;
}
