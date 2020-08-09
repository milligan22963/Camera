/**
 * GPIO.cpp
 * 
 * GPIO implementation defining a port that can read/write
 * 
 * Copyright 2020 AFM Software
 */

#include <iostream>
#include <error.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "bcm_host.h"
#include "vcos/vcos.h"

#include "mmal/mmal.h"
#include "mmal/mmal_logging.h"

#include "GPIO.h"

namespace afm
{
    namespace communication
    {
        const std::string sc_gpio_base = "/sys/class/gpio";
        const std::string sc_gpio_out = "out";
        const std::string sc_gpio_in = "in";
        const char *sc_gpio_low = "0";
	const char *sc_gpio_high = "1";
        const std::string sc_gpio_export = "export";
        const std::string sc_gpio_unexport = "unexport";
        const std::string sc_gpio_direction = "direction";
        const std::string sc_gpio_value = "value";
        const std::string sc_gpio_edge_type[] =
        {
            "none",
            "falling",
            "rising",
            "both"
        };
        const std::string sc_gpio_edge = "edge";

        GPIO::GPIO()
            : m_pin_exported(false)
        {

        }

        bool GPIO::read(uint8_t &value)
        {
            bool success = false;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                if (m_is_write == true)
                {
                    set_direction(true);
                }

                lseek(m_device_handle, 0, 0);

                char gpio_value;

                size_t bytes_read = ::read(m_device_handle, &gpio_value, 1);
                if (bytes_read == 1)
                {
                    // convert from ascii to low/high
                    value = gpio_value == '1' ? afm::constants::sc_gpio_high : afm::constants::sc_gpio_low;
                    success = true;
                }
            }

            return success;
        }

        bool GPIO::write(uint8_t value)
        {
            bool success = false;

            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                if (m_is_write == false)
                {
                    set_direction(false);
                }
                const char *gpio_value = value == 1 ? sc_gpio_high : sc_gpio_low;
                if (::write(m_device_handle, gpio_value, 1) == 1)
                {
                    success = true;
                }
            }

            return success;
        }

        uint16_t GPIO::read(data::Buffer &buffer)
        {
            bool success = false;
            uint8_t value;

            if (read(value) == true)
            {
                buffer.push_back(value);
                success = true;
            }

            return success == true ? 1 : 0;
        }

        uint16_t GPIO::write(const data::Buffer &buffer)
        {
            return write(buffer[0]) == true ? 1 : 0;
        }

        uint16_t GPIO::transfer(const data::Buffer &output_buffer, data::Buffer &input_buffer)
        {
            return 0; // can't really do a transfer
        }

        bool GPIO::enable_interrupt(data::GPIOInterruptEdge edge, data::GPIOInterruptRoutine interrupt_callback)
        {
            bool success = false;

            if (edge == data::GPIOInterruptEdge::RISING) {
                m_interrupt_edge = afm::constants::sc_gpio_high;
            } else {
                m_interrupt_edge = afm::constants::sc_gpio_low;
            }

            m_interrupt_callback = interrupt_callback;

            set_direction(true); // interrupts are input oddly enough

            std::string gpio_path = m_base_path + "/" + sc_gpio_edge;

            int file_handle = open(gpio_path.c_str(), O_WRONLY);
            if (file_handle != -1)
            {
                if (::write(file_handle, sc_gpio_edge_type[edge].c_str(), sc_gpio_edge_type[edge].length()) >= 0)
                {
                    success = true;
                }
                close(file_handle);

                // do after file handle closed
                if (success == true)
                {
                    m_interrupt_active = true;
                    m_interrupt_observer_thread = std::thread(&GPIO::run, this);
                }
            }
            return success;
        }

        // internal parts
        bool GPIO::setup_device()
        {
            bool success = false;
            // instance is pin
            // device will be bank if used
            // export the pin so we can utilize it
            char buffer[32];
            struct stat dir_stats;
            bool already_exported = false;

            int length = snprintf(buffer, sizeof(buffer), "%u", get_instance());

            m_base_path = sc_gpio_base + "/gpio" + buffer; // save so we can access it later

            // see if we already exported
            if (stat(m_base_path.c_str(), &dir_stats) == 0) {
                if (S_ISDIR(dir_stats.st_mode) || S_ISLNK(dir_stats.st_mode)) {
                    already_exported = true;
                    m_pin_exported = true;
                    success = true;
                }
            }

            if (already_exported == false) {
                // Create gpio access string
                std::string gpio_path = sc_gpio_base + "/" + sc_gpio_export;

                int file_handle = ::open(gpio_path.c_str(), O_WRONLY);

                if (file_handle >= 0) {
                    if (::write(file_handle, buffer, length) == length) {
                        m_pin_exported = true;
                        success = true;
                    }
                    ::close(file_handle);
                }
            }

            // default to output
            set_direction(false);

            // Open it
            std::string gpio_path = m_base_path + "/" + sc_gpio_value;

            m_device_handle = ::open(gpio_path.c_str(), O_WRONLY);

            return success;
        }

        void GPIO::shutdown_device()
        {
            if (m_device_handle != constants::sc_invalid_file_handle)
            {
                ::close(m_device_handle);
            }

            if (m_pin_exported == true)
            {
                m_pin_exported = false;

                 // Create gpio access string
                std::string gpio_path = sc_gpio_base + "/" + sc_gpio_unexport;

                int file_handle = ::open(gpio_path.c_str(), O_WRONLY);

                if (file_handle >= 0) {
                    char buffer[32];

                    int length = snprintf(buffer, sizeof(buffer), "%u", get_instance());

                    if (::write(file_handle, buffer, length) <= 0) {
                        // error
                    }
                    ::close(file_handle);
                }
            }

            if (m_interrupt_active == true)
            {
                m_interrupt_active = false;
                m_interrupt_observer_thread.join();
            }
        }

        void GPIO::run()
        {
            int32_t interrupt_debounce = 1000 * 1000 * 200; // 1000 for nanoseconds, 1000 for microseconds, 200 for milli

            if (m_device_handle >= 0) {
                struct timespec last_time;
                struct timeval tv = {1, 0}; // 1 second timeout
                fd_set interrupt_descriptor_set;
                char interrupt_value[64];
                bool transition_seen = true;

                // default to current time for last
                clock_gettime(CLOCK_REALTIME, &last_time);

                while (m_interrupt_active == true) {

                    // reset due to Linux modifying the return value
                    tv.tv_sec = 1;
                    tv.tv_usec = 0;

                    // Done in the loop so we reset after every select
                    FD_ZERO(&interrupt_descriptor_set);
                    FD_SET(m_device_handle, &interrupt_descriptor_set);

                    if (select(m_device_handle + 1, nullptr, nullptr, &interrupt_descriptor_set, &tv) > 0) {
                        vcos_log_error("We were selected...");

                        if (FD_ISSET(m_device_handle, &interrupt_descriptor_set)) {
                            struct timespec current_time;

                            // Process interrupt if we transition from high to low
                            bool processInterrupt = true;

                            if (clock_gettime(CLOCK_REALTIME, &current_time) != -1) {
                                if (current_time.tv_sec == last_time.tv_sec) {

                                    // is current time greater than last time i.e. is it debounced...
                                    if (current_time.tv_nsec < (last_time.tv_nsec + interrupt_debounce)) {
                                        processInterrupt = false;
                                    }
                                }
                            }

                            lseek(m_device_handle, 0, 0);

                            size_t bytesRead = ::read(m_device_handle, interrupt_value, 64);
                            if (bytesRead > 0) {
                                interrupt_value[bytesRead] = '\0';
                                if (atoi(interrupt_value) == m_interrupt_edge) {
                                    if ((processInterrupt == true) && (transition_seen == true)) {
                                        m_interrupt_callback(get_instance());
                                        last_time = current_time;
                                    }
                                    transition_seen = false;
                                } else {
                                    transition_seen = true;
                                }
                            }
                        }
                    }
                    else
                    {
                        vcos_log_error("Select timed out...");
                        m_interrupt_active = false;
                    }
                }
            }
            else {
                vcos_log_error("Device handle is closed");
            }
        }

        // private parts
        void GPIO::set_direction(bool make_input)
        {
            // Create gpio access string
            std::string gpio_path = m_base_path + "/" + sc_gpio_direction;

            int file_handle = ::open(gpio_path.c_str(), O_WRONLY);

            if (file_handle != constants::sc_invalid_file_handle) {
                std::string direction_flag = sc_gpio_out;
                if (make_input == true) {
                    direction_flag = sc_gpio_in;
                }
                if (::write(file_handle, direction_flag.c_str(), direction_flag.length()) == (int)direction_flag.length()) {
                    m_is_write = make_input == false;
                }
                ::close(file_handle);

                ::close(m_device_handle);

                std::string gpio_path = m_base_path + "/" + sc_gpio_value;
                if (m_is_write == false) {
                    m_device_handle = ::open(gpio_path.c_str(), O_RDONLY);
                } else {
                    m_device_handle = ::open(gpio_path.c_str(), O_WRONLY);
                }
            }
        }
    }
}
