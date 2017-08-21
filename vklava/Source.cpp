#include "VulkanRenderAPI.h"
using namespace lava;

int main( void )
{
  VulkanRenderAPI* vr = VulkanRenderAPI::getInstance( );
  vr->initialize( );
  vr->__init__( );
  vr->run( );
  vr->cleanup( );
  system( "PAUSE" );
  return 0;
}