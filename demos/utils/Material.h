/**
 * Copyright (c) 2017 - 2018, Pompeii
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef __MATERIAL__
#define __MATERIAL__

#include <pompeii/pompeii.h>

namespace pompeii
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