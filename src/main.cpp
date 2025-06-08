#include "process_info.h"
#include "snmp_agent.h"

#include <csignal>
#include <iostream>
#include <atomic>

// stop flag
std::atomic_bool stopped = false;

int main()
{
	auto stopfunc = []( int ) { stopped = true; };

	// stopping on this signals
	std::signal( SIGINT, stopfunc );
	std::signal( SIGTERM, stopfunc );

#ifdef NDEBUG
	// output current processes
	for ( const auto & proc : GetAllMatchingProcesses() )
	{
		std::cout << proc.pid << " " << proc.user << ":" << proc.name << " [" << proc.status << "] "
				  << "cputime=" << proc.cputime << ", memory=" << proc.memory << " KiB\n";
	}
	std::cout.flush();
#endif

	try
	{
		std::string agentName = "snmp_test_agent";
		std::string agentSocket = "/var/agentx/master";
		SnmpAgent agent( agentName, agentSocket );

		agent.Run( stopped );	
	}
	catch ( const std::exception & ex )
	{
		std::cerr << "Exception occurred: " << ex.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
