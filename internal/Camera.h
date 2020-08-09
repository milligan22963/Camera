/**
 * Camera.h
 *
 * Note: Based off of https://github.com/raspberrypi/userland/blob/master/host_applications/linux/apps/raspicam/RaspiStill.c
 */

#ifndef _H_CAMERA
#define _H_CAMERA

#include <atomic>
#include <cstdint>
#include <fstream>
#include <memory>
#include <semaphore.h>

#include "mmal/mmal.h"
#include "mmal/mmal_parameters_camera.h"
#include "mmal/util/mmal_connection.h"

/// Frame advance method
enum FRAME_ADVANCE_METHOD
{
   FRAME_NEXT_SINGLE,
   FRAME_NEXT_TIMELAPSE,
   FRAME_NEXT_KEYPRESS,
   FRAME_NEXT_FOREVER,
   FRAME_NEXT_GPIO,
   FRAME_NEXT_SIGNAL,
   FRAME_NEXT_IMMEDIATELY,
   END_FRAME_ADVANCE_METHOD
};

class Camera
{
    public:
        static std::shared_ptr<Camera> GetInstance();

    public:
        Camera();
        virtual ~Camera();

        bool TakePicture(const std::string &file_name);
        bool SetStereoMode(uint8_t port, const MMAL_PARAMETER_STEREOSCOPIC_MODE_T &mode);

        // Internal callbacks
        void DefaultControlCallBack(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
        void DefaultEncoderBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

    private:
        bool Initialize();
        void Shutdown();
        bool ConfigureCamera();
        bool ConfigureEncoder();
        bool ConfigurePorts();
        bool ConfigurePreview(bool use_null_sink);
        bool ConnectPorts(MMAL_PORT_T *poutput_port, MMAL_PORT_T *pinput_port, MMAL_CONNECTION_T **ppconnection);
        void DisablePort(MMAL_PORT_T *pport);
        void OnError();

    private:
        std::atomic<bool> m_image_progress;                 // turns true if data is coming in
        bool    m_camera_ready = false;
        std::fstream m_capture_file;
        std::string m_camera_name;
//        VCOS_SEMAPHORE_T m_camera_semaphore;                // semaphore for camera callback interaction
        sem_t   m_camera_semaphore;
        int8_t  m_sharpness = 0;                            // -100 to 100
        int8_t  m_contrast = 0;                             // -100 to 100
        int8_t  m_brightness = 0;                           // 0 to 100
        int8_t  m_saturation = 0;                           // -100 to 100
        int8_t  m_exposure_compensation = 0;                // -10 to 10 can this be float?
        uint8_t m_num_cameras = 0;
        int16_t m_ISO = 100;                                // 100, 200, 400, 640, 800 ?
        uint16_t m_sensor_width = 0;
        uint16_t m_sensor_height = 0;
        uint32_t m_sensor_mode = 0;                         // 0 auto
        uint32_t m_jpg_quality = 85;                        // 0 - 100 quaility
        uint32_t m_jpg_restart_interval = 0;                // jpg restart interval
        uint32_t m_shutter_speed = 0;                       // Shutter speed 0 = auto
        MMAL_COMPONENT_T *m_pcamera = nullptr;              // the camera this represents
        MMAL_COMPONENT_T *m_pencoder = nullptr;             // the encoder in use
        MMAL_POOL_T *m_pencoder_pool = nullptr;             // the encoder pool
        MMAL_COMPONENT_T *m_ppreview = nullptr;             // The preview component
        MMAL_PORT_T *m_ppreview_port = nullptr;             // the image preview port
        MMAL_PORT_T *m_pvideo_port = nullptr;               // the video capture port
        MMAL_PORT_T *m_pstill_port = nullptr;               // the still image port
        MMAL_PARAMETER_STEREOSCOPIC_MODE_T m_stereo_mode;   // the stereo mode of the camera
        MMAL_PARAMETER_INT32_T m_camera_num;                // Number of cameras
        MMAL_PARAMETER_CAMERA_CONFIG_T m_camera_config;     // the camera config
        MMAL_PARAMETER_FPS_RANGE_T m_fps_range;             // fps range for camera
        MMAL_FOURCC_T m_encoding = MMAL_ENCODING_JPEG;      // encoding for decoders
        MMAL_CONNECTION_T *m_ppreview_connection = nullptr; // connection for preview flow
        MMAL_CONNECTION_T *m_ppcamera_connection = nullptr; // connection from camera to encoder
};

using CameraSPtr = std::shared_ptr<Camera>;

#endif