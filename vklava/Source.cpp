#include "VulkanRenderAPI.h"

int main( )
{
  VulkanRenderAPI vr;
  vr.initialize( );
  vr.run( );
  vr.cleanup( );
  system( "PAUSE" );
  return 0;
}