#pragma once

#ifndef SNMP_SUBAGENT_H
#define SNMP_SUBAGENT_H

#include "snmp_constants.h"

#include <sstream>
#include <string>
#include <string_view>
#include <atomic>

class SnmpAgent
{
public:
    explicit SnmpAgent( const std::string & name );

    ~SnmpAgent();

    void Run( std::atomic_bool& stopped );

protected:
    virtual void RegisterOids();

private:
    void CheckSnmpRetVal( int ret, std::string_view funcName );

    void RegisterOidTable(
        const std::string & oidName,
        const OIDVec & oidVec,
        const std::vector< unsigned > & vc,
        Netsnmp_Node_Handler* handler
    );


private:
    std::string agentName;
};

#endif // SNMP_SUBAGENT_H