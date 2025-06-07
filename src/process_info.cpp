#include "process_info.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <pwd.h>
#include <unistd.h>
#include <sys/stat.h>


std::vector< ProcessInfo > GetAllMatchingProcesses()
{
    std::vector< ProcessInfo > result;

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
        stat( entry.path().c_str(), &st );
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
        for ( const auto & description : monitorProcesses )
        {
            if ( processName.find( description.name ) != std::string::npos && userName == description.user )
            {
                info.name = description.name;
                info.user = description.user;
                info.pid = entryName;
                found = true;
                break;
            }
        }

        if ( !found )
        {
            continue;
        }
        
        std::ifstream statFile( pidPath / "stat" );
        std::string stat;
        std::getline( statFile, stat );

        std::stringstream ss;
        ss << stat;
        std::vector< std::string > stats;
        while ( std::getline( ss, stat, ' ' ) )
        {
            stats.push_back( stat );
        }

        std::vector< std::string >::iterator newEnd = stats.end();
        while ( !stats[ 1 ].ends_with( ')' ) )
        {
            stats[ 1 ] += stats[ 2 ];
            newEnd = std::remove( stats.begin(), stats.end(), stats[ 2 ] );
        }
        stats.erase( newEnd, stats.end() );

        // Пункт (2) - состояние процесса (state)
        info.status = stats[ 2 ][ 0 ];

        uint64_t clockTicks = sysconf( _SC_CLK_TCK );
        // Пункт (14) - время в режиме пользователя (utime), пункт (15) - время в режиме ядра (stime)
        info.cputime = ( std::stoll( stats[ 13 ] ) + std::stoll( stats[ 14 ] ) ) * ( 100.0 / clockTicks );

        int pagesize = getpagesize();
        // Пункт (23) - используемая память (vsize) 
        info.memory = std::stoll( stats[ 22 ] ) * ( pagesize / 1024 );

        result.push_back( std::move( info ) );
    }

    return result;
}