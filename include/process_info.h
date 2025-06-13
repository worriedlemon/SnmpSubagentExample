#pragma once

#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H

#include "constexpr_helper.h"

#include <filesystem>
#include <map>
#include <optional>

/// @brief Default processes
constexpr const std::array< std::string_view, 3 > myProc = {
    "root:upowerd",
    "kernoops:kerneloops",
    "syslog:rsyslogd",
};

#ifndef OTHER_CORE_PROCESSES
/// @note If no other processes presented, than this string should be empty
#define OTHER_CORE_PROCESSES ""
#endif

/// @brief Other processes
constexpr const auto otherProc = WORD_STRING_TO_ARRAY( OTHER_CORE_PROCESSES );

struct ProcessDescription
{
    std::string_view user;
    std::string_view name;
};

template < std::size_t N >
constexpr std::array< ProcessDescription, N > SplitToUserAndName( std::array< std::string_view, N > procs )
{
    std::array< ProcessDescription, N > result;
    for ( std::size_t i = 0; i < N; ++i )
    {
        auto proc = procs[ i ];
        ProcessDescription desc{};
        std::size_t colonPos = proc.find( ':' );
        desc.user = proc.substr( 0, colonPos );
        desc.name = proc.substr( colonPos + 1, proc.size() );
        result[ i ] = desc;
    }
    return result;
}

constexpr const auto monitorProcesses = SplitToUserAndName( ConcatArrays( myProc, otherProc ) );

/// @brief Process info string
struct ProcessInfo
{
    uint32_t pid;           ///< process ID
    std::string name;       ///< process name
    std::string user;       ///< process owner (user)
    std::uint64_t cputime;  ///< process CPU time
    std::uint64_t memory;   ///< process RAM usage
    char status;            ///< process state
    
    ProcessInfo()
        : cputime( 0 )
        , memory( 0 )
        , status( 0 )
    {}
};

/// @brief Function for getting all processes matching `monitorProcesses` constant
/// @return `std::map<uint32_t, ProcessInfo>` containing information about running processes
std::map< uint32_t, ProcessInfo > GetAllMatchingProcesses();

#endif // PROCESS_INFO_H