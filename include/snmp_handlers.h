#pragma once

#ifndef SNMP_HANDLERS_H
#define SNMP_HANDLERS_H

#include "netsnmp-includes.h"

namespace handlers
{

int hrSWRunTableHandler(
	netsnmp_mib_handler *,
	netsnmp_handler_registration *,
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * requests
);

int hrSWRunPerfTableHandler(
	netsnmp_mib_handler *,
	netsnmp_handler_registration *,
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * requests
);

} // namespace handlers

#endif // SNMP_HANDLERS_H