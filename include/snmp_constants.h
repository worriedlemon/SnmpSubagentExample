#pragma once

#ifndef SNMP_CONSTANTS_H
#define SNMP_CONSTANTS_H

#include "netsnmp-includes.h"

#include <vector>

/// @brief type definition for vector representing OID
typedef std::vector< oid > OIDVec;

namespace oids
{
    /// @brief `hrSWRunTable` OID
    const OIDVec hrSWRunTable = { 1, 3, 6, 1, 2, 1, 25, 4, 2 };

    /// @brief `hrSWRunPerfTable` OID
    const OIDVec hrSWRunPerfTable = { 1, 3, 6, 1, 2, 1, 25, 5, 1 };
}

#endif // SNMP_CONSTANTS_H