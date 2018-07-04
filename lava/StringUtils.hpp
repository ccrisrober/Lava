/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef __LAVA_STRING_UTILS__
#define __LAVA_STRING_UTILS__

#include <lava/api.h>

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>

namespace lava
{
  class StringUtils
  {
  public:
    static std::string replace( std::string str, std::string& from, std::string& to )
    {
      size_t start_pos = str.find( from );
      if ( start_pos == std::string::npos )
        return str;
      str.replace( start_pos, from.length( ), to );
      return str;
    }
    static std::vector<std::string> split_str( const std::string& str, const std::string& delim )
    {
      std::vector<std::string> tokens;
      size_t prev = 0, pos = 0;
      do
      {
        pos = str.find( delim, prev );
        if ( pos == std::string::npos ) pos = str.length( );
        std::string token = str.substr( prev, pos - prev );
        if ( !token.empty( ) ) tokens.push_back( token );
        prev = pos + delim.length( );
      } while ( pos < str.length( ) && prev < str.length( ) );
      return tokens;
    }

    template< typename E >
    static void toValue( std::string input, E& out )
    {
      std::stringstream ss;
      ss << input;
      ss >> out;
    }

    template< typename E>
    static std::vector< E > split( std::string entryStr, char delim )
    {
      std::vector< E > res;
      std::istringstream iss( entryStr );
      while( !iss.eof( ) )
      {
        std::string str;
        getline( iss, str, delim );
        E v;
        toValue< E >( str, v);
        res.push_back( v );
      }
      return res;
    }

    LAVA_API
    static std::string replaceAll( std::string str, std::string from, std::string to )
    {
      if ( from.empty( ) )
      {
        return str;
      }

      size_t start_pos = 0;
      while ( ( start_pos = str.find( from, start_pos ) ) != std::string::npos )
      {
        str.replace( start_pos, from.length(), to );
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
      }

      return str;
    }

    static std::string getFileExtension( std::string path )
    {
      auto pos = path.find_last_of( "." );
      if ( pos == std::string::npos )
      {
        return "";
      }

      return path.substr( pos );
    }

    static std::string splitLines( std::string input, int charsPerLine )
    {
      std::stringstream out;

      std::stringstream ss( input );
      std::string buffer;
      std::vector< std::string > lines;
      while ( std::getline( ss, buffer, '\n' ) )
      {
        lines.push_back( buffer );
      }

      for ( auto line : lines )
      {
        std::stringstream str;
        str << line;

        int charCount = 0;
        while ( !str.eof( ) )
        {
          std::string temp;
          str >> temp;
          charCount += temp.length() + 1;
          if ( charCount >= charsPerLine )
          {
            out << "\n";
            charCount = 0;
          }

          out << temp << " ";
        }
        out << "\n";
      }

      return out.str();
    }

    static std::string toUpper( const std::string &str )
    {
      std::string result( str );
      std::transform( result.begin( ), result.end( ), result.begin( ), ::toupper );
      return result;
    }

    static std::string toLower( const std::string &str )
    {
      std::string result( str );
      std::transform( result.begin( ), result.end( ), result.begin( ), ::tolower );
      return result;
    }

    template< typename ... Args >
    static std::string toString( Args &&... args )
    {
      std::stringstream ss;
      ( void ) std::initializer_list< int >
      {
        (
        ss << args,
          0
      )...
      };

      return ss.str();
    }
  };
}

#endif /* __LAVA_STRING_UTILS__ */
