#include "snmp_agent.h"
#include "snmp_handlers.h"

#include <thread>

SnmpAgent::SnmpAgent( const std::string & name )
    : agentName( name )
{
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
    netsnmp_ds_set_string(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_X_SOCKET, "/var/agentx/master");

    CheckSnmpRetVal( init_agent( agentName.c_str() ), "init_agent" );

    RegisterOids();

    init_snmp( agentName.c_str() );
}

SnmpAgent::~SnmpAgent()
{
    snmp_shutdown( agentName.c_str() );
}

void SnmpAgent::RegisterOids()
{
	using namespace oids;

    RegisterOidTable( "hrSWRunTable", hrSWRunTable, hrSWRunTableVC, handlers::hrSWRunTableHandler );
    RegisterOidTable( "hrSWRunPerfTable", hrSWRunPerfTable, hrSWRunPerfTableVC, handlers::hrSWRunPerfTableHandler );
}

void SnmpAgent::CheckSnmpRetVal( int ret, std::string_view funcName )
{
    if ( ret != SNMPERR_SUCCESS )
    {
        std::stringstream ss;
        ss << "Error in function " << funcName << " (" << snmp_errstring( ret ) << ")";
        throw std::invalid_argument( ss.str() );
    }
}

void SnmpAgent::RegisterOidTable(
	const std::string & oidName,
	const OIDVec & oidVec,
	const std::vector< unsigned > & vc,
	Netsnmp_Node_Handler* handler
)
{
	auto * vcPtr = new unsigned[ vc.size() ];
	std::copy( vc.begin(), vc.end(), vcPtr );

	auto * validColumns = SNMP_MALLOC_TYPEDEF( netsnmp_column_info );
	validColumns->isRange = 0;
	validColumns->list_count = vc.size();
	validColumns->details.list = vcPtr;

	auto * tableInfo = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
	tableInfo->min_column = vc.front();
	tableInfo->max_column = vc.back();
	tableInfo->valid_columns = validColumns;
	netsnmp_table_helper_add_index( tableInfo, ASN_INTEGER );

	auto * reg = netsnmp_create_handler_registration( oidName.c_str(), handler, oidVec.data(), oidVec.size(), HANDLER_CAN_RONLY );
	CheckSnmpRetVal( netsnmp_register_table( reg, tableInfo ), "netsnmp_register_table" );
}

void SnmpAgent::Run( std::atomic_bool& stopped )
{
    while ( !stopped )
	{
		agent_check_and_process(1);
	}
}