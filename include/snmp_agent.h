#pragma once

#ifndef SNMP_SUBAGENT_H
#define SNMP_SUBAGENT_H

#include "snmp_constants.h"

#include <sstream>
#include <string>
#include <string_view>
#include <atomic>

/// @brief SNMP subagent class
class SnmpAgent
{
public:
    /// @brief Constructor for subagent
    /// @param name - subagent name
    /// @param socket - subagent socket to connect to
    explicit SnmpAgent( const std::string & name, const std::string & socket );

    virtual ~SnmpAgent();

    /// @brief Run agent
    /// @param stopped - global stop flag reference
    void Run( std::atomic_bool& stopped );

protected:
    // Function for registering oids
    virtual void RegisterOids();

private:
    /// @brief Checking for NetSNMP return values
    /// @param ret - return values
    /// @param funcName - function name
    /// @throws std::invalid_argument on `ret != SNMPERR_SUCCESS` 
    void CheckSnmpRetVal( int ret, std::string_view funcName );

    /// @brief Register table
    /// @param oidName - name
    /// @param oidVec - vector representing OID
    /// @param vc - valid columns for table
    /// @param handler - handler for OID
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