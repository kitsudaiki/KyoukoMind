/**
 *  @file    includes.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef INCLUDES_H
#define INCLUDES_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctime>

#include <vector>
#include <string.h>
#include <iostream>
#include <string>
#include <dirent.h>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <ctype.h>
#include <algorithm>
#include <iterator>
#include <map>
#include <assert.h>
#include <thread>
#include <mutex>
#include <istream>
#include <time.h>
#include <queue>
#include <condition_variable>
#include <unistd.h>
#include <cmath>
#include <utility>
#include <atomic>

#include <libKitsunemimiCommon/common_items/data_items.h>
#include <libKitsunemimiCommon/data_buffer.h>

using Kitsunemimi::Common::DataBuffer;
using Kitsunemimi::Common::DataItem;
using Kitsunemimi::Common::DataArray;
using Kitsunemimi::Common::DataValue;
using Kitsunemimi::Common::DataMap;

#include <libKitsunemimiJson/json_item.h>

using Kitsunemimi::Json::JsonItem;

#endif // INCLUDES_H
