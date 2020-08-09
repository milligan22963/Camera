/**
 * Persistence.cpp
 */

#include <string>
#include "Persistence.h"

const std::string sc_database_filename = "/home/pi/Documents/camera.db";

PersistenceSPtr Persistence::GetInstance()
{
    static PersistenceSPtr pPersistence = nullptr;

    if (pPersistence == nullptr)
    {
        pPersistence = std::make_shared<Persistence>();

        if (pPersistence->Initialize() == false)
        {
            pPersistence = nullptr;
        }
    }
    return pPersistence;
}

Persistence::Persistence()
{
}

Persistence::~Persistence()
{
    m_p_image_settings = nullptr;
    m_p_database = nullptr;
}

std::string Persistence::GetNextImageFileName()
{
    std::string nextImageName = m_p_image_settings->GetNextImageName();

    m_p_image_settings->Save();

    return nextImageName;
}

void Persistence::Shutdown()
{
    if (m_p_image_settings != nullptr)
    {
        m_p_image_settings->Save();
    }
}

bool Persistence::Initialize()
{
    bool success = false;

    m_p_database = std::make_shared<Database>();

    success = m_p_database->Initialize(sc_database_filename);

    if (success == true)
    {
        m_p_image_settings = std::make_shared<ImageSettings>();

        success = m_p_image_settings->Initialize(m_p_database);
    }

    return success;
}