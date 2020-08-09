/**
 * Port.h
 * 
 * Base implementation defining a port that can read/write
 * 
 * Copyright 2020 AFM Software
 */

#ifndef _H_PORT
#define _H_PORT

#include <cstdint>
#include <memory>

#include "DataTypes.h"
#include "IPort.h"

namespace afm
{
    namespace communication
    {
        class Port : public IPort
        {
            public:
                Port();
                virtual ~Port();

                virtual bool initialize(uint32_t instance, uint32_t device) final;

                virtual bool read(uint8_t &value) override;
                virtual bool write(uint8_t value) override;
                virtual uint16_t read(data::Buffer &buffer) override;
                virtual uint16_t write(const data::Buffer &buffer) override;
                virtual uint16_t transfer(const data::Buffer &output_buffer, data::Buffer &input_buffer) override;
                virtual bool is_port(uint32_t instance, uint32_t device) final;

            protected:
                virtual bool setup_device() = 0;
                virtual void shutdown_device() { }

                uint32_t get_instance() { return m_instance; }
                uint32_t get_device() { return m_device; }

            private:
                uint32_t    m_instance = 0;
                uint32_t    m_device = 0;
        };
    }
}
#endif