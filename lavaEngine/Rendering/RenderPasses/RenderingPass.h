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

#ifndef __LAVAENGINE_RENDERINGPASS__
#define __LAVAENGINE_RENDERINGPASS__

#include <lavaEngine/api.h>
#include <lavaEngine/Rendering/BatchQueue.h>

#include <lavaRenderer/lavaRenderer.h>

namespace lava
{
  namespace engine
  {
    class Renderer
    {
    public:
      virtual ~Renderer( void ) { if( w ) delete w; }
      virtual void setViewport( ) { }
      virtual void endRender( void ) { }
    protected:
      VulkanWindow* w;
    };
    class Application
    {
    public:
      LAVAENGINE_API
      Application( void )
      {
        _renderer = nullptr; // TODO
      }
      LAVAENGINE_API
      virtual ~Application( void ) { }
      LAVAENGINE_API
      void setSceneNode( Group* scene )
      {
        _scene = scene;
        _cameras.clear( );
        if ( _scene != nullptr )
        {

          _simulationClock.reset( );
        }
      }
      LAVAENGINE_API
      virtual int run( void )
      {
        bool fail = false;
        while ( !fail )
        {
          fail = !update( );
        }
        return 0;
      }
      LAVAENGINE_API
      virtual bool update( void )
      {
        /*//////////////////////////////////////////////
        //              UPDATE STEP                 //
        //////////////////////////////////////////////
        findVisibleCameras( );

        _simulationClock.tick( );
        std::vector< std::shared_ptr<BatchQueue> > bqCollection;
        std::vector< Light *> lights;
        FetchLights fl;
        _scene->perform( fl );
        lights = fl.lights( );

        for ( const auto& c : _cameras )
        {
          if ( c != nullptr && c->isEnabled( ) )
          {
            auto bq = std::make_shared<BatchQueue>( );
            ComputeBatchQueue cbq( c, bq );
            _scene->perform( cbq );
            bqCollection.push_back( bq );
          }
        }
        //////////////////////////////////////////////
        //              RENDER STEP                 //
        //////////////////////////////////////////////
        
        if ( !bqCollection.empty( ) )
        {
          std::shared_ptr<BatchQueue> mainQueue = nullptr;
          for ( const auto &bq : bqCollection )
          {
            if ( bq->getCamera( ) != Camera::getMainCamera( ) )
            {
              // _renderer->~Renderer( bq, bq->getCamera( )->renderPass( ) );
            }
            else
            {
              mainQueue = bq;
            }
          }
          if ( mainQueue != nullptr )
          {
            // _renderer->render( mainQueue, mainQueue->getCamera( )->renderPass( ) );
          }
        }
        _renderer->endRender( );*/

        return false;
      }
      LAVAENGINE_API
      Renderer* getRenderer( void)
      {
        return _renderer;
      }
    protected:
      Clock _simulationClock;
      Group* _scene;
      Renderer* _renderer;
      std::vector<Camera*> _cameras;
    private:
      void findVisibleCameras( void )
      {
        if ( !Camera::findCameras )
        {
          return;
        }
        _cameras.clear( );
        /*FetchCameras fc;
        _scene->perform( fc );
        fc.forEachCameras( [ & ]( Camera* c )
        {
          if ( Camera::getMainCamera( ) == nullptr || c->isMainCamera( ) )
          {
            Camera::setMainCamera( c );
          }
          _cameras.push_back( c );
        } );*/
        Camera::findCameras = false;
      }
    };
    class RenderingPass
    {
    public:
      LAVAENGINE_API
      virtual ~RenderingPass( void ) { }

      LAVAENGINE_API
      virtual void beginRender( Renderer* r, std::shared_ptr<BatchQueue>& bq ) = 0;
      LAVAENGINE_API
      virtual void render( Renderer* r, std::shared_ptr<BatchQueue>& bq, Camera* c ) = 0;

      //LAVAENGINE_API
      //virtual void applyImageEffects( Renderer* r, Camera* c );
    private:
      void swapBuffers( Renderer* r );
    };
  }
}

#endif /* __LAVAENGINE_RENDERINGPASS__ */