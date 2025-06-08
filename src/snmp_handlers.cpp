#include "snmp_handlers.h"
#include "snmp_constants.h"
#include "process_info.h"

#include <iostream>

namespace handlers
{


namespace
{

/// @brief Cached processes
std::vector< ProcessInfo > processCache;

/// @brief Last process caching timestamp
int64_t processCacheTs = 0;

/// @brief Process caching delta
const int64_t processCacheTsDelta = std::chrono::seconds( 5 ).count();

/// @brief type definition for a function to handle one run table setting
typedef void oneSettingHandlerT(
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * request,
	ProcessInfo& pi,
	oid setting
);

/// @brief Internal implementation for a run table handler
/// @param reqinfo - agent request info
/// @param requests - request info
/// @param oidVec - vector representing OID
/// @param vc - valid columns for current table
/// @param oneSettingHandler - subhandler for specific setting
void RunTableHandlerImpl(
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * requests,
	const OIDVec & oidVec,
	const std::vector<unsigned> & vc,
	oneSettingHandlerT* oneSettingHandler
)
{
	try
	{
		if ( reqinfo->mode != MODE_GET && reqinfo->mode != MODE_GETNEXT )
		{
			netsnmp_request_set_error_all( requests, reqinfo->mode );
			return;
		}

		auto ts = std::chrono::system_clock::now().time_since_epoch().count();
		if ( ts >= processCacheTs + processCacheTsDelta )
		{
			processCacheTs = ts;
			processCache = GetAllMatchingProcesses();
		}

		for ( auto * request = requests; request != nullptr; request = request->next )
		{
			auto* tableInfo = netsnmp_extract_table_info( request );
			oid setting = tableInfo->colnum;
			oid index = *tableInfo->indexes->val.integer;

			if ( reqinfo->mode == MODE_GET && ( index == 0 || index > processCache.size() ))
			{
				netsnmp_set_request_error( reqinfo, request, SNMP_NOSUCHOBJECT );
				continue;
			}
			else if ( reqinfo->mode == MODE_GETNEXT )
			{
				if ( index == processCache.size() )
				{
					if ( setting == vc.back() )
					{
						setting = vc.back() + 1;
					}
					else
					{
						setting = *( ++std::find( vc.begin(), vc.end(), setting ) );
					}
					index = 1;
				}
				else
				{
					++index;
				}
				
				OIDVec nextOid = oidVec;
				nextOid.push_back( 1 );
				nextOid.push_back( setting );
				nextOid.push_back( index );

				// setting next oid
				int ret = snmp_set_var_objid( request->requestvb, nextOid.data(), nextOid.size() );
				if ( ret != SNMPERR_SUCCESS )
				{
					std::cerr << "Error!" << std::endl;
					continue;
				}
				
				// past the end handling
				if ( setting > vc.back() )
				{
					continue;
				}
			}

			ProcessInfo pi = processCache[ index - 1 ];

			oneSettingHandler( reqinfo, request, pi, setting );
		}
	}
	catch ( const std::exception & ex )
	{
		std::cerr << "Exception occurred: " << ex.what() << std::endl;
	}
}

/// @brief Subhandler for one setting of `hrSWRunTable`
/// @param reqinfo - agent request info
/// @param request - request info
/// @param[in] pi - process info
/// @param setting - setting 
void HandleOneSettingRunTable(
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * request,
	ProcessInfo& pi,
	oid setting
)
{
	switch ( setting )
	{
		case 1:
		{
			uint32_t value = std::stoi( pi.pid );
			snmp_set_var_typed_value( request->requestvb, ASN_INTEGER, &value, sizeof( value ) );
			break;
		}
		case 2:
		{
			std::string value;
			value.append( pi.user ).append( ":" ).append( pi.name );
			snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR, value.c_str(), value.size() );
			break;
		}
		case 7:
		{
			uint32_t value;
			switch ( pi.status )
			{
				case 'R':
					value = 1;
					break;
				case 'S':
					value = 2;
					break;
				case 'T':
				case 'D':
					value = 3;
					break;
				default:
					value = 4;
					break;
			}
			
			snmp_set_var_typed_value( request->requestvb, ASN_INTEGER, &value, sizeof( value ) );
			break;
		}
		default:
		{
			netsnmp_set_request_error( reqinfo, request, SNMP_NOSUCHOBJECT );
		}
	}
}

/// @brief Subhandler for one setting of `hrSWRunPerfTable`
/// @param reqinfo - agent request info
/// @param request - request info
/// @param[in] pi - process info
/// @param setting - setting 
void HandleOneSettingRunPerfTable(
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * request,
	ProcessInfo& pi,
	oid setting
)
{
	switch ( setting )
	{
		case 1:
		{
			uint32_t value = pi.cputime;
			snmp_set_var_typed_value( request->requestvb, ASN_INTEGER, &value, sizeof( value ) );
			break;
		}
		case 2:
		{
			uint32_t value = pi.memory;
			snmp_set_var_typed_value( request->requestvb, ASN_INTEGER, &value, sizeof( value ) );
			break;
		}
		default:
		{
			netsnmp_set_request_error( reqinfo, request, SNMP_NOSUCHOBJECT );
		}
	}
}


} // namespace


int hrSWRunTableHandler(
	netsnmp_mib_handler *,
	netsnmp_handler_registration *,
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * requests
)
{
	RunTableHandlerImpl( reqinfo, requests, oids::hrSWRunTable, oids::hrSWRunTableVC, HandleOneSettingRunTable );
	return SNMP_ERR_NOERROR;
}


int hrSWRunPerfTableHandler(
	netsnmp_mib_handler *,
	netsnmp_handler_registration *,
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * requests
)
{
	RunTableHandlerImpl( reqinfo, requests, oids::hrSWRunPerfTable, oids::hrSWRunPerfTableVC, HandleOneSettingRunPerfTable );
	return SNMP_ERR_NOERROR;
}


} // namespace handlers