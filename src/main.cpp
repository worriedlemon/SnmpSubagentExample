#include "process_info.h"
#include "snmp_agent.h"

#include <csignal>
#include <iostream>
#include <atomic>

std::atomic_bool stopped = false;

int main()
{
	auto stopfunc = []( int ) { stopped = true; };

	std::signal( SIGINT, stopfunc );
	std::signal( SIGTERM, stopfunc );

	try
	{
		std::string agentName = "snmp_test_agent";
		SnmpAgent agent( agentName );

		agent.Run( stopped );	
	}
	catch ( const std::exception & ex )
	{
		std::cerr << "Exception occurred: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
