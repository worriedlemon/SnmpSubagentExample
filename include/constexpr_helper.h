#pragma once

#ifndef CONSTEXPR_HELPER_H
#define CONSTEXPR_HELPER_H

#include <array>
#include <string_view>
#include <algorithm>

/// @brief Function for calculating words in string
/// @param str - space-separated word string
/// @return word count
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

/// @brief Function for transforming word string (space-separeted string) to array
/// @tparam N - size of array
/// @param str - input space-separted word string
/// @return resulting array
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

/// @brief Function for concatenating multiple arrays in compile time
/// @tparam ...N - sizes of arrays
/// @param ...arr - arrays
/// @return resulting concatenated array
template < std::size_t... N >
constexpr auto ConcatArrays( std::array< std::string_view, N >... arr ) -> std::array< std::string_view, (N + ...) >
{
    std::array< std::string_view, (N + ...) > newArr;
    std::size_t cur = 0;
    (( std::copy_n( arr.begin(), N, newArr.begin() + cur ), cur += N ), ... );
    return newArr;
}


/// @brief Macro for transforming word string to array in compile time
#define WORD_STRING_TO_ARRAY( str ) WordsToArray< WordCount( str ) >( str )

#endif // CONSTEXPR_HELPER_H