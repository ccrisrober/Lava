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

#include "Queue.h"

#include <pompeii/CommandBuffer.h>
#include <pompeii/Swapchain.h>

#define DEFAULT_FENCE_TIMEOUT 100000000000

namespace pompeii
{
  SubmitInfo::SubmitInfo(
    vk::ArrayProxy<const std::shared_ptr<Semaphore>> const& waitSemaphores_,
    vk::ArrayProxy<const vk::PipelineStageFlags> waitDstStageMasks_,
    vk::ArrayProxy<const std::shared_ptr<CommandBuffer>> const& commandBuffers_,
    vk::ArrayProxy<const std::shared_ptr<Semaphore>> signalSemaphores_ )
    : waitSemaphores( waitSemaphores_.begin( ), waitSemaphores_.end( ) )
    , waitDstStageMasks( waitDstStageMasks_.begin( ), waitDstStageMasks_.end( ) )
    , commandBuffers( commandBuffers_.begin( ), commandBuffers_.end( ) )
    , signalSemaphores( signalSemaphores_.begin( ), signalSemaphores_.end( ) )
  {
    assert( waitSemaphores.size( ) == waitDstStageMasks.size( ) );
  }

  SubmitInfo::SubmitInfo( SubmitInfo const& rhs )
    : SubmitInfo( rhs.waitSemaphores, rhs.waitDstStageMasks,
      rhs.commandBuffers, rhs.signalSemaphores )
  {}

  SubmitInfo& SubmitInfo::operator=( SubmitInfo const& rhs )
  {
    waitSemaphores = rhs.waitSemaphores;
    waitDstStageMasks = rhs.waitDstStageMasks;
    commandBuffers = rhs.commandBuffers;
    signalSemaphores = rhs.signalSemaphores;
    return *this;
  }

  vk::Result Queue::submit( vk::ArrayProxy<const SubmitInfo> submitInfos,
    const std::shared_ptr<Fence>& fenceIn )
  {
    // create a new fence if none has been passed to track completion of the submit.
    auto fence = fenceIn ? fenceIn : _device->createFence( false );

    _submitInfos.insert( std::make_pair( fence,
      std::vector<SubmitInfo>( submitInfos.begin( ), submitInfos.end( ) ) ) );

    std::vector<std::vector<vk::Semaphore>> waitSemaphores;
    waitSemaphores.reserve( submitInfos.size( ) );

    std::vector<std::vector<vk::CommandBuffer>> commandBuffers;
    commandBuffers.reserve( submitInfos.size( ) );

    std::vector<std::vector<vk::Semaphore>> signalSemaphores;
    signalSemaphores.reserve( submitInfos.size( ) );

    std::vector<vk::SubmitInfo> to_submit;
    to_submit.reserve( submitInfos.size( ) );
    for ( auto const& si : submitInfos )
    {
      assert( si.waitSemaphores.size( ) == si.waitDstStageMasks.size( ) );

      waitSemaphores.push_back( std::vector<vk::Semaphore>( ) );
      waitSemaphores.back( ).reserve( si.waitSemaphores.size( ) );
      for ( auto const& sem : si.waitSemaphores )
      {
        waitSemaphores.back( ).push_back( sem ?
          static_cast< vk::Semaphore >( *sem ) : nullptr );
      }

      commandBuffers.push_back( std::vector<vk::CommandBuffer>( ) );
      commandBuffers.back( ).reserve( si.commandBuffers.size( ) );
      for ( auto const& cmd : si.commandBuffers )
      {
        commandBuffers.back( ).push_back( cmd ?
          static_cast< vk::CommandBuffer >( *cmd ) : nullptr );
      }

      signalSemaphores.push_back( std::vector<vk::Semaphore>( ) );
      signalSemaphores.back( ).reserve( si.signalSemaphores.size( ) );
      for ( auto const& sem : si.signalSemaphores )
      {
        signalSemaphores.back( ).push_back( sem ?
          static_cast< vk::Semaphore >( *sem ) : nullptr );
      }

      to_submit.push_back(
        vk::SubmitInfo(
          waitSemaphores.back( ).size( ),
          waitSemaphores.back( ).data( ),
          si.waitDstStageMasks.data( ),
          commandBuffers.back( ).size( ),
          commandBuffers.back( ).data( ),
          signalSemaphores.back( ).size( ),
          signalSemaphores.back( ).data( )
        )
      );
    }

    //_queue.submit( to_submit, *fence );

    return _queue.submit( to_submit.size( ), to_submit.data( ), *fence );
  }

  void Queue::submit( const std::shared_ptr<CommandBuffer>& commandBuffer,
    const std::shared_ptr<Fence>& fence )
  {
    submit( SubmitInfo( nullptr, nullptr, commandBuffer, nullptr ), fence );
  }


  std::vector<vk::Result> Queue::presentToDisplay(
    vk::ArrayProxy<const std::shared_ptr<Semaphore>> waitSemaphores,
    vk::ArrayProxy<const std::shared_ptr<Swapchain>> swapchains,
    vk::ArrayProxy<const uint32_t> imageIndices,
    const vk::Rect2D& srcRect, const vk::Rect2D& dstRect,
    bool persistent )
  {
    vk::DisplayPresentInfoKHR displayPresentInfo;
    displayPresentInfo.srcRect = srcRect;
    displayPresentInfo.dstRect = dstRect;
    displayPresentInfo.persistent = persistent ? VK_TRUE : VK_FALSE;
    return present( waitSemaphores, swapchains, imageIndices, &displayPresentInfo );
  }

  std::vector<vk::Result> Queue::present(
    vk::ArrayProxy<const std::shared_ptr<Semaphore>> waitSemaphores,
    vk::ArrayProxy<const std::shared_ptr<Swapchain>> swapchains,
    vk::ArrayProxy<const uint32_t> imageIndices,
    const vk::DisplayPresentInfoKHR* displayPresentInfo )
  {
    assert( swapchains.size( ) == imageIndices.size( ) );

    std::vector<vk::Semaphore> waitSemaphoreData;
    waitSemaphoreData.reserve( waitSemaphores.size( ) );
    for ( auto const& s : waitSemaphores )
    {
      waitSemaphoreData.push_back( *s );
    }

    std::vector<vk::SwapchainKHR> swapchainData;
    swapchainData.reserve( swapchains.size( ) );
    for ( auto const& s : swapchains )
    {
      swapchainData.push_back( static_cast< vk::SwapchainKHR >( *s ) );
    }

    std::vector<vk::Result> results( swapchains.size( ) );
    vk::PresentInfoKHR info(
      waitSemaphoreData.size( ), waitSemaphoreData.data( ),
      swapchainData.size( ), swapchainData.data( ),
      imageIndices.data( ), results.data( ) );
    info.pNext = displayPresentInfo;
    _queue.presentKHR( info );
    return results;
  }

  void Queue::waitIdle( void )
  {
    _queue.waitIdle( );
  }

  void Queue::submitAndWait( std::shared_ptr<CommandBuffer>& cmd )
  {
    auto fence = _device->createFence( false );
    this->submit( cmd, fence );

    std::vector<vk::Fence> vkFences;
    vkFences.push_back( *fence );
    /*vkFences.reserve( fences.size( ) );
    for ( auto const& f : fences )
    {
    vkFences.push_back( *f );
    }*/

    // Wait for the fence to signal that command buffer has finished executing
    static_cast< vk::Device >( *_device ).waitForFences( vkFences, VK_TRUE,
      DEFAULT_FENCE_TIMEOUT );
  }

  Queue::Queue( const std::shared_ptr<Device>& device, vk::Queue queue, 
    uint32_t queueFamilyIndex )
    : VulkanResource( device )
    , _queue( queue )
    , _queueFamilyIndex( queueFamilyIndex )
  {
  }
}
