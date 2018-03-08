#ifndef __MATERIAL__
#define __MATERIAL__

#include <lava/lava.h>

namespace lava
{
  class Material: public std::enable_shared_from_this< Material >
  {
  public:
    //using Ptr = std::shared_ptr< Material >;
    virtual void configure( const std::string& dir, std::shared_ptr< Device > dev, 
      std::shared_ptr<RenderPass> renderPass ) = 0;

    /*static Material::Ptr create( void )
    {
      Material* m = new Material( );
      return Ptr( m );
    }*/

    virtual void bind( std::shared_ptr< CommandBuffer > cmd );
  //protected:
    Material( void );
    
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
    virtual void configure( const std::string& dir, std::shared_ptr< Device > dev,
      std::shared_ptr<RenderPass> renderPass );
  };

  class BasicTessTriangle: public Material
  {
  public:
    virtual void configure( const std::string& dir, std::shared_ptr< Device > dev,
      std::shared_ptr<RenderPass> renderPass );
  };
}

#endif /* __MATERIAL__ */