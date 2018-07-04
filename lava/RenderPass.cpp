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

#include "RenderPass.h"
#include "Device.h"

namespace lava
{
  RenderPass::RenderPass( const std::shared_ptr<Device>& device, 
    vk::ArrayProxy<const vk::AttachmentDescription> attachments, 
    vk::ArrayProxy<const vk::SubpassDescription> subpasses,
    vk::ArrayProxy<const vk::SubpassDependency> dependencies )
    : VulkanResource( device )
  {
    vk::RenderPassCreateInfo rpci;
    rpci.setPAttachments( attachments.data( ) );
    rpci.setAttachmentCount( attachments.size( ) );
    rpci.setPDependencies( dependencies.data( ) );
    rpci.setDependencyCount( dependencies.size( ) );
    rpci.setPSubpasses( subpasses.data( ) );
    rpci.setSubpassCount( subpasses.size( ) );
    _renderPass = static_cast< vk::Device >( *_device ).createRenderPass( rpci );
  }

  RenderPass::~RenderPass( )
  {
    static_cast< vk::Device >( *_device ).destroyRenderPass( _renderPass );
  }
}
