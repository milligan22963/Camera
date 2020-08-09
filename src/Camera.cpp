/**
 * Camera.cpp
 *
 * Note: Based off of https://github.com/raspberrypi/userland/blob/master/host_applications/linux/apps/raspicam/RaspiStill.c
 */

#include <iostream>
#include "Camera.h"

#include "bcm_host.h"
#include "vcos/vcos.h"

#include "mmal/mmal.h"
#include "mmal/mmal_logging.h"
#include "mmal/mmal_buffer.h"
#include "mmal/util/mmal_util.h"
#include "mmal/util/mmal_util_params.h"
#include "mmal/util/mmal_default_components.h"
#include "mmal/util/mmal_connection.h"
#include "mmal/mmal_parameters_camera.h"

// Standard port setting for the camera component
const uint8_t MMAL_CAMERA_PREVIEW_PORT  = 0;
const uint8_t MMAL_CAMERA_VIDEO_PORT    = 1;
const uint8_t MMAL_CAMERA_CAPTURE_PORT  = 2;

// Stills format information
// 0 implies variable
const uint8_t STILLS_FRAME_RATE_NUM = 0;
const uint8_t STILLS_FRAME_RATE_DEN = 1;

/// Video render needs at least 2 buffers.
const uint8_t VIDEO_OUTPUT_BUFFERS_NUM = 3;

const uint8_t MAX_USER_EXIF_TAGS        = 32;
const uint8_t MAX_EXIF_PAYLOAD_LENGTH   = 128;

/// Amount of time before first image taken to allow settling of
/// exposure etc. in milliseconds.
const uint16_t CAMERA_SETTLE_TIME = 1000;

/// Layer that preview window should be displayed on
#define PREVIEW_LAYER      2

// Frames rates of 0 implies variable, but denominator needs to be 1 to prevent div by 0
const uint8_t PREVIEW_FRAME_RATE_NUM  = 0;
const uint8_t PREVIEW_FRAME_RATE_DEN  = 1;

const uint8_t FULL_RES_PREVIEW_FRAME_RATE_NUM  = 0;
const uint8_t FULL_RES_PREVIEW_FRAME_RATE_DEN  = 1;

const uint16_t FULL_FOV_PREVIEW_16x9_X = 1280;
const uint16_t FULL_FOV_PREVIEW_16x9_Y = 720;

const uint16_t FULL_FOV_PREVIEW_4x3_X  = 1296;
const uint16_t FULL_FOV_PREVIEW_4x3_Y = 972;

const uint8_t FULL_FOV_PREVIEW_FRAME_RATE_NUM  = 0;
const uint8_t FULL_FOV_PREVIEW_FRAME_RATE_DEN  = 1;

void CameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
void EncoderBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

std::shared_ptr<Camera> Camera::GetInstance()
{
    static CameraSPtr pCameraInstance = nullptr;

    if (pCameraInstance == nullptr)
    {
        vcos_log_info("Starting camera initializing...");
        pCameraInstance = std::make_shared<Camera>();
        if (pCameraInstance->Initialize() == false)
        {
            vcos_log_error("Failed to initialize camera");
            pCameraInstance->Shutdown(); // need to get rid of everything
            pCameraInstance = nullptr; // failed to init
        }
    }

    return pCameraInstance;
}

bool Camera::TakePicture(const std::string &file_name)
{
    bool success = false;

    m_capture_file.open(file_name, std::fstream::out | std::fstream::binary);

    if (m_capture_file.is_open() == true)
    {
        mmal_port_parameter_set_boolean(m_pencoder->output[0], MMAL_PARAMETER_EXIF_DISABLE, 1);

        // Set shutter speed
        if (mmal_port_parameter_set_uint32(m_pcamera->control, MMAL_PARAMETER_SHUTTER_SPEED, m_shutter_speed) == MMAL_SUCCESS)
        {
            uint32_t user_data[8];

            m_pencoder->output[0]->userdata = (struct MMAL_PORT_USERDATA_T *)&user_data;

            // Enable the encoder output port and tell it its callback function
            if (mmal_port_enable(m_pencoder->output[0], EncoderBufferCallback) == MMAL_SUCCESS)
            {
                // Send all the buffers to the encoder output port
                int num_queues = mmal_queue_length(m_pencoder_pool->queue);

                if (num_queues == 0)
                {
                    vcos_log_error("No queues in pool");
                }

                for (int queue = 0; queue < num_queues; queue++)
                {
                    MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(m_pencoder_pool->queue);

                    if (!buffer)
                    {
                        vcos_log_error("Unable to get a required buffer %d from pool queue", queue);
                    }

                    if (mmal_port_send_buffer(m_pencoder->output[0], buffer)!= MMAL_SUCCESS)
                    {
                        vcos_log_error("Unable to send a buffer to encoder output port (%d)", queue);
                    }
                }

                m_image_progress = false; // start no progress
                // setup capture
                if (mmal_port_parameter_set_boolean(m_pstill_port, MMAL_PARAMETER_CAPTURE, 1) == MMAL_SUCCESS)
                {
                    bool done = false;
                    struct timespec camera_timeout;

                    while (done == false)
                    {
                        if (clock_gettime(CLOCK_REALTIME, &camera_timeout) == -1)
                        {
                            vcos_log_error("Unable to get realtime clock");
                        }

                        camera_timeout.tv_sec += 5; // wait for 5 seconds or more...

                        //if (vcos_semaphore_wait_timeout(&m_camera_semaphore, 5000) == VCOS_SUCCESS)
                        if (sem_timedwait(&m_camera_semaphore, &camera_timeout) == 0)
                        {
                            success = true;
                            done = true;
                        }
                        else
                        {
                            if (m_image_progress == false)
                            {
                                vcos_log_error("Timed out capturing image.");
                                done = true;
                            }
                            else
                            {
                                m_image_progress = false;
                            }
                        }
                    }
                }
                mmal_port_disable(m_pencoder->output[0]);
            }
            else
            {
                vcos_log_error("Unable to enable encoder output port during image capture");
            }
        }
        else
        {
            vcos_log_error("Unable to set shutter speed");
        }
        m_capture_file.close();
    }

    return success;
}

bool Camera::SetStereoMode(uint8_t port, const MMAL_PARAMETER_STEREOSCOPIC_MODE_T &mode)
{
    bool success = false;

    success = mmal_port_parameter_set(m_pcamera->output[port], &mode.hdr) == MMAL_SUCCESS;

    return success;
}

Camera::Camera()
    : m_image_progress(false)
{

}

Camera::~Camera()
{
    Shutdown();
}

// private parts
bool Camera::Initialize()
{
    bool success = true;
    MMAL_COMPONENT_T *pcamera_info = nullptr;

    m_camera_ready = false;
    m_camera_name = "OV5647";
    m_sensor_width = 2592;
    m_sensor_height = 1944;

    if (mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA_INFO, &pcamera_info) == MMAL_SUCCESS)
    {
        MMAL_PARAMETER_CAMERA_INFO_T param;

        vcos_log_info("Create camera info component.");
        param.hdr.id = MMAL_PARAMETER_CAMERA_INFO;
        param.hdr.size = sizeof(param) - 4;  // Deliberately undersize to check firmware version
        if (mmal_port_parameter_get(pcamera_info->control, &param.hdr) != MMAL_SUCCESS)
        {
            // Try or new firmware
            param.hdr.size = sizeof(param);
            if (mmal_port_parameter_get(pcamera_info->control, &param.hdr) == MMAL_SUCCESS)
            {
                m_num_cameras = (uint8_t)param.num_cameras;

                // Take the parameters from the first camera listed.
                m_sensor_width = param.cameras[0].max_width;
                m_sensor_height = param.cameras[0].max_height;
                param.cameras[0].camera_name[MMAL_PARAMETER_CAMERA_INFO_MAX_STR_LEN - 1] = '\0';
                m_camera_name = param.cameras[0].camera_name;

                vcos_log_error("Camera name: %s", m_camera_name.c_str());
            }
            else
            {
                vcos_log_error("Unable to query camera.");
                success = false;
            }
        }
        mmal_component_destroy(pcamera_info);
    }
    else
    {
        success = false;
        vcos_log_error("Unable to create camera info component.");
    }
    if (success == true)
    {
        success = false;

        if (ConfigureCamera() == true)
        {
            if (ConfigurePreview(false) == true)
            {
                success = ConfigureEncoder();

                if (success == true)
                {
                    if (ConnectPorts(m_ppreview_port, m_ppreview->input[0], &m_ppreview_connection) != MMAL_SUCCESS)
                    {
                        vcos_log_error("Connecting prevew ports failed");
                        success = false;
                    }
                    else
                    {
                        // Now connect the camera to the encoder
                        if (ConnectPorts(m_pstill_port, m_pencoder->input[0], &m_ppcamera_connection) != MMAL_SUCCESS)
                        {
                            vcos_log_error("Connecting camera ports failed");
                            success = false;
                        }
                        else
                        {
//                            if (vcos_semaphore_create(&m_camera_semaphore, "Camera-CB-Sem", 0) == VCOS_SUCCESS)
                            if (sem_init(&m_camera_semaphore, 0, 0) == 0)
                            {
                                vcos_sleep(1500); // ok everything connected, lets let it roll
                                vcos_log_info("Created camera");
                                m_camera_ready = true;
                            }
                            else
                            {
                                vcos_log_error("Failed to create semaphore");
                                success = false;
                            }
                        }
                    }
                }
                else
                {
                    vcos_log_error("Unable to configure encoder");
                }
            }
            else
            {
                vcos_log_error("Unable to configure the preview");
            }
        }
        else
        {
            vcos_log_error("Unable to configure camera component");
        }
    }
    return success;
}

void Camera::Shutdown()
{
    if (m_camera_ready == true)
    {
        sem_destroy(&m_camera_semaphore);
//        vcos_semaphore_delete(&m_camera_semaphore);
    }

    m_camera_ready = false;

    // Disable un-connected ports
    DisablePort(m_pvideo_port);
    if (m_pencoder != nullptr)
    {
        DisablePort(m_pencoder->output[0]);
    }

    // Close down connections
    if (m_ppreview_connection != nullptr)
    {
        mmal_connection_destroy(m_ppreview_connection);
        m_ppreview_connection = nullptr;
    }

    if (m_ppcamera_connection != nullptr)
    {
        mmal_connection_destroy(m_ppcamera_connection);
        m_ppcamera_connection = nullptr;
    }

    // clean up encoder and related
    if (m_pencoder_pool != nullptr)
    {
        mmal_port_pool_destroy(m_pencoder->output[0], m_pencoder_pool);
        m_pencoder_pool = nullptr;
    }

    if (m_pencoder != nullptr)
    {
        mmal_component_disable(m_pencoder);
        mmal_component_destroy(m_pencoder);
        m_pencoder = nullptr;
    }

    // Clean up preview
    if (m_ppreview != nullptr)
    {
        mmal_component_disable(m_ppreview);
        mmal_component_destroy(m_ppreview);
        m_ppreview = nullptr;
    }

    // Clean up camera
    if (m_pcamera != nullptr)
    {
        mmal_component_disable(m_pcamera);
        mmal_component_destroy(m_pcamera);
        m_pcamera = nullptr;
    }
    m_ppreview_port = nullptr;
    m_pstill_port = nullptr;
    m_pvideo_port = nullptr;
}

bool Camera::ConfigureCamera()
{
    bool success = true;

    if (mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &m_pcamera) != MMAL_SUCCESS)
    {
        vcos_log_error("Failed to create camera component");
        success = false;
    }
    else
    {
        m_stereo_mode.hdr.id = MMAL_PARAMETER_STEREOSCOPIC_MODE;
        m_stereo_mode.hdr.size = sizeof(m_stereo_mode);
        m_stereo_mode.mode = MMAL_STEREOSCOPIC_MODE_NONE;
        m_stereo_mode.decimate = MMAL_FALSE;
        m_stereo_mode.swap_eyes = MMAL_FALSE;

        if (SetStereoMode(MMAL_CAMERA_PREVIEW_PORT, m_stereo_mode) == false)
        {
            success = false;
        }
        else if (SetStereoMode(MMAL_CAMERA_VIDEO_PORT, m_stereo_mode) == false)
        {
            success = false;
        }
        else if (SetStereoMode(MMAL_CAMERA_CAPTURE_PORT, m_stereo_mode) == false)
        {
            success = false;
        }

        m_camera_num.hdr.id = MMAL_PARAMETER_CAMERA_NUM;
        m_camera_num.hdr.size = sizeof(m_camera_num);
        m_camera_num.value = 0;

        success = mmal_port_parameter_set(m_pcamera->control, &m_camera_num.hdr) == MMAL_SUCCESS;

        if (success == true)
        {
            if (m_pcamera->output_num == 0)
            {
                vcos_log_error("Camera output number == 0");
                success = false;
            }
            else
            {
                success = mmal_port_parameter_set_uint32(m_pcamera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, m_sensor_mode) == MMAL_SUCCESS;
                if (success == false)
                {
                    vcos_log_error("Failed to set sensor mode");
                }
            }

            if (success == true)
            {
                m_ppreview_port = m_pcamera->output[MMAL_CAMERA_PREVIEW_PORT];
                m_pvideo_port = m_pcamera->output[MMAL_CAMERA_VIDEO_PORT];
                m_pstill_port = m_pcamera->output[MMAL_CAMERA_CAPTURE_PORT];

                // Enable the camera, and tell it its control callback function
                success = mmal_port_enable(m_pcamera->control, CameraControlCallback) == MMAL_SUCCESS;
            }

            if (success == true)
            {
                m_camera_config =
                {
                    {
                        MMAL_PARAMETER_CAMERA_CONFIG,
                        sizeof(m_camera_config)
                    },
                    .max_stills_w = m_sensor_width,
                    .max_stills_h = m_sensor_height,
                    .stills_yuv422 = 0,
                    .one_shot_stills = 1,
                    .max_preview_video_w = 1024,
                    .max_preview_video_h = 768,
                    .num_preview_video_frames = 3,
                    .stills_capture_circular_buffer_height = 0,
                    .fast_preview_resume = 0,
                    .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
                };

                success = mmal_port_parameter_set(m_pcamera->control, &m_camera_config.hdr) == MMAL_SUCCESS;

                if (success == true)
                {
                    success = ConfigurePorts();
                }
            }
        }
        else
        {
            vcos_log_error("Unable to set port parementers for camera");
        }

        if (success == true)
        {
            if (mmal_component_enable(m_pcamera) != MMAL_SUCCESS)
            {
                success = false;
                Shutdown();
            }
        }
    }
    return success;
}

bool Camera::ConfigurePorts()
{
    bool success = true;

    m_ppreview_port->format->encoding = MMAL_ENCODING_OPAQUE;
    m_ppreview_port->format->encoding_variant = MMAL_ENCODING_I420;

    m_fps_range =
    {
        {
            MMAL_PARAMETER_FPS_RANGE,
            sizeof(m_fps_range)
        },
        {
            167,
            1000
        },
        {
            999,
            1000
        }
    };
    
    success = mmal_port_parameter_set(m_ppreview_port, &m_fps_range.hdr) == MMAL_SUCCESS;

    if (success == true)
    {
        // Use a full FOV 4:3 mode
        m_ppreview_port->format->es->video.width = VCOS_ALIGN_UP(800, 32);
        m_ppreview_port->format->es->video.height = VCOS_ALIGN_UP(480, 16);
        m_ppreview_port->format->es->video.crop.x = 0;
        m_ppreview_port->format->es->video.crop.y = 0;
        m_ppreview_port->format->es->video.crop.width = 800;
        m_ppreview_port->format->es->video.crop.height = 480;
        m_ppreview_port->format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
        m_ppreview_port->format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;

        if (mmal_port_format_commit(m_ppreview_port) == MMAL_SUCCESS)
        {
            // configure video the same for now
            mmal_format_full_copy(m_pvideo_port->format, m_ppreview_port->format);
            if (mmal_port_format_commit(m_pvideo_port) != MMAL_SUCCESS)
            {
                success = false;
            }
            else
            {
                if (m_pvideo_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
                {
                    m_pvideo_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
                }
            }

            if (success == true)
            {
                success = mmal_port_parameter_set(m_pstill_port, &m_fps_range.hdr) == MMAL_SUCCESS;
                if (success == true)
                {
                    m_pstill_port->format->encoding = MMAL_ENCODING_OPAQUE;
                    m_pstill_port->format->es->video.width = VCOS_ALIGN_UP(m_sensor_width, 32);
                    m_pstill_port->format->es->video.height = VCOS_ALIGN_UP(m_sensor_height, 16);
                    m_pstill_port->format->es->video.crop.x = 0;
                    m_pstill_port->format->es->video.crop.y = 0;
                    m_pstill_port->format->es->video.crop.width = m_sensor_width;
                    m_pstill_port->format->es->video.crop.height = m_sensor_height;
                    m_pstill_port->format->es->video.frame_rate.num = STILLS_FRAME_RATE_NUM;
                    m_pstill_port->format->es->video.frame_rate.den = STILLS_FRAME_RATE_DEN;

                    if (mmal_port_format_commit(m_pstill_port) != MMAL_SUCCESS)
                    {
                        success = false;
                    }
                    else
                    {
                        if (m_pstill_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
                        {
                            m_pstill_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
                        }
                    }
                }
            }
        }
        else
        {
            success = false;
        }
    }

    return success;
}

bool Camera::ConfigurePreview(bool use_null_sink)
{
    bool success = true;

    if (use_null_sink == true)
    {
        if (mmal_component_create("vc.null_sink", &m_ppreview) != MMAL_SUCCESS)
        {
            success = false;
        }
    }
    else
    {
        if (mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &m_ppreview) != MMAL_SUCCESS)
        {
            success = false;
        }
        else if (m_ppreview->input_num == 0)
        {
            success = false;
        }
        else
        {
            if (m_ppreview->input != nullptr)
            {
                MMAL_DISPLAYREGION_T param =
                {
                    {
                        MMAL_PARAMETER_DISPLAYREGION,
                        sizeof(MMAL_DISPLAYREGION_T)
                    },
                    MMAL_DISPLAY_SET_LAYER | MMAL_DISPLAY_SET_ALPHA | MMAL_DISPLAY_SET_DEST_RECT | MMAL_DISPLAY_SET_FULLSCREEN | MMAL_DISPLAY_SET_NUM,
                    0, // display num
                    MMAL_FALSE, // full screen
                    MMAL_DISPLAYTRANSFORM_T {},
                    {0, 0, 0, 0},
                    {0, 0, 0, 0},
                    MMAL_FALSE, // noaspect
                    MMAL_DISPLAY_MODE_FILL, // fill
                    0,
                    0,
                    PREVIEW_LAYER,
                    MMAL_FALSE, // no copyright
                    0 // opacity
                };

                if (mmal_port_parameter_set(m_ppreview->input[MMAL_CAMERA_PREVIEW_PORT], &param.hdr) != MMAL_SUCCESS)
                {
                    vcos_log_error("Unable to set preview display region.");
                    success = false;
                }
            }
            else
            {
                success = false;
                vcos_log_error("No input preview ports");
            }
        }
    }

    if (success == true)
    {
        /* Enable component */
        success = mmal_component_enable(m_ppreview) == MMAL_SUCCESS;
        if (success == false)
        {
            vcos_log_error("Failed to enable preview component");
        }
    }
    else if (m_ppreview != nullptr)
    {
        mmal_component_destroy(m_ppreview);
        m_ppreview = nullptr;
    }

    return success;
}

bool Camera::ConfigureEncoder()
{
    bool success = false;

    if (mmal_component_create(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER, &m_pencoder) == MMAL_SUCCESS)
    {
        if ((m_pencoder->input_num != 0) && (m_pencoder->output_num != 0))
        {
            MMAL_PORT_T *pencoder_input = m_pencoder->input[0];
            MMAL_PORT_T *pencoder_output = m_pencoder->output[0];

            // We want same format on input and output
            mmal_format_copy(pencoder_output->format, pencoder_input->format);

            // Specify out output format
            pencoder_output->format->encoding = m_encoding;

            pencoder_output->buffer_size = pencoder_output->buffer_size_recommended;

            if (pencoder_output->buffer_size < pencoder_output->buffer_size_min)
            {
                pencoder_output->buffer_size = pencoder_output->buffer_size_min;
            }

            pencoder_output->buffer_num = pencoder_output->buffer_num_recommended;

            if (pencoder_output->buffer_num < pencoder_output->buffer_num_min)
            {
                pencoder_output->buffer_num = pencoder_output->buffer_num_min;
            }

            // Commit the port changes to the output port
            if (mmal_port_format_commit(pencoder_output) == MMAL_SUCCESS)
            {
                // Set the JPEG quality level
                if (mmal_port_parameter_set_uint32(pencoder_output, MMAL_PARAMETER_JPEG_Q_FACTOR, m_jpg_quality) != MMAL_SUCCESS)
                {
                    success = false;
                }
                else
                {
                    // Set the JPEG restart interval
                    if (mmal_port_parameter_set_uint32(pencoder_output, MMAL_PARAMETER_JPEG_RESTART_INTERVAL, m_jpg_restart_interval) != MMAL_SUCCESS)
                    {
                        success = false;
                    }
                    else
                    {
                        // Set up any required thumbnail
                        MMAL_PARAMETER_THUMBNAIL_CONFIG_T param_thumb =
                        {
                            {
                                MMAL_PARAMETER_THUMBNAIL_CONFIGURATION,
                                sizeof(MMAL_PARAMETER_THUMBNAIL_CONFIG_T)
                            },
                            1,
                            64,
                            48,
                            85
                        };
                        if (mmal_port_parameter_set(m_pencoder->control, &param_thumb.hdr) != MMAL_SUCCESS)
                        {
                            success = false;
                        }
                        else
                        {
                            //  Enable component
                            if (mmal_component_enable(m_pencoder) == MMAL_SUCCESS)
                            {
                                /* Create pool of buffer headers for the output port to consume */
                                m_pencoder_pool = mmal_port_pool_create(pencoder_output, pencoder_output->buffer_num, pencoder_output->buffer_size);

                                if (m_pencoder_pool == nullptr)
                                {
                                    vcos_log_error("Unable to create encoder pool");
                                    success = false;
                                }
                                else
                                {
                                    success = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (success == false)
        {
            if (m_pencoder_pool != nullptr)
            {
                mmal_port_pool_destroy(m_pencoder->output[0], m_pencoder_pool);
                m_pencoder_pool = nullptr;
            }
            mmal_component_destroy(m_pencoder);
            m_pencoder = nullptr;
        }
    }
    return success;
}

bool Camera::ConnectPorts(MMAL_PORT_T *poutput_port, MMAL_PORT_T *pinput_port, MMAL_CONNECTION_T **ppconnection)
{
    bool success = false;

    if (mmal_connection_create(ppconnection, poutput_port, pinput_port,
        MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT) == MMAL_SUCCESS)
    {
        if (mmal_connection_enable(*ppconnection) != MMAL_SUCCESS)
        {
            mmal_connection_destroy(*ppconnection);
            *ppconnection = nullptr;
        }
    }
    return success;
}

void Camera::DisablePort(MMAL_PORT_T *pport)
{
    if (pport != nullptr)
    {
        if (pport->is_enabled)
        {
            mmal_port_disable(pport);
        }
    }
}

void Camera::OnError()
{

}

void Camera::DefaultControlCallBack(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
   if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
   {
      MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
      switch (param->hdr.id)
      {
      case MMAL_PARAMETER_CAMERA_SETTINGS:
      {
         MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
         vcos_log_error("Exposure now %u, analog gain %u/%u, digital gain %u/%u",
                        settings->exposure,
                        settings->analog_gain.num, settings->analog_gain.den,
                        settings->digital_gain.num, settings->digital_gain.den);
         vcos_log_error("AWB R=%u/%u, B=%u/%u",
                        settings->awb_red_gain.num, settings->awb_red_gain.den,
                        settings->awb_blue_gain.num, settings->awb_blue_gain.den);
      }
      break;
      }
   }
   else if (buffer->cmd == MMAL_EVENT_ERROR)
   {
       OnError();
   }
   else
   {
       OnError();
   }

   mmal_buffer_header_release(buffer);
}

void Camera::DefaultEncoderBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    bool done = false;

    if (m_capture_file.is_open() == true)
    {
        if (buffer->length > 0)
        {
            long current_size = m_capture_file.tellp();

            mmal_buffer_header_mem_lock(buffer);

            m_capture_file.write((char *)buffer->data, buffer->length);

            mmal_buffer_header_mem_unlock(buffer);

            if (m_capture_file.tellp() == current_size + buffer->length)
            {
                // good read
                if (buffer->flags & (MMAL_BUFFER_HEADER_FLAG_FRAME_END | MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED))
                {
                    done = true;
                }
            }
            else
            {
                done = true;
            }
        }
    }

    // release buffer back to the pool
    mmal_buffer_header_release(buffer);

    // and send one back to the port (if still open)
    if (port->is_enabled)
    {
        MMAL_STATUS_T status = MMAL_SUCCESS;
        MMAL_BUFFER_HEADER_T *new_buffer;

        new_buffer = mmal_queue_get(m_pencoder_pool->queue);

        if (new_buffer)
        {
            status = mmal_port_send_buffer(port, new_buffer);
        }
        if (!new_buffer || status != MMAL_SUCCESS)
        {
            vcos_log_error("Unable to return a buffer to the encoder port");
        }
    }

    m_image_progress = true;

    if (done == true)
    {
        sem_post(&m_camera_semaphore);
//        vcos_semaphore_post(&m_camera_semaphore);
    }
}

void CameraControlCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    CameraSPtr pCamera = Camera::GetInstance();

    if (pCamera != nullptr)
    {
        pCamera->DefaultControlCallBack(port, buffer);
    }
    else
    {
        vcos_log_error("Camera on control callback is nullptr");
    }
}

void EncoderBufferCallback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    CameraSPtr pCamera = Camera::GetInstance();

    if (pCamera != nullptr)
    {
        pCamera->DefaultEncoderBufferCallback(port, buffer);
    }
    else
    {
        vcos_log_error("Camera on encoder buffer callback is nullptr");
    }
}
