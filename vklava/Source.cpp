#include "VulkanRenderAPI.h"
using namespace lava;

int main( void )
{
  VulkanRenderAPI vr;
  vr.initialize( );
  vr.run( );
  vr.cleanup( );
  system( "PAUSE" );
  return 0;
}