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

#include "Fence.h"

namespace lava
{
  Fence::Fence( const std::shared_ptr<Device>& device, bool signaled )
    : VulkanResource( device )
  {
    vk::FenceCreateInfo fenceCreateInfo( signaled ?
      vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlags( ) );

    _fence = static_cast< vk::Device >( *_device ).createFence( fenceCreateInfo );
  }

  Fence::~Fence( )
  {
    // From the spec:
    //    fence must not be associated with any queue command that has not yet completed execution on that queue
    static_cast< vk::Device >( *_device ).destroyFence( _fence );
  }

  bool Fence::isSignaled( ) const
  {
    vk::Result result = static_cast< vk::Device >( *_device ).getFenceStatus( _fence );
    assert( ( result == vk::Result::eSuccess ) || ( result == vk::Result::eNotReady ) );
    return( result == vk::Result::eSuccess );
  }

  void Fence::reset( )
  {
    static_cast< vk::Device >( *_device ).resetFences( _fence );
  }

  void Fence::wait( uint64_t timeout ) const
  {
    static_cast< vk::Device >( *_device ).waitForFences( _fence, true, timeout );
  }

  void Fence::resetFences(vk::ArrayProxy<const std::shared_ptr<Fence>> fences)
  {
    if ( !fences.empty( ) )
    {
      std::vector <vk::Fence> fencesArray;
      for ( const auto& fence : fences )
      {
        assert( fences.front( )->getDevice( ) == fence->getDevice( ) );
        fencesArray.push_back( *fence );
      }
      static_cast<vk::Device>( *fences.front( )->getDevice( ) )
        .resetFences( fencesArray );
    }
  }

  void Fence::waitForFences(vk::ArrayProxy<const std::shared_ptr<Fence>> fences, 
    bool all, uint32_t timeout)
  {
    if ( !fences.empty( ) )
    {
      std::vector< vk::Fence > fencesArray;
      for (const auto& fence : fences)
      {
        assert( fences.front( )->getDevice( ) == fence->getDevice( ) );
        fencesArray.push_back(*fence);
      }
      static_cast<vk::Device>( *fences.front( )->getDevice( ) )
        .waitForFences(fencesArray, all, timeout);
    }
  }
}