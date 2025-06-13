#include "process_info.h"

#include <filesystem>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>


std::map< uint32_t, ProcessInfo > GetAllMatchingProcesses()
{
    std::map< uint32_t, ProcessInfo > result;

    for ( const auto & entry : std::filesystem::directory_iterator( "/proc" ) )
    {
        std::filesystem::path pidPath = entry.path();
        std::string entryName = pidPath.stem();

        if ( !entry.is_directory() ||
             !std::all_of( entryName.begin(), entryName.end(), []( char c ) { return std::isdigit( c ); } ) )
        {
            continue;
        }

        ProcessInfo info{};

        struct stat st;
        stat( pidPath.c_str(), &st );
        passwd* pwd = getpwuid( st.st_uid );

        if ( pwd == nullptr )
        {
            continue;
        }
        
        std::string userName = pwd->pw_name;
        std::string processName;

        std::ifstream cmdlineStream( pidPath / "cmdline" );
        std::getline( cmdlineStream, processName );

        bool found = false;
        // Check whether this current process is in the monitorProcesses list
        for ( const auto & description : monitorProcesses )
        {
            if ( processName.find( description.name ) != std::string::npos && userName == description.user )
            {
                info.name = description.name;
                info.user = description.user;
                info.pid = std::stoul( entryName );
                found = true;
                break;
            }
        }

        if ( !found )
        {
            continue;
        }
        
        std::ifstream statFile( pidPath / "stat" );
        std::vector< std::string > stats;
        for ( std::string stat; std::getline( statFile, stat, ' ' ); )
        {
            stats.push_back( std::move( stat ) );
        }

        // Field (2) - process name, limited to 15 characters in parenthesis (spaces are possible)
        // Most commonly there will be zero iterations
        std::vector< std::string >::iterator newEnd = stats.end();
        while ( !stats[ 1 ].ends_with( ')' ) )
        {
            stats[ 1 ] += stats[ 2 ];
            newEnd = std::remove( stats.begin(), stats.end(), stats[ 2 ] );
        }
        stats.erase( newEnd, stats.end() );

        // Field (3) - process state as a character
        info.status = stats[ 2 ][ 0 ];

        uint64_t clockTicks = sysconf( _SC_CLK_TCK );
        // Field (14) - process user time (utime), field (15) - process kernel time (stime) in clock ticks
        info.cputime = ( std::stoll( stats[ 13 ] ) + std::stoll( stats[ 14 ] ) ) * ( 100.0 / clockTicks );

        int pagesize = getpagesize();
        // Field (24) - used memory in pages (rss) 
        info.memory = std::stoll( stats[ 23 ] ) * ( pagesize / 1024 );

        result[ info.pid ] = std::move( info );
    }

    return result;
}