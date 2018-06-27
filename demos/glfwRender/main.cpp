#include <iostream>

#include <glfwLava/glfwLava.h>

int main( int argc, char** argv )
{
  lava::GLFWVulkanWindow* app = new lava::GLFWVulkanWindow( 500, 500, "GLFW", true );
  while ( true )
  {
    app->beginFrame( );
    app->update( );

    //Update( );

    if ( glfwWindowShouldClose( app->getWindow( ) ) )
      break;

    app->render( );
    //app->Render( renderer );
    app->endFrame( );
  }

  //Cleanup( );

  return EXIT_SUCCESS;
}