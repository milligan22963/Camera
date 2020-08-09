/**
 * PortFactory.h
 * 
 * Creates ports based on user needs
 * 
 * Copyright 2020 AFM Software
 */

#ifndef _H_PORT_FACTORY
#define _H_PORT_FACTORY

#include <map>
#include <memory>

#include "DataTypes.h"
#include "IPort.h"

namespace afm
{
    namespace communication
    {
        using PortMap = std::multimap<data::PortType, IPortSPtr>;

        class PortFactory;

        using PortFactorySPtr = std::shared_ptr<PortFactory>;

        class PortFactory
        {
            public:
                PortFactory();
                virtual ~PortFactory();

                static PortFactorySPtr getInstance();

                IPortSPtr createPort(data::PortType port_type, uint32_t instance, uint32_t device);
                IPortSPtr createPort(std::string port_type, uint32_t instance, uint32_t device);

            private:
                PortMap m_ports;
        };
    }
}
#endif