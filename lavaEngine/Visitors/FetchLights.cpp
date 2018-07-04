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

#include "FetchLights.h"

namespace lava
{
  namespace engine
  {
    void FetchLights::reset( void )
    {
      _lights.clear( );
      Visitor::reset( );
    }

    void FetchLights::visitLight( Light *c )
    {
      _lights.push_back( c );
    }

    void FetchLights::forEachLight( std::function< void( Light * ) > cb )
    {
      for ( auto& light : _lights )
      {
        cb( light );
      }
      _lights.clear( );
    }
  }
}
