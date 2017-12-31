#ifndef __LAVA_WINDOW__
#define __LAVA_WINDOW__

#include "includes.hpp"

#include <lava/api.h>
#include <functional>

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
    void setWindowTitle( const std::string& title );

    LAVA_API
    GLFWwindow *getWindow( void );

    LAVA_API
    void pollEvents( void );

    LAVA_API
    inline operator GLFWwindow*( void )
    {
      return _window;
    }

    LAVA_API
    void setChangeSizeCallback( std::function<void( int, int )> cb );

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
  public:
    std::function<void( int, int )> _callbackResize = nullptr;
  };
}

#endif /* __LAVA_WINDOW__ */