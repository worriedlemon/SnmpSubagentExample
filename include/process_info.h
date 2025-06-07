#pragma once

#ifndef PROCESS_INFO_H
#define PROCESS_INFO_H

#include <array>
#include <vector>
#include <string_view>
#include <algorithm>
#include <filesystem>
#include <optional>

inline constexpr std::size_t WordCount( std::string_view str )
{
    if ( str.empty() )
    {
        return 0;
    }

    std::size_t n = 1;
    for ( std::size_t i = 0; i < str.size(); ++i )
    {
        if ( str[ i ] == ' ' )
        {
            ++n;
        }
    }

    return n;
}

template < std::size_t N >
inline constexpr auto WordsToArray( std::string_view str ) -> std::array< std::string_view, N >
{
    std::array< std::string_view, N > words;
    
    int oldI = 0;
    int n = 0;
    for ( std::size_t i = 0; i < str.size(); ++i )
    {
        if ( str[ i ] == ' ' )
        {
            words[ n++ ] = str.substr( oldI, i );
            oldI = i + 1;
        }
    }

    words[ n ] = str.substr( oldI, str.size() );

    return words;
}

#define WORD_STRING_TO_ARRAY( str ) WordsToArray< WordCount( str ) >( str )

constexpr const std::array< std::string_view, 4 > myProc = {
    "emilshteinberg:cinnamon-session",
    "emilshteinberg:redshift",
    "root:upowerd",
    "kernoops:kerneloops"
};

#ifndef OTHER_CORE_PROCESSES
#define OTHER_CORE_PROCESSES ""
#endif

constexpr const auto otherProc = WORD_STRING_TO_ARRAY( OTHER_CORE_PROCESSES );

template < std::size_t... N >
constexpr auto ConcatArrays( std::array< std::string_view, N >... arr ) -> std::array< std::string_view, (N + ...) >
{
    std::array< std::string_view, (N + ...) > newArr;
    std::size_t cur = 0;
    (( std::copy_n( arr.begin(), N, newArr.begin() + cur ), cur += N ), ... );
    return newArr;
}

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

struct ProcessInfo
{
    std::string pid;
    std::string name;
    std::string user;
    std::uint64_t cputime;
    std::uint64_t memory;
    char status;
    
    ProcessInfo()
        : cputime( 0 )
        , memory( 0 )
        , status( 0 )
    {}
};

std::vector< ProcessInfo > GetAllMatchingProcesses();

#endif // PROCESS_INFO_H