/**
 * Table.cpp
 */

#include <sstream>
#include "Table.h"

Table::Table()
{

}

Table::~Table()
{
    m_p_database = nullptr;
}

bool Table::Initialize(DatabaseSPtr pDatabase)
{
    bool success = true;

    m_p_database = pDatabase;

    // make sure table exists
    success = m_p_database->IssueCommand(GetCreateString());

    return success;
}

void Table::Save()
{
    OnSaveData();
}

void Table::SetTableName(const std::string &table_name)
{
    m_table_name = table_name;
}

std::string Table::GetCreateString()
{
    std::stringstream create_string;

    create_string << "CREATE TABLE IF NOT EXISTS ";
    create_string << m_table_name << " (";

    create_string << OnGetCreateString();

    create_string << ");";

    return create_string.str();
}

std::string Table::OnGetCreateString()
{
    return "";
}

void Table::OnSaveData()
{
    // do nothing
}
