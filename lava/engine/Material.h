#ifndef __LAVA_ENGINE_MATERIAL__
#define __LAVA_ENGINE_MATERIAL__

#include <lava/api.h>

#include "../Device.h"
#include "../Pipeline.h"

namespace lava
{
  class CommandBuffer;
  namespace engine
  {
    class Material: public std::enable_shared_from_this< Material >
    {
    public:
      //using Ptr = std::shared_ptr< Material >;
      LAVA_API
      virtual void configure( const std::string& dir, std::shared_ptr< Device > dev, 
        std::shared_ptr<RenderPass> renderPass ) = 0;

      /*static Material::Ptr create( void )
      {
        Material* m = new Material( );
        return Ptr( m );
      }*/

      LAVA_API
      virtual void bind( std::shared_ptr< CommandBuffer > cmd );
    //protected:
      LAVA_API
      Material( void );
      
      LAVA_API
      virtual ~Material( void ) 
      {
      }

      const std::shared_ptr<PipelineLayout>& pipelineLayout( void )
      {
        return _pipelineLayout;
      }

    protected:
      std::shared_ptr<Pipeline> _pipeline;
      std::shared_ptr<PipelineLayout> _pipelineLayout;
    };

    class BasicTriangle: public Material
    {
    public:
      LAVA_API
      virtual void configure( const std::string& dir, std::shared_ptr< Device > dev,
        std::shared_ptr<RenderPass> renderPass );
    };

    class BasicTessTriangle: public Material
    {
    public:
      LAVA_API
      virtual void configure( const std::string& dir, std::shared_ptr< Device > dev,
        std::shared_ptr<RenderPass> renderPass );
    };
  }
}

#endif /* __LAVA_ENGINE_MATERIAL__ */