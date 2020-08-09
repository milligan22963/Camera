/**
 * Port.cpp
 * 
 * Copyright 2020 AFM Software
 */

#include "Port.h"

namespace afm
{
    namespace communication
    {
        Port::Port()
        {

        }

        Port::~Port()
        {
            shutdown_device();
        }

        bool Port::initialize(uint32_t instance, uint32_t device)
        {
            m_instance = instance;
            m_device = device;

            return setup_device();
        }

        bool Port::read(uint8_t &value)
        {
            return false;
        }

        bool Port::write(uint8_t value)
        {
            return false;
        }

        uint16_t Port::read(data::Buffer &buffer)
        {
            return 0;
        }

        uint16_t Port::write(const data::Buffer &buffer) 
        {
            return 0;
        }

        uint16_t Port::transfer(const data::Buffer &output_buffer, data::Buffer &input_buffer)
        {
            uint16_t bytes_read = 0;

            if (write(output_buffer) > 0)
            {
                bytes_read = read(input_buffer);
            }
            return bytes_read;
        }

        bool Port::is_port(uint32_t instance, uint32_t device)
        {
            bool matches = false;

            if ((instance == m_instance) && (device == m_device))
            {
                matches = true;
            }

            return matches;
        }
    }
}