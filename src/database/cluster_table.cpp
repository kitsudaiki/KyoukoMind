#include "cluster_table.h"

#include <libKitsunemimiCommon/common_items/table_item.h>
#include <libKitsunemimiCommon/common_methods/string_methods.h>
#include <libKitsunemimiJson/json_item.h>

#include <libKitsunemimiSakuraDatabase/sql_database.h>

ClusterTable::ClusterTable(Kitsunemimi::Sakura::SqlDatabase* db)
    : HanamiSqlTable(db)
{
    m_tableName = "clusters";

    DbHeaderEntry clusterName;
    clusterName.name = "cluster_name";
    clusterName.maxLength = 256;
    m_tableHeader.push_back(clusterName);

}

ClusterTable::~ClusterTable()
{

}
