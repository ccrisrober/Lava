/**
 * Copyright (c) 2017 - 2018, Lava
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

#ifndef __LAVAUTILS_CUSTOMMATERIAL__
#define __LAVAUTILS_CUSTOMMATERIAL__

#include <lavaUtils/api.h>

#include <memory>
#include <lava/lava.h>

namespace lava
{
  namespace utility
  {
    class CustomMaterial: public lava::VulkanResource
    {
    public:
      LAVAUTILS_API
      virtual ~CustomMaterial( void ) { }
      LAVAUTILS_API
      virtual void configure( const std::string& sourceDir,
        std::shared_ptr< Device > device, std::shared_ptr< RenderPass > renderPass,
        std::shared_ptr< PipelineCache > pipCache = nullptr ) = 0;

      LAVAUTILS_API
      virtual void bind( std::shared_ptr< CommandBuffer > cmd );

      LAVAUTILS_API
      const std::shared_ptr< PipelineLayout >& pipelineLayout( void ) const
      {
        return _pipelineLayout;
      }
      LAVAUTILS_API
      const std::shared_ptr< Pipeline >& pipeline( void ) const
      {
        return _pipeline;
      }

    protected:
      std::shared_ptr< Pipeline > _pipeline;
      std::shared_ptr< PipelineLayout > _pipelineLayout;
    };
  }
}

#endif /* __LAVAUTILS_CUSTOMMATERIAL__  */
