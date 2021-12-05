/**
 * Camera
 *
 * Raspberry PI camera polaroid
 */

#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <gtk/gtk.h>
#include <X11/Xlib.h>
#include <semaphore.h>
#include <sqlite3.h>

#include "bcm_host.h"
#include "vcos/vcos.h"
#include "vcos/vcos_logging.h"

#include "App.h"
#include "MainWindow.h"
#include "Camera.h"
#include "util.h"
#include "handlers.h"
#include "DataTypes.h"
#include "GPIO.h"
#include "PortFactory.h"
#include "Persistence.h"

VCOS_LOG_CAT_T afm_camera_log_category;

#define VCOS_LOG_CATEGORY (&afm_camera_log_category)

// GPIO pin definitions
const uint8_t sc_camera_on = 16;
const uint8_t sc_take_picture = 26;

sem_t       g_picture_semaphore;
afm::communication::IPortSPtr g_p_camera_on = nullptr;

void Handle_Camera_On(uint8_t interrupt_triggered);
void Handle_Interrupt(uint8_t interrupt_triggered);
void CameraThread();

int main(int argc, char *argv[])
{
    int         return_code = 0;
    afm::communication::IPortSPtr p_take_picture;

    PersistenceSPtr persistence = Persistence::GetInstance();

    LogSPtr pLog = Logger::GetInstance();

    if (!pLog->InitializeLog(Logger::LOG_TYPE::LOG_FILE, "/var/log/camera.log", Logger::LOG_LEVEL::LOG_LEVEL_INFO)) {
        std::cout << "Failed to setup log.\n";
    }

    bcm_host_init();

    g_p_camera_on = afm::communication::PortFactory::getInstance()->createPort(afm::data::PortType::PORT_GPIO, sc_camera_on, 0);
    p_take_picture = afm::communication::PortFactory::getInstance()->createPort(afm::data::PortType::PORT_GPIO, sc_take_picture, 0);

    // Configure the take picture button as an interrupt
    afm::communication::GPIOSPtr pGPIO = std::dynamic_pointer_cast<afm::communication::GPIO>(p_take_picture);
    if (pGPIO != nullptr) {
        pGPIO->enable_interrupt(afm::data::GPIOInterruptEdge::RISING, Handle_Interrupt);
    }

    pGPIO = std::dynamic_pointer_cast<afm::communication::GPIO>(g_p_camera_on);
    if (pGPIO != nullptr) {
        pGPIO->enable_interrupt(afm::data::GPIOInterruptEdge::BOTH, Handle_Camera_On);
    }

    pGPIO = nullptr;

    gtk_init(&argc, &argv);

    if (sem_init(&g_picture_semaphore, 0, 0) != 0)
    {
        pLog->Log("Unable to initialize our semaphore for pictures\n", Logger::LOG_LEVEL::LOG_LEVEL_ERROR);
        return -1;
    }

    afm::graphic::IAppSPtr pApp = afm::graphic::App::GetApplication();

    if (pApp != nullptr) {
        if (pApp->Initialize(glade_file) == true) {
            afm::graphic::IAfmWindowSPtr p_main_window = std::make_shared<afm::graphic::MainWindow>();

            pApp->ShowWindow(p_main_window);

            pApp->Run();

            p_main_window = nullptr;
        }

        pApp->Shutdown();
        pApp = nullptr;
    }

    pLog->Log("Shutting down\n", Logger::LOG_LEVEL::LOG_LEVEL_INFO);

    persistence->Shutdown();

    p_take_picture = nullptr;
    g_p_camera_on = nullptr;

    sem_destroy(&g_picture_semaphore);

    return return_code;
}

std::atomic<bool> g_camera_processing(false);

void Handle_Camera_On(uint8_t interrupt_triggered)
{
    static bool camera_on = false;
    static std::thread camera_thread;

    uint8_t camera_state = 0;

    if (g_p_camera_on->read(camera_state) == true) {
        camera_on = camera_state == 1 ? true : false;

        if (camera_on == true) {
            g_camera_processing = true;
            camera_thread = std::thread(CameraThread);
            //
        } else {
            g_camera_processing = false;
            camera_thread.join();
        }
    }
}

void Handle_Interrupt(uint8_t interrupt_triggered)
{
    // interrupt fired
    sem_post(&g_picture_semaphore);
    Logger::GetInstance()->Log("Taking a picture\n", Logger::LOG_LEVEL::LOG_LEVEL_INFO);
}

void CameraThread()
{
    CameraSPtr pCamera = Camera::GetInstance();
    PersistenceSPtr persistence = Persistence::GetInstance();
    LogSPtr pLog = Logger::GetInstance();

    if (pCamera != nullptr) {
        while (g_camera_processing == true) {
            struct timespec camera_timeout;

            if (clock_gettime(CLOCK_REALTIME, &camera_timeout) == -1) {
                pLog->Log("Unable to get realtime clock\n", Logger::LOG_LEVEL::LOG_LEVEL_ERROR);
            }

            pLog->Log("Waiting\n", Logger::LOG_LEVEL::LOG_LEVEL_INFO);

            camera_timeout.tv_sec += 1; // wait for 1 second so if picture isn't fired we can come back and check off
            if (sem_timedwait(&g_picture_semaphore, &camera_timeout) == 0) {
                std::string nextFileName = persistence->GetNextImageFileName();

                pLog->Log("Taking picture now - ", Logger::LOG_LEVEL::LOG_LEVEL_INFO);
                pLog->Log(nextFileName, Logger::LOG_LEVEL::LOG_LEVEL_INFO);
                pLog->Log("\n", Logger::LOG_LEVEL::LOG_LEVEL_INFO);

                //pCamera->TakePicture(persistence->GetNextImageFileName());
                pCamera->TakePicture(nextFileName);
            } else {
                pLog->Log("Timedout\n", Logger::LOG_LEVEL::LOG_LEVEL_INFO);
            }
        }
    } else {
        pLog->Log("Unable to create camera.\n", Logger::LOG_LEVEL::LOG_LEVEL_ERROR);
    }
}
