#ifndef ARGS_H
#define ARGS_H

#include <libKitsunemimiArgs/arg_parser.h>

namespace ToriiGateway
{

bool
registerArguments(Kitsunemimi::Args::ArgParser &argparser)
{
    argparser.registerString("config,c",
                             "absolute path to config-file",
                             true);

    argparser.registerInteger("log-dir",
                              "absolute path to the directory, "
                              "where the log-file should be written \n"
                              "(Default: /var/log/)",
                              false);

    argparser.registerInteger("log-base",
                              "base name for the log-fil \n"
                              "(Default: ToriiGateway)",
                              false);

    argparser.registerPlain("debug,d",
                            "enable debug-mode");
    return true;
}

}

#endif // ARGS_H
