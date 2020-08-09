/**
 * GPIO.h
 * 
 * GPIO implementation defining a port that can read/write
 * 
 * Copyright 2020 AFM Software
 */

#ifndef _H_GPIO
#define _H_GPIO

#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <thread>

#include "Constants.h"
#include "Port.h"

namespace afm
{
    namespace communication
    {        
        class GPIO : public Port
        {
            public:
                GPIO();

                virtual bool read(uint8_t &value) override;
                virtual bool write(uint8_t value) override;
                virtual uint16_t read(data::Buffer &buffer) override;
                virtual uint16_t write(const data::Buffer &buffer) override;
                virtual uint16_t transfer(const data::Buffer &output_buffer, data::Buffer &input_buffer) override;

                bool enable_interrupt(data::GPIOInterruptEdge edge, data::GPIOInterruptRoutine interrupt_callback);

            protected:
                virtual bool setup_device() override;
                virtual void shutdown_device() override;
                void run();

            private:
                void set_direction(bool make_input);

            private:
                bool                        m_interrupt_active = false;
                bool                        m_is_write = false;
                std::atomic<bool>           m_pin_exported;
                int                         m_device_handle = constants::sc_invalid_file_handle;
                std::string                 m_base_path;
                data::GPIOInterruptRoutine  m_interrupt_callback = nullptr;
                std::thread                 m_interrupt_observer_thread;
                uint8_t                     m_interrupt_edge = afm::constants::sc_gpio_low;
       };

       using GPIOSPtr = std::shared_ptr<GPIO>;
    }
}
#endif