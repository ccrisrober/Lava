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

#include "Framebuffer.h"

#include <pompeii/RenderPass.h>
#include <pompeii/Image.h>

namespace pompeii
{
  Framebuffer::Framebuffer( const std::shared_ptr<Device>& device, 
    const std::shared_ptr<RenderPass>& renderPass,
    vk::ArrayProxy<const std::shared_ptr<ImageView>> attachments,
    const vk::Extent2D & extent, uint32_t layers )
    : VulkanResource( device )
    , _renderPass( renderPass )
    , _attachments( attachments.begin( ), attachments.end( ) )
  {
    std::vector<vk::ImageView> nativeAttachments;
    for ( auto const& att : _attachments )
    {
      nativeAttachments.push_back( *att );
    }

    vk::FramebufferCreateInfo fci;
    fci.setRenderPass( *_renderPass );
    fci.setAttachmentCount( nativeAttachments.size( ) );
    fci.setPAttachments( nativeAttachments.data( ) );
    fci.setWidth( extent.width );
    fci.setHeight( extent.height );
    fci.setLayers( layers );

    _framebuffer = static_cast< vk::Device >( *_device )
                                        .createFramebuffer( fci );
  }

  Framebuffer::~Framebuffer( void )
  {
    static_cast< vk::Device >( *_device )
                                        .destroyFramebuffer( _framebuffer );
  }
}