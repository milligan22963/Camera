/**
 * I2C.h
 * 
 * I2C implementation defining a port that can read/write
 * 
 * Copyright 2020 AFM Software
 */

#include <cstdio>
#include <cstdlib>

#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "I2C.h"

namespace afm
{
    namespace communication
    {
        const static std::string sc_i2c_dev = "i2c-%u";
        const int sc_max_device_name = 32;
        const int sc_i2c_success = 0;  // default success though might be positive depending on ioctl
        const int sc_max_buffer_size = 256; // handle a number of cases
        const int sc_tx_buffer_slot = 0;  // slot in an i2c buffer messages
        const int sc_rx_buffer_slot = 1;
        const int sc_i2c_max_messages = 2;

        I2C::I2C()
            : Port()
        {

        }

        bool I2C::read(uint8_t &value)
        {
            bool success = false;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                if (::read(m_device_handle, &value, sc_max_buffer_size) == 1)
                {
                    success = true;
                }
            }

            return success;
        }

        bool I2C::write(uint8_t value)
        {
            bool success = false;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                if (::write(m_device_handle, &value, sc_max_buffer_size) == 1)
                {
                    success = true;
                }
            }

            return success;
        }

        uint16_t I2C::read(data::Buffer &buffer)
        {
            uint16_t bytes_read = 0;

            if (select_address() == true)
            {
                data::BufferDataType data_buffer[sc_max_buffer_size] = {0};
                int data_read = 0;
                do
                {
                    data_read = ::read(m_device_handle, data_buffer, sc_max_buffer_size);
                    if (data_read > 0)
                    {
                        buffer.reserve(bytes_read + (uint16_t)data_read);
                        memcpy(&buffer.data()[bytes_read], data_buffer, data_read);
                        bytes_read += (uint16_t)data_read;
                    }
                } while (data_read == sc_max_buffer_size);
            }

            return bytes_read;
        }

        uint16_t I2C::write(const data::Buffer &buffer)
        {
            uint16_t bytes_written = 0;

            if (select_address() == true)
            {
                bytes_written = ::write(m_device_handle, buffer.data(), buffer.size());
            }

            return bytes_written;
        }

        uint16_t I2C::transfer(const data::Buffer &output_buffer, data::Buffer &input_buffer)
        {
            uint16_t bytes_transferred = 0;

            if (select_address() == true)
            {
                data::BufferDataType *tx_data = new data::BufferDataType[output_buffer.size()];

                struct i2c_rdwr_ioctl_data i2c_data;
                struct i2c_msg messages[sc_i2c_max_messages];

                memcpy(tx_data, output_buffer.data(), output_buffer.size());

                messages[sc_tx_buffer_slot].addr = get_device();
                messages[sc_tx_buffer_slot].flags = 0;
                messages[sc_tx_buffer_slot].len = output_buffer.size();
                messages[sc_tx_buffer_slot].buf = tx_data;

                messages[sc_rx_buffer_slot].addr = get_device();
                messages[sc_rx_buffer_slot].flags = I2C_M_RD;
                messages[sc_rx_buffer_slot].len = input_buffer.size();
                messages[sc_rx_buffer_slot].buf = input_buffer.data();

                i2c_data.nmsgs = sc_i2c_max_messages;
                i2c_data.msgs = messages;

                int ioctl_bytes = ioctl(m_device_handle, I2C_RDWR, &i2c_data);
                if (ioctl_bytes != -1)
                {
                    bytes_transferred = (uint16_t)ioctl_bytes;
                }

                delete [] tx_data;
            }

            return bytes_transferred;
        }

        // internal parts
        bool I2C::setup_device()
        {
            bool success = false;
            char device_name[sc_max_device_name] = {0};

            snprintf(device_name, sc_max_device_name - 1, sc_i2c_dev.c_str(), get_instance());

            std::string device_file_name = constants::sc_device_path + "/" + device_name;

            m_device_handle = ::open(device_file_name.c_str(), O_RDWR, O_RDWR);

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                success = true;
            }

            return success;
        }

        void I2C::shutdown_device()
        {
            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                ::close(m_device_handle);
                m_device_handle = constants::sc_invalid_file_handle;
            }
        }

        // private parts
        bool I2C::select_address()
        {
            bool success = false;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                if (ioctl(m_device_handle, I2C_SLAVE, get_device()) >= sc_i2c_success)
                {
                    success = true;
                }
            }
            return success;
        }

    }
}