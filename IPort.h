/**
 * IPort.h
 * 
 * Interface defining a port that can read/write
 * 
 * Copyright 2020 AFM Software
 */

#ifndef _H_IPORT
#define _H_IPORT

#include <cstdint>
#include <memory>

#include "DataTypes.h"

namespace afm
{
    namespace communication
    {
        class IPort
        {
            public:
                virtual ~IPort() {}

                virtual bool initialize(uint32_t instance, uint32_t device) = 0;

                virtual bool read(uint8_t &value) = 0;
                virtual bool write(uint8_t value) = 0;
                virtual uint16_t read(data::Buffer &buffer) = 0;
                virtual uint16_t write(const data::Buffer &buffer) = 0;
                virtual uint16_t transfer(const data::Buffer &output_buffer, data::Buffer &input_buffer) = 0;

                virtual bool is_port(uint32_t instance, uint32_t device) = 0;
        };

        using IPortSPtr = std::shared_ptr<IPort>;
    }
}
#endif