/**
 * DataTypes.h
 * 
 * Various data types used by the application
 * 
 * Copyright 2020 AFM Software
 */

#ifndef _H_DATA_TYPES
#define _H_DATA_TYPES

#include <cstdint>
#include <functional>
#include <map>
#include <vector>

namespace afm
{
    namespace data
    {
        using BufferDataType = uint8_t;
        using Buffer = std::vector<uint8_t>;

        /**
         * Port specific data types and enumerations
         */
        enum PortType
        {
            PORT_I2C,
            PORT_SPI,
            PORT_GPIO,
            END_PORT_TYPES
        };

        /**
         * GPIO specific data types and enumerations
         */
        using GPIOInterruptRoutine = std::function<void (uint8_t)>;

        enum GPIOInterruptEdge
        {
            NONE,
            FALLING,
            RISING,
            BOTH,
            END_GPIO_INTERRUPT_EDGES
        };

        template <typename T>
        struct Coordinate
        {
            Coordinate(T xval, T yval)
            {
                x = xval;
                y = yval;
            }
            T x;
            T y;
        };

        using Coordinate_8t = Coordinate<uint8_t>;

        /**
         * Colors
         */
        struct Color
        {
            Color(uint8_t r, uint8_t g, uint8_t b)
            {
                red = r;
                green = g;
                blue = b;
            }
            uint8_t red;
            uint8_t blue;
            uint8_t green;
        };

        /**
         * Display Types
         */
        enum DisplayType
        {
            SESP525,
            END_DISPLAY_TYPES
        };

        /**
         * Image Types
         */
         enum ImageType
         {
             IMG_JPEG,
             IMG_GIF,
             IMG_PNG,
             IMG_PPM,
             IMG_TGA,
             IMG_BMP,
             IMG_RAW,
             END_IMAGE_TYPES
         };

         /**
          * Image Extensions
          */
          const std::string sc_image_extensions[] =
          {
              "jpg",
              "gif",
              "png",
              "ppm",
              "tga",
              "bmp",
              "raw"
          };
    }
}
#endif