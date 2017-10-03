#include "Window.h"

namespace lava
{
  Window::Window( const char* title, uint32_t width, uint32_t height )
  {
    glfwInit( );
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    _window = glfwCreateWindow( width, height, title, nullptr, nullptr );

    glfwSetInputMode( _window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
  }

  GLFWwindow * Window::getWindow( void )
  {
    return _window;
  }

  void Window::setErrorCallback( GLFWerrorfun fn )
  {
    glfwSetErrorCallback( fn );
  }

  uint32_t Window::getWidth( void ) const
  {
    int width, height;
    glfwGetFramebufferSize( _window, &width, &height );
    return width;
  }

  uint32_t Window::getHeight( void ) const
  {
    int width, height;
    glfwGetFramebufferSize( _window, &width, &height );
    return height;
  }

  bool Window::isRunning( void ) const
  {
    glfwPollEvents( );
    return !glfwWindowShouldClose( _window );
  }

  Window::~Window( void )
  {
    glfwDestroyWindow( _window );
    glfwTerminate( );
  }
}