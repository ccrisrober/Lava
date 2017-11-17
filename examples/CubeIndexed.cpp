#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#include "materials/RenderNormalMaterial.h"

const float side = 1.0f;
const float side2 = side / 2.0f;
const std::vector<material::RenderNormalMaterial::Vertex> vertices =
{
  { { -side2, -side2, side2 },{ 0.0f, 0.0f, 1.0f } },
  { { side2, -side2, side2 },{ 0.0f, 0.0f, 1.0f } },
  { { side2, side2, side2 },{ 0.0f, 0.0f, 1.0f } },
  { { -side2, side2, side2 },{ 0.0f, 0.0f, 1.0f } },


  { { side2, -side2, side2 },{ 1.0f, 0.0f, 0.0f } },
  { { side2, -side2, -side2 },{ 1.0f, 0.0f, 0.0f } },
  { { side2, side2, -side2 },{ 1.0f, 0.0f, 0.0f } },
  { { side2, side2, side2 },{ 1.0f, 0.0f, 0.0f } },


  { { -side2, -side2, -side2 },{ 0.0f, 0.0f, -1.0f } },
  { { -side2, side2, -side2 },{ 0.0f, 0.0f, -1.0f } },
  { { side2, side2, -side2 },{ 0.0f, 0.0f, -1.0f } },
  { { side2, -side2, -side2 },{ 0.0f, 0.0f, -1.0f } },


  { { -side2, -side2, side2 },{ -1.0f, 0.0f, 0.0f } },
  { { -side2, side2, side2 },{ -1.0f, 0.0f, 0.0f } },
  { { -side2, side2, -side2 },{ -1.0f, 0.0f, 0.0f } },
  { { -side2, -side2, -side2 },{ -1.0f, 0.0f, 0.0f } },


  { { -side2, -side2, side2 },{ 0.0f, -1.0f, 0.0f } },
  { { -side2, -side2, -side2 },{ 0.0f, -1.0f, 0.0f } },
  { { side2, -side2, -side2 },{ 0.0f, -1.0f, 0.0f } },
  { { side2, -side2, side2 },{ 0.0f, -1.0f, 0.0f } },


  { { -side2, side2, side2 },{ 0.0f, 1.0f, 0.0f } },
  { { side2, side2, side2 },{ 0.0f, 1.0f, 0.0f } },
  { { side2, side2, -side2 },{ 0.0f, 1.0f, 0.0f } },
  { { -side2, side2, -side2 },{ 0.0f, 1.0f, 0.0f } },
};
const std::vector<uint16_t> indices =
{
  0, 1, 2, 0, 2, 3,
  4, 5, 6, 4, 6, 7,
  8, 9, 10, 8, 10, 11,
  12, 13, 14, 12, 14, 15,
  16, 17, 18, 16, 18, 19,
  20, 21, 22, 20, 22, 23
};

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<VertexBuffer> vertexBuffer;
  std::shared_ptr<IndexBuffer> indexBuffer;

  std::shared_ptr<material::RenderNormalMaterial> material;
  std::shared_ptr<CommandPool> commandPool;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( 
        material::RenderNormalMaterial::Vertex );
      vertexBuffer = std::make_shared<VertexBuffer>( _device, vertexBufferSize );
      vertexBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );
      indexBuffer = std::make_shared<IndexBuffer>( _device, 
        vk::IndexType::eUint16, indices.size( ) );
      indexBuffer->writeData( 0, indexBufferSize, indices.data( ) );
    }

    material = std::make_shared<material::RenderNormalMaterial>( );
    material->configure( LAVA_EXAMPLES_SPV_ROUTE, _device, _renderPass );

    // create a command pool for command buffer allocation
    commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );

  }
  std::shared_ptr<CommandBuffer> commandBuffer;
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    material->uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    material->uboVS.model = glm::rotate( material->uboVS.model, time * glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    
    material->uboVS.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    material->uboVS.proj = glm::perspective(glm::radians(45.0f), width / (float) height, 0.1f, 10.0f);
    material->uboVS.proj[1][1] *= -1;

    material->uniformBufferMVP->writeData(0, sizeof( material->uboVS), &material->uboVS );
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );

    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->beginSimple( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), 
      vk::Rect2D( { 0, 0 }, 
      _defaultFramebuffer->getExtent( ) ),
      { vk::ClearValue( ccv ), vk::ClearValue( 
        vk::ClearDepthStencilValue( 1.0f, 0 ) )
      }, vk::SubpassContents::eInline );
    material->bind( commandBuffer );
    vertexBuffer->bind( commandBuffer );
    indexBuffer->bind( commandBuffer );
    
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    
    commandBuffer->drawIndexed( indices.size( ), 1, 0, 0, 1 );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_ESCAPE:
      switch ( action )
      {
      case GLFW_PRESS:
        getWindow( )->close( );
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
};

void glfwErrorCallback( int error, const char* description )
{
  fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

int main( void )
{
  try
  {
    VulkanApp* app = new MyApp( "Cube Indexed", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      //app->waitEvents( );
      app->paint( );
    }

    delete app;
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  return 0;
}