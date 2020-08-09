/**
 * Persistence.h
 */

 #ifndef _H_PERSISTENCE
 #define _H_PERSISTENCE

#include <memory>

#include "Database.h"
#include "ImageSettings.h"

class Persistence
{
    public:
        static std::shared_ptr<Persistence> GetInstance();

        Persistence();
        virtual ~Persistence();

        std::string GetNextImageFileName();

        void Shutdown();

    private:
        bool Initialize();

    private:
        DatabaseSPtr m_p_database = nullptr;
        ImageSettingsSPtr m_p_image_settings = nullptr;
};

using PersistenceSPtr = std::shared_ptr<Persistence>;

 #endif