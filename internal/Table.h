/**
 * Table.h
 */

#ifndef _H_TABLE
#define _H_TABLE

#include <memory>
#include <string>
#include <sqlite3.h>
#include "Database.h"

class Table
{
    public:
        Table();
        virtual ~Table();

        virtual bool Initialize(DatabaseSPtr pDatabase);

        void Save();

    protected:
        void SetTableName(const std::string &table_name);

        DatabaseSPtr GetDatabase() { return m_p_database; }

        std::string GetCreateString();
        virtual std::string OnGetCreateString();

        virtual void OnSaveData();
        
    private:
        std::string m_table_name;
        DatabaseSPtr m_p_database = nullptr;
};

using TableSPtr = std::shared_ptr<Table>;

#endif