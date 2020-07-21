/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
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
#include <math.h>
#include <cmath>
#include <utility>
#include <atomic>

#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;

#include <libKitsunemimiCommon/buffer/data_buffer.h>
using Kitsunemimi::DataBuffer;

#include <libKitsunemimiCommon/buffer/stack_buffer.h>
using Kitsunemimi::StackBuffer;

#include <libKitsunemimiCommon/common_items/data_items.h>
using Kitsunemimi::DataItem;
using Kitsunemimi::DataArray;
using Kitsunemimi::DataValue;
using Kitsunemimi::DataMap;

#include <libKitsunemimiJson/json_item.h>
using Kitsunemimi::Json::JsonItem;

#endif // INCLUDES_H
