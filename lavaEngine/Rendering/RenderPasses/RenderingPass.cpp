#include "RenderingPass.h"

namespace lava
{
  namespace engine
  {
    void RenderingPass::swapBuffers( Renderer* r )
    {
      //auto source = 1;
      //auto destination = 2;

      std::cout << "Set source buffer with destination texture" << std::endl;
      //renderer->setFBO(SOURCE, destination);
      std::cout << "Set destination buffer with source texture" << std::endl;
      //renderer->setFBO(DESTINATION, source);
    }
  }
}