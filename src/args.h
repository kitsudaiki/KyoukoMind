/**
 * @file        args.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef KYOUKOMIND_ARGS_H
#define KYOUKOMIND_ARGS_H

#include <libKitsunemimiArgs/arg_parser.h>

/**
 * @brief register all available arguments for the CLI input
 *
 * @param argparser reference to predefined argument-parser
 *
 * @return false, if registering argument failed, else true
 */
bool
registerArguments(Kitsunemimi::Args::ArgParser &argparser)
{
    std::string helpText = "";

    helpText = "absolute path to config-file";
    if(argparser.registerString("config,c", helpText) == false) {
        return false;
    }

    helpText = "enable debug-mode";
    if(argparser.registerPlain("debug,d", helpText) == false) {
        return false;
    }

    return true;
}

#endif // KYOUKOMIND_ARGS_H
