/**
 * PortFactory.cpp
 * 
 * Creates ports based on user needs
 * 
 * Copyright 2020 AFM Software
 */

#include "PortFactory.h"

#include "GPIO.h"
#include "I2C.h"
#include "SPI.h"

namespace afm
{
    namespace communication
    {
        PortFactory::PortFactory()
        {

        }

        PortFactory::~PortFactory()
        {
            m_ports.clear();
        }

        PortFactorySPtr PortFactory::getInstance()
        {
            static PortFactorySPtr p_instance = nullptr;

            if (p_instance == nullptr)
            {
                p_instance = std::make_shared<PortFactory>();
            }

            return p_instance;
        }

        IPortSPtr PortFactory::createPort(data::PortType port_type, uint32_t instance, uint32_t device)
        {
            IPortSPtr p_port = nullptr;

            // might already exist, lets check
            for (auto iter = m_ports.lower_bound(port_type); iter != m_ports.upper_bound(port_type); iter++)
            {
                if (iter->second->is_port(instance, device) == true)
                {
                    p_port = iter->second;
                    break;
                }
            }

            if (p_port == nullptr)
            {
                switch (port_type)
                {
                    case data::PortType::PORT_I2C:
                    {
                        p_port = std::make_shared<communication::I2C>();
                    }
                    break;
                    case data::PortType::PORT_SPI:
                    {
                        p_port = std::make_shared<communication::SPI>();
                    }
                    break;
                    case data::PortType::PORT_GPIO:
                    {
                        p_port = std::make_shared<communication::GPIO>();
                    }
                    break;
                    default:
                    {
                        // error
                    }
                    break;
                }

                if (p_port != nullptr)
                {
                    if (p_port->initialize(instance, device) == true)
                    {
                        m_ports.insert(std::make_pair(port_type, p_port));
                    }
                    else
                    {
                        p_port = nullptr;
                    }                    
                }
            }

            return p_port;
        }

        IPortSPtr PortFactory::createPort(std::string port_type, uint32_t instance, uint32_t device)
        {
            data::PortType type = data::PortType::END_PORT_TYPES;

            if (port_type == constants::sc_gpio_port)
            {
                type = data::PortType::PORT_GPIO;
            }
            else if (port_type == constants::sc_i2c_port)
            {
                type = data::PortType::PORT_I2C;
            }
            else if (port_type == constants::sc_spi_port)
            {
                type = data::PortType::PORT_SPI;
            }

            return createPort(type, instance, device);
        }
    }
}