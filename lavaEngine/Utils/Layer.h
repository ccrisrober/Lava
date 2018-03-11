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

#ifndef __LAVAENGINE_LAYER__
#define __LAVAENGINE_LAYER__

#include <lavaEngine/api.h>

#include <array>

namespace lava
{
  namespace engine
  {
    class LayerLookup
    {
    public:
      LAVAENGINE_API
      static void loadFromFile( const std::string& file );
      LAVAENGINE_API
      static const std::string getName( const int channel );
      LAVAENGINE_API
      static void setName( const std::string& name, const int index );
      LAVAENGINE_API
      static int nameToID( const std::string& layer );
    protected:
      static std::array<std::string, 32> _names;
      static bool _initialized;

      static void initialize( );
    };
    class Layer
    {
    public:
      LAVAENGINE_API
      Layer( void );
      LAVAENGINE_API
      void set( const int channel );
      LAVAENGINE_API
      void set( const std::string& layer );
      LAVAENGINE_API
      void enable( const int channel );
      LAVAENGINE_API
      void enable( const std::string& layer );
      LAVAENGINE_API
      void toggle( const int channel );
      LAVAENGINE_API
      void toggle( const std::string& layer );
      LAVAENGINE_API
      void disable( const int channel );
      LAVAENGINE_API
      void disable( const std::string& layer );
      LAVAENGINE_API
      bool check( const Layer& layer2 ) const;
      LAVAENGINE_API
      bool check( const int channel ) const;
      LAVAENGINE_API
      bool check( const std::string& layer ) const;
    protected:
      int _mask;
    private:
      static int layerNameToID( const std::string& layer );
    };
  }
}

#endif /* __LAVAENGINE_LAYER__ */
