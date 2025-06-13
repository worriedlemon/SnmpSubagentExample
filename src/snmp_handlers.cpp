#include "snmp_handlers.h"
#include "snmp_constants.h"
#include "process_info.h"

#include <iostream>

namespace handlers
{


namespace
{

/// @brief Cached processes
std::map< uint32_t, ProcessInfo > processCache;

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
/// @param oneSettingHandler - subhandler for specific setting
void RunTableHandlerImpl(
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * requests,
	const OIDVec & oidVec,
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

			auto * validColumns = tableInfo->reg_info->valid_columns;
			std::vector< unsigned int > vc( validColumns->details.list,
											validColumns->details.list + validColumns->list_count );

			if ( reqinfo->mode == MODE_GET && !processCache.contains( index ) )
			{
				netsnmp_set_request_error( reqinfo, request, SNMP_NOSUCHOBJECT );
				continue;
			}
			else if ( reqinfo->mode == MODE_GETNEXT )
			{
				if ( index >= ( --processCache.end() )->first )
				{
					if ( setting == vc.back() )
					{
						setting = vc.back() + 1;
					}
					else
					{
						setting = *( ++std::find( vc.begin(), vc.end(), setting ) );
					}
					index = processCache.begin()->first;
				}
				else
				{
					using pt = typename decltype( processCache )::value_type;
					auto res = std::find_if( processCache.begin(), processCache.end(),
								             [ index ]( const pt & p ) { return p.first > index; } );

					index = res->first;
				}
				
				OIDVec nextOid = oidVec;
				nextOid.push_back( 1 ); // one for table entry
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

			oneSettingHandler( reqinfo, request, processCache[ index ], setting );
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
			snmp_set_var_typed_value( request->requestvb, ASN_INTEGER, &pi.pid, sizeof( pi.pid ) );
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
				case 'R': // Runnable
					value = 1;
					break;
				case 'S': // Interruptible Sleep
					value = 2;
					break;
				case 'T': // Paused by job control
				case 'D': // Uninterruptible sleep
				case 't': // Paused by debugger
					value = 3;
					break;
				default:  // Other (Z - Zombie, X - Dead, I - Idle thread)
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
	RunTableHandlerImpl( reqinfo, requests, oids::hrSWRunTable, HandleOneSettingRunTable );
	return SNMP_ERR_NOERROR;
}


int hrSWRunPerfTableHandler(
	netsnmp_mib_handler *,
	netsnmp_handler_registration *,
	netsnmp_agent_request_info * reqinfo,
	netsnmp_request_info * requests
)
{
	RunTableHandlerImpl( reqinfo, requests, oids::hrSWRunPerfTable, HandleOneSettingRunPerfTable );
	return SNMP_ERR_NOERROR;
}


} // namespace handlers