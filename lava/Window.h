#ifndef __LAVA_WINDOW__
#define __LAVA_WINDOW__

#include "includes.hpp"

#include <lava/api.h>

namespace lava
{
  class Window
  {
  public:
    LAVA_API
    Window( const char* title, uint32_t width, uint32_t height );
    LAVA_API
    ~Window( void );

    LAVA_API
    GLFWwindow *getWindow( void );

    LAVA_API
    inline operator GLFWwindow*( void )
    {
      return _window;
    }

    LAVA_API
    void setErrorCallback( GLFWerrorfun fn );

    LAVA_API
    uint32_t getWidth( void ) const;
    LAVA_API
    uint32_t getHeight( void ) const;

    LAVA_API
    bool isRunning( void ) const;

    LAVA_API
    void close( void );
  protected:
    GLFWwindow* _window;
  };
}

#endif /* __LAVA_WINDOW__ */