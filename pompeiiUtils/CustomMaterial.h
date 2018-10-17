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

#ifndef __POMPEIIUTILS_CUSTOMMATERIAL__
#define __POMPEIIUTILS_CUSTOMMATERIAL__

#include <pompeiiUtils/api.h>

#include <memory>
#include <pompeii/pompeii.h>

namespace pompeii
{
  namespace utils
  {
    class CustomMaterial: public pompeii::VulkanResource
    {
    public:
      POMPEIIUTILS_API
      virtual ~CustomMaterial( void ) { }
      POMPEIIUTILS_API
      virtual void configure( const std::string& sourceDir,
        std::shared_ptr< Device > device, std::shared_ptr< RenderPass > renderPass,
        std::shared_ptr< PipelineCache > pipCache = nullptr ) = 0;

      POMPEIIUTILS_API
      virtual void bind( std::shared_ptr< CommandBuffer > cmd );

      POMPEIIUTILS_API
      const std::shared_ptr< PipelineLayout >& pipelineLayout( void ) const
      {
        return _pipelineLayout;
      }
      POMPEIIUTILS_API
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

#endif /* __POMPEIIUTILS_CUSTOMMATERIAL__  */
