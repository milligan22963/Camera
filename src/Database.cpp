/**
 * Database.cpp
 */

#include "Database.h"

Database::Database()
{
    sqlite3_initialize();
}

Database::~Database()
{
    sqlite3_close(m_p_camera_db);
    sqlite3_shutdown();
}

bool Database::Initialize(const std::string &database_name)
{
    bool success = false;

    if (sqlite3_open(database_name.c_str(), &m_p_camera_db) == 0)
    {
        success = true;
    }

    return success;
}

bool Database::IssueCommand(const std::string &command)
{
    return IssueCommand(command, nullptr, nullptr);
}

bool Database::IssueCommand(const std::string &command, SQLCallbackFunction callback, void *p_data)
{
    bool success = true;
    char *p_err_message = nullptr;

    if (sqlite3_exec(m_p_camera_db, command.c_str(), callback, p_data, &p_err_message) != SQLITE_OK)
    {
        if (p_err_message != nullptr)
        {
            sqlite3_free(p_err_message);
        }
    }
    return success;
}
