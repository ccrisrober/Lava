#include <lava/lava.h>
#include <lavaUtils/lavaUtils.h>

struct ThreadPushData
{
  glm::mat4 model;
  glm::vec3 color;
};

struct PerThreadData
{
  std::shared_ptr<lava::CommandPool> commandPool;
  std::vector<lava::CommandBuffer> cmdBuffers;
  std::vector<ThreadPushData> pushData;

  glm::mat4 viewProjection;
};

lava::utility::ThreadPool pool;

std::shared_ptr<lava::CommandPool> primaryCmdPool;
std::shared_ptr<lava::CommandBuffer> primaryCmd;

int randInInterval( int min, int max )
{
  return ( min + ( rand( ) % ( int ) ( max - min + 1 ) ) );
}

uint32_t numworkers;

void updateCommandBuffers( void )
{
  primaryCmd = primaryCmdPool->allocateCommandBuffer( );
  primaryCmd->begin( );

  std::array<vk::ClearValue, 2 > clearValues;
  std::array<float, 4> ccv = { 1.0f, 0.0f, 1.0f, 1.0f };
  clearValues[ 0 ].color = vk::ClearColorValue( ccv );
  clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

  /*const vk::Offset2D size = _window->swapChainImageSize( );
  auto cmd = _window->currentCommandBuffer( );
  vk::Rect2D rect;
  rect.extent.width = size.x;
  rect.extent.height = size.y;
  cmd->beginRenderPass(
    _window->defaultRenderPass( ),
    _window->currentFramebuffer( ),
    rect, clearValues, vk::SubpassContents::eInline
  );*/

  std::vector<std::shared_ptr<lava::CommandBuffer>> secondaryBuffers;
  for ( uint32_t i = 0; i < numworkers; ++i )
  {
    for ( uint32_t j = 0; j < 5; ++j )
    {
      pool.workers[ i ]->addJob( [ = ] {
        std::this_thread::sleep_for( std::chrono::milliseconds(
          randInInterval( 500, 1500 ) ) );
        std::stringstream ss;
        ss << "HOLA DESDE (" << i << "," << j << ")";
        std::cerr << ss.str( ) << std::endl;
      } );
    }
  }
  primaryCmd->executeCommands( secondaryBuffers );

  primaryCmd->endRenderPass( );
  primaryCmd->end( );
}

int main( )
{
  numworkers = std::thread::hardware_concurrency( );
  std::cout << "Numworkers: " << numworkers << std::endl;
  pool.setThreadCount( numworkers );

  srand( time( NULL ) );

  while ( true )
  {
    updateCommandBuffers( );
    pool.wait( );

    std::cout << "RENDER DESDE MAIN" << std::endl;
  }
  return 0;
}