/**
 * Constants.h
 * 
 * Various constants used by the application
 * 
 * Copyright 2020 AFM Software
 */

#ifndef _H_CONSTANTS
#define _H_CONSTANTS

#include <cstdint>
#include <string>
#include <vector>

#include "DataTypes.h"

namespace afm
{
    namespace constants
    {
        const std::string sc_device_path = "/dev";

        const int sc_invalid_file_handle = -1;

        const uint8_t sc_gpio_high = 1;
        const uint8_t sc_gpio_low = 0;

        const data::Color RED(0xFF, 0x00, 0x00);
        const data::Color GREEN(0x00, 0xFF, 0x00);
        const data::Color BLUE(0x00, 0x00, 0xFF);
        const data::Color BLACK(0x00, 0x00, 0x00);
        const data::Color WHITE(0xFF, 0xFF, 0xFF);
        const data::Color GREY(0x7F, 0x7F, 0x7F);

        const std::string sc_gpio_port = "gpio";
        const std::string sc_i2c_port = "i2c";
        const std::string sc_spi_port = "spi";

        const std::string sc_sesp525_display = "sesp525";
    }
}
#endif