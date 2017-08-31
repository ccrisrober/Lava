#include "Queue.h"

#include "Device.h"
#include "Swapchain.h"

namespace lava
{
  SubmitInfo::SubmitInfo( vk::ArrayProxy<const std::shared_ptr<Semaphore>> const& waitSemaphores_,
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

  SubmitInfo & SubmitInfo::operator=( SubmitInfo const& rhs )
  {
    waitSemaphores = rhs.waitSemaphores;
    waitDstStageMasks = rhs.waitDstStageMasks;
    commandBuffers = rhs.commandBuffers;
    signalSemaphores = rhs.signalSemaphores;
    return *this;
  }




  Fence::Fence( const DeviceRef& device, bool signaled )
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


  /*void resetFences(vk::ArrayProxy<const std::shared_ptr<Fence>> fences)
  {
    if (!fences.empty())
    {
      std::vector <vk::Fence> fencesArray;
      for (const std::shared_ptr<Fence>& fence : fences)
      {
        assert(fences.front()->_device == fence->_device);
        fencesArray.push_back(*fence);
      }
      static_cast<vk::Device>(*fences.front()->_device).resetFences(fencesArray);
    }
  }

  void waitForFences(vk::ArrayProxy<const std::shared_ptr<Fence>> fences, bool all, uint32_t timeout)
  {
    if (!fences.empty())
    {
      std::vector <vk::Fence> fencesArray;
      for (const std::shared_ptr<Fence>& fence : fences)
      {
        assert(fences.front()->_device == fence->_device);
        fencesArray.push_back(*fence);
      }
      static_cast<vk::Device>(*fences.front()->_device).waitForFences(fencesArray, all, timeout);
    }
  }*/




  void Queue::submit( vk::ArrayProxy<const SubmitInfo> submitInfos, const std::shared_ptr<Fence>& fenceIn )
  {
    // create a new fence if none has been passed to track completion of the submit.
    std::shared_ptr<Fence> fence = fenceIn ? fenceIn : _device->createFence( false );

    _submitInfos.insert( std::make_pair( fence, std::vector<SubmitInfo>( submitInfos.begin( ), submitInfos.end( ) ) ) );

    std::vector<std::vector<vk::Semaphore>> waitSemaphores;
    waitSemaphores.reserve( submitInfos.size( ) );

    std::vector<std::vector<vk::CommandBuffer>> commandBuffers;
    commandBuffers.reserve( submitInfos.size( ) );

    std::vector<std::vector<vk::Semaphore>> signalSemaphores;
    signalSemaphores.reserve( submitInfos.size( ) );

    std::vector<vk::SubmitInfo> vkSubmitInfos;
    vkSubmitInfos.reserve( submitInfos.size( ) );
    for ( auto const& si : submitInfos )
    {
      assert( si.waitSemaphores.size( ) == si.waitDstStageMasks.size( ) );

      waitSemaphores.push_back( std::vector<vk::Semaphore>( ) );
      waitSemaphores.back( ).reserve( si.waitSemaphores.size( ) );
      for ( auto const& s : si.waitSemaphores )
      {
        waitSemaphores.back( ).push_back( s ? static_cast< vk::Semaphore >( *s ) : nullptr );
      }

      commandBuffers.push_back( std::vector<vk::CommandBuffer>( ) );
      commandBuffers.back( ).reserve( si.commandBuffers.size( ) );
      for ( auto const& cb : si.commandBuffers )
      {
        commandBuffers.back( ).push_back( cb ? static_cast< vk::CommandBuffer >( *cb ) : nullptr );
      }

      signalSemaphores.push_back( std::vector<vk::Semaphore>( ) );
      signalSemaphores.back( ).reserve( si.signalSemaphores.size( ) );
      for ( auto const& s : si.signalSemaphores )
      {
        signalSemaphores.back( ).push_back( s ? static_cast< vk::Semaphore >( *s ) : nullptr );
      }

      vkSubmitInfos.push_back(
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

    _queue.submit( vkSubmitInfos, *fence );
  }

  void Queue::submit( const std::shared_ptr<CommandBuffer>& commandBuffer, const std::shared_ptr<Fence>& fence )
  {
    submit( SubmitInfo( nullptr, nullptr, commandBuffer, nullptr ), fence );
  }

  std::vector<vk::Result> Queue::present( vk::ArrayProxy<const std::shared_ptr<Semaphore>> waitSemaphores,
    vk::ArrayProxy<const std::shared_ptr<Swapchain>> swapchains, vk::ArrayProxy<const uint32_t> imageIndices )
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
    _queue.presentKHR( vk::PresentInfoKHR( waitSemaphoreData.size( ), waitSemaphoreData.data( ), swapchainData.size( ), swapchainData.data( ),
      imageIndices.data( ), results.data( ) ) );
    return results;
  }

  void Queue::waitIdle( )
  {
    _queue.waitIdle( );
  }

  Queue::Queue( const DeviceRef& device, vk::Queue queue )
    : VulkanResource( device )
    , _queue( queue )
  {

  }
}