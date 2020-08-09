/**
 * Database.h
 */

#ifndef _H_DATABASE
#define _H_DATABASE

#include <memory>
#include <sqlite3.h>

using SQLCallbackFunction = int (*)(void*,int,char**,char**);

class Database
{
    public:
        Database();
        virtual ~Database();

        bool Initialize(const std::string &database_name);

        bool IssueCommand(const std::string &command);
        bool IssueCommand(const std::string &command, SQLCallbackFunction callback, void *p_data);

    private:
        sqlite3     *m_p_camera_db = nullptr;
};

using DatabaseSPtr = std::shared_ptr<Database>;

#endif