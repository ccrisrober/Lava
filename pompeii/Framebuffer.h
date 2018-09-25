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

#ifndef __POMPEII_FRAMEBUFFER__
#define __POMPEII_FRAMEBUFFER__

#include <pompeii/api.h>

#include "includes.hpp"

#include "VulkanResource.h"
#include "noncopyable.hpp"

namespace pompeii
{
  class Device;
  class RenderPass;
  class ImageView;
  class Framebuffer
    : public VulkanResource
    , private NonCopyable<Framebuffer>
  {
  public:
    POMPEII_API
    Framebuffer( const std::shared_ptr<Device>& device, 
      const std::shared_ptr<RenderPass>& renderPass,
      vk::ArrayProxy<const std::shared_ptr<ImageView>> attachments,
      const vk::Extent2D& extent, uint32_t layers );
    POMPEII_API
    virtual ~Framebuffer( void );

    inline operator vk::Framebuffer( void ) const
    {
      return _framebuffer;
    }

  private:
    std::shared_ptr<RenderPass> _renderPass;
    vk::Framebuffer _framebuffer;
    std::vector<std::shared_ptr<ImageView>> _attachments;
  };
}

#endif /* __POMPEII_FRAMEBUFFER__ */