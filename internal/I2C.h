/**
 * I2C.h
 * 
 * I2C implementation defining a port that can read/write
 * 
 * Copyright 2020 AFM Software
 */

#ifndef _H_I2C
#define _H_I2C

#include "Constants.h"
#include "Port.h"

namespace afm
{
    namespace communication
    {
        class I2C : public Port
        {
            public:
                I2C();

                virtual bool read(uint8_t &value) override;
                virtual bool write(uint8_t value) override;
                virtual uint16_t read(data::Buffer &buffer) override;
                virtual uint16_t write(const data::Buffer &buffer) override;
                virtual uint16_t transfer(const data::Buffer &output_buffer, data::Buffer &input_buffer) override;

            protected:
                virtual bool setup_device() override;
                virtual void shutdown_device() override;

            private:
                bool select_address();

            private:
                int m_device_handle = constants::sc_invalid_file_handle;
        };
    }
}
#endif