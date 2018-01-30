/**
 * Copyright (c) 2017, Lava
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

#include <lava/lava.h>
#include <lavaRenderer/lavaRenderer.h>
using namespace lava;

#include <routes.h>

#define TESS_MODE // Comment for not indexing mode
#define INDEXING_MODE // Comment for not indexing mode

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    std::string title = "Triangle";
#ifdef INDEXING_MODE
    title += std::string( " indexed" );
#endif // INDEXING_MODE
#ifdef TESS_MODE
    title += std::string( " tesselated" );
#endif // TESS_MODE
    _window->setWindowTitle( title );
  }

  struct Vertex
  {
    glm::vec4 position;
    glm::vec4 color;
  };

  const std::vector<Vertex> vertices =
  {
    { {  0.75f,  0.75f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f } },
    { { -0.75f,  0.75f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f } },
    { {   0.0f, -0.75f, 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f } }
  };
#ifdef INDEXING_MODE
  const std::vector<uint32_t> indices = { 0, 1, 2 };
#endif

  void initResources( void ) override
  {
    auto device = _window->device( );
    auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
    cmd->begin( );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );

      vertexBuffer = device->createBuffer( vertexBufferSize, 
        vk::BufferUsageFlagBits::eVertexBuffer | 
        vk::BufferUsageFlagBits::eTransferDst, 
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ), 
        vertices.data( ) } );
    }

#ifdef INDEXING_MODE
    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );

      indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      indexBuffer->update<uint32_t>( cmd, 0, { uint32_t( indices.size( ) ),
        indices.data( ) } );
    }
    cmd->end( );
    _window->gfxQueue( )->submitAndWait( cmd );
#endif
#ifdef TESS_MODE
    material = std::make_shared<lava::engine::BasicTessTriangle>( );
#else
    material = std::make_shared<lava::engine::BasicTriangle>( );
#endif
    material->configure( LAVA_EXAMPLES_SPV_ROUTE, device, _window->defaultRenderPass( ) );
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }
    
    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    material->bind( cmd );
    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
#ifdef INDEXING_MODE
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint32 );
#endif

    cmd->setViewportScissors( _window->getExtent( ) );
#ifdef INDEXING_MODE
    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );
#else
    cmd->draw( uint32_t( vertices.size( ) ), 1, 0, 0 );
#endif

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  VulkanWindow *_window;
  std::shared_ptr<lava::engine::Material> material;
  std::shared_ptr<Buffer> vertexBuffer;
#ifdef INDEXING_MODE
  std::shared_ptr<Buffer> indexBuffer;
#endif
};

class CustomVkWindow : public VulkanWindow
{
public:
  VulkanWindowRenderer* createRenderer( void ) override
  {
    return new CustomRenderer( this );
  }
};

int main( void )
{
  std::shared_ptr<Instance> instance;

  // Create instance
  vk::ApplicationInfo appInfo(
    "App Name",
    VK_MAKE_VERSION( 1, 0, 0 ),
    "FooEngine",
    VK_MAKE_VERSION( 1, 0, 0 ),
    VK_API_VERSION_1_0
  );


  std::vector<const char*> layers =
  {
#ifndef NDEBUG
    "VK_LAYER_LUNARG_standard_validation",
#endif
  };
  std::vector<const char*> extensions =
  {
    VK_KHR_SURFACE_EXTENSION_NAME,  // Surface extension
    LAVA_KHR_EXT, // OS specific surface extension
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
  };


  instance = Instance::create( vk::InstanceCreateInfo(
    { },
    &appInfo,
    layers.size( ),
    layers.data( ),
    extensions.size( ),
    extensions.data( )
  ) );

  CustomVkWindow w;
  w.setVulkanInstance( instance );
  w.resize( 500, 500 );

  w.show( );

  return 0;
}