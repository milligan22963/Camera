/**
 * SPI.cpp
 * 
 * SPI implementation defining a port that can read/write
 * 
 * Copyright 2020 AFM Software
 */

#include <fcntl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <memory.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "SPI.h"

namespace afm
{
    namespace communication
    {
        const static std::string sc_spi_dev = "spidev%u.%u";
        const int sc_max_device_name = 32;
        const int sc_max_buffer_size = 256; // handle a number of cases
        const uint8_t sc_default_bits_per_word = 8;
        const uint32_t sc_default_frequency = 1000000;
        const int sc_tx_buffer_slot = 0;  // slot in an i2c buffer messages
        const int sc_rx_buffer_slot = 1;
        const int sc_spi_max_messages = 2;

        SPI::SPI()
            : Port()
            , m_bits_per_word(sc_default_bits_per_word)
            , m_frequency(sc_default_frequency)
        {

        }

        bool SPI::read(uint8_t &value)
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

        bool SPI::write(uint8_t value)
        {
            bool success = false;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                if (::write(m_device_handle, &value, 1) == 1)
                {
                    success = true;
                }
            }

            return success;
        }

        uint16_t SPI::read(data::Buffer &buffer)
        {
            uint16_t bytes_read = 0;

            if (m_device_handle != constants::sc_invalid_file_handle)
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

        uint16_t SPI::write(const data::Buffer &buffer)
        {
            uint16_t bytes_written = 0;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                bytes_written = ::write(m_device_handle, buffer.data(), buffer.size());
            }

            return bytes_written;
        }

        uint16_t SPI::transfer(const data::Buffer &output_buffer, data::Buffer &input_buffer)
        {
            uint16_t bytes_transferred = 0;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                struct spi_ioc_transfer	xfer[sc_spi_max_messages];

                memset(xfer, 0, sizeof(xfer));

                xfer[sc_tx_buffer_slot].tx_buf = (unsigned long)output_buffer.data();
                xfer[sc_tx_buffer_slot].len = output_buffer.size();
                xfer[sc_tx_buffer_slot].delay_usecs = 0;
                xfer[sc_tx_buffer_slot].speed_hz = m_frequency;
                xfer[sc_tx_buffer_slot].bits_per_word = m_bits_per_word;
                xfer[sc_rx_buffer_slot].rx_buf = (unsigned long)input_buffer.data();
                xfer[sc_rx_buffer_slot].len = input_buffer.size();
                xfer[sc_rx_buffer_slot].delay_usecs = 0;
                xfer[sc_rx_buffer_slot].speed_hz = m_frequency;
                xfer[sc_rx_buffer_slot].bits_per_word = m_bits_per_word;

                int ioctl_bytes = ioctl(m_device_handle, SPI_IOC_MESSAGE(sc_spi_max_messages), &xfer);
                if (ioctl_bytes != -1)
                {
                    bytes_transferred = (uint16_t)ioctl_bytes;
                }
            }
			return bytes_transferred;
        }

        // internal parts
        bool SPI::setup_device()
        {
            bool success = false;
            char device_name[sc_max_device_name] = {0};

            snprintf(device_name, sc_max_device_name - 1, sc_spi_dev.c_str(), get_instance(), get_device());

            std::string device_file_name = constants::sc_device_path + "/" + device_name;

            m_device_handle = ::open(device_file_name.c_str(), O_RDWR, O_RDWR);

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                if (set_mode(SPI_MODE_3) == true)
                {
uint8_t spi_msb = 0;
		ioctl(m_device_handle, SPI_IOC_RD_LSB_FIRST, &spi_msb);
		ioctl(m_device_handle, SPI_IOC_WR_LSB_FIRST, &spi_msb);
                    if (set_bits_per_word(sc_default_bits_per_word) == true)
                    {
                        success = set_frequency(sc_default_frequency);
                    }
                }
            }

            return success;
        }

        void SPI::shutdown_device()
        {
            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                ::close(m_device_handle);
                m_device_handle = constants::sc_invalid_file_handle;
            }
        }

        // private parts
        bool SPI::set_frequency(uint32_t frequency)
        {
            bool success = false;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
				if (ioctl(m_device_handle, SPI_IOC_WR_MAX_SPEED_HZ, &frequency) != -1)
				{
					if (ioctl(m_device_handle, SPI_IOC_RD_MAX_SPEED_HZ, &frequency) != -1)
                    {
                        m_frequency = frequency;
                        success = true;
                    }
                }
            }

            return success;
        }
        
        bool SPI::set_mode(uint8_t mode)
        {
            bool success = false;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
				if (ioctl(m_device_handle, SPI_IOC_WR_MODE, &mode) != -1)
				{
					if (ioctl(m_device_handle, SPI_IOC_RD_MODE, &mode) != -1)
                    {
                        success = true;
                    }
                }
            }

            return success;
        }

        bool SPI::set_bits_per_word(uint8_t num_bits)
        {
            bool success = false;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
				if (ioctl(m_device_handle, SPI_IOC_WR_BITS_PER_WORD, &num_bits) != -1)
				{
					if (ioctl(m_device_handle, SPI_IOC_RD_BITS_PER_WORD, &num_bits) != -1)
                    {
                        m_bits_per_word = num_bits;
                        success = true;
                    }
                }
            }

            return success;
        }
    }
}
