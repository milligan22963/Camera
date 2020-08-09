/**
 * SPI.h
 * 
 * SPI implementation defining a port that can read/write
 * 
 * Copyright 2020 AFM Software
 */

#ifndef _H_SPI
#define _H_SPI

#include "Constants.h"
#include "Port.h"

namespace afm
{
    namespace communication
    {
        class SPI : public Port
        {
            public:
                SPI();

                virtual bool read(uint8_t &value) override;
                virtual bool write(uint8_t value) override;
                virtual uint16_t read(data::Buffer &buffer) override;
                virtual uint16_t write(const data::Buffer &buffer) override;
                virtual uint16_t transfer(const data::Buffer &output_buffer, data::Buffer &input_buffer) override;

            protected:
                virtual bool setup_device() override;
                virtual void shutdown_device() override;

            private:
                bool set_frequency(uint32_t frequency);
                bool set_mode(uint8_t mode);
                bool set_bits_per_word(uint8_t num_bits);

            private:
                int         m_device_handle = constants::sc_invalid_file_handle;
                uint8_t     m_bits_per_word;
                uint32_t    m_frequency;
        };
    }
}
#endif