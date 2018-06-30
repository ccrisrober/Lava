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

#include "BatchQueue.h"

namespace lava
{
	namespace engine
	{
    BatchQueue::BatchQueue( void )
    {
      _renderables[ RenderableType::OPAQUE ] = std::vector<Renderable>( );
      _renderables[ RenderableType::TRANSPARENT ] = std::vector<Renderable>( );
    }
    BatchQueue::~BatchQueue( void )
    {
      reset( );
    }
    std::vector<Renderable> BatchQueue::renderables( RenderableType t )
    {
      return _renderables[ t ];
    }
    void BatchQueue::pushGeometry( Geometry* geom )
    {
      auto renderType = RenderableType::OPAQUE;
      /*if( state.getBlending( ).isEnabled( ) )
      {
        renderType = RenderableType::TRANSPARENT;
      }*/
      Renderable renderable( geom,
        geom->getTransform( ), 
        geom->getAbsolutePosition( ).z
      );
      auto queue = &_renderables[ renderType ];
      if ( renderType == RenderableType::TRANSPARENT )
      {
        // Order back to front
        auto it = queue->begin( );
        while ( it != queue->end( ) && it->zDistance >= renderable.zDistance )
        {
          ++it;
        }
        queue->insert( it, renderable );
      }
      else
      {
        // Order by material (TODO: FAIL!)
        auto it = queue->begin( );
        /*auto mat1 = materials->first( );
        while ( it != queue->end( ) )
        {
          auto mat2 = renderable.geometry->getComponent< mb::MaterialComponent >( )->first( );
          if ( mat1 != mat2 )
          {
            ++it;
          }
          else { break; }
        }
        queue->insert( it, renderable );
        // TODO: Required order?*/
        queue->insert( it, renderable );
      }

      /*if ( geom->castShadows( ) )
      {
        _renderables[ RenderableType::SHADOW ].push_back( renderable );
      }*/
    }
    void BatchQueue::pushLight( Light* l )
    {
      _lights.push_back( l );
    }
    void BatchQueue::reset( void )
    {
      setCamera( nullptr );
      _lights.clear( );
      for ( auto &it : _renderables )
      {
        it.second.clear( );
      }
      _renderables.clear( );
    }
    void BatchQueue::setCamera( Camera* c )
    {
      if ( c != nullptr )
      {
        _camera = c;
        _projMatrix = _camera->getProjection( );
        _viewMatrix = _camera->getView( );
      }
      else
      {
        _camera = nullptr;
        _projMatrix = glm::mat4( 1.0f );
        _viewMatrix = glm::mat4( 1.0f );
      }
    }
    Camera* BatchQueue::getCamera( void )
    {
      return _camera;
    }
  }
}