#pragma once

#ifndef SNMP_CONSTANTS_H
#define SNMP_CONSTANTS_H

#include "netsnmp-includes.h"

#include <vector>

typedef std::vector< oid > OIDVec;

namespace oids
{
    const OIDVec hrSWRunTable = { 1, 3, 6, 1, 2, 1, 25, 4, 2 };
    const std::vector<unsigned> hrSWRunTableVC = { 1, 2, 7 };

    const OIDVec hrSWRunPerfTable = { 1, 3, 6, 1, 2, 1, 25, 5, 1 };
    const std::vector<unsigned> hrSWRunPerfTableVC = { 1, 2 };
}

#endif // SNMP_CONSTANTS_H