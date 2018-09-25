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

#ifndef __POMPEII_RENDERPASS__
#define __POMPEII_RENDERPASS__

#include "includes.hpp"
#include "VulkanResource.h"

namespace pompeii
{
  class RenderPass : public VulkanResource
  {
  public:
    POMPEII_API
    RenderPass( const std::shared_ptr<Device>& device,
      vk::ArrayProxy<const vk::AttachmentDescription> attachments, 
      vk::ArrayProxy<const vk::SubpassDescription> subpasses,
      vk::ArrayProxy<const vk::SubpassDependency> dependencies );
    POMPEII_API
    ~RenderPass( void );

    POMPEII_API
    inline operator vk::RenderPass( void ) const
    {
      return _renderPass;
    }

  protected:
    vk::RenderPass _renderPass;
  };
}

#endif /* __POMPEII_RENDERPASS__ */