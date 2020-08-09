/**
 * Camera
 *
 * Raspberry PI camera polaroid
 */

#include <atomic>
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

    // Register our application with the logging system
    vcos_log_register("AfmCamera", VCOS_LOG_CATEGORY);

    gtk_init(&argc, &argv);

    if (sem_init(&g_picture_semaphore, 0, 0) != 0)
    {
        vcos_log_error("Unable to initialize our semaphore for pictures");
        return -1;
    }

    afm::graphic::IAppSPtr pApp = afm::graphic::App::GetApplication();

    if (pApp != nullptr) {
        if (pApp->Initialize(glade_file) == true) {
            afm::graphic::IAfmWindowSPtr p_main_window = std::make_shared<afm::graphic::MainWindow>();

            pApp->Run(p_main_window);

            p_main_window = nullptr;
        }

        pApp->Shutdown();
        pApp = nullptr;
    }

    vcos_log_error("Shutting down");

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
    vcos_log_info("Take a picture");
}

void CameraThread()
{
    CameraSPtr pCamera = Camera::GetInstance();
    PersistenceSPtr persistence = Persistence::GetInstance();

    if (pCamera != nullptr) {
        while (g_camera_processing == true) {
            struct timespec camera_timeout;

            if (clock_gettime(CLOCK_REALTIME, &camera_timeout) == -1)
            {
                vcos_log_error("Unable to get realtime clock");
            }
            
            vcos_log_info("Waiting....");

            camera_timeout.tv_sec += 1; // wait for 1 second so if picture isn't fired we can come back and check off
            if (sem_timedwait(&g_picture_semaphore, &camera_timeout) == 0)
            {
                std::string nextFileName = persistence->GetNextImageFileName();
                vcos_log_error("Taking picture now - %s", nextFileName.c_str());
                //pCamera->TakePicture(persistence->GetNextImageFileName());
                pCamera->TakePicture(nextFileName);
            }
            else
            {
                vcos_log_info("Timedout");
            }
        }
    }
    else
    {
        vcos_log_error("Unable to create camera.");
    }
}
