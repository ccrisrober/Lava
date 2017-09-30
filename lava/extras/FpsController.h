#ifndef __LAVA_FPSCONTROLLER__
#define __LAVA_FPSCONTROLLER__

#include <thread>
#include <chrono>

namespace lava
{
  namespace extras
  {
    class FpsController
    {
    public:
      FpsController( uint32_t fps ) 
        : _frameDurationInMilisecond(1000 / fps)
      {
      }

      void sleep( void )
      {
        static auto lastTime = std::chrono::high_resolution_clock::now( );
        auto currentTime = std::chrono::high_resolution_clock::now( );
        auto endTime = lastTime + _frameDurationInMilisecond;
        auto timeToWait = endTime - currentTime;

        if ( timeToWait.count( ) > 0 )
        {
          std::this_thread::sleep_for( timeToWait );
        }

        lastTime = std::chrono::high_resolution_clock::now( );
      }

    private:
      const std::chrono::milliseconds _frameDurationInMilisecond;
    };
  }
}

#endif /* __LAVA_FPSCONTROLLER__ */