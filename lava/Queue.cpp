#include "Queue.h"

#include "Device.h"
#include "Swapchain.h"
#include "CommandBuffer.h"

#define DEFAULT_FENCE_TIMEOUT 100000000000

namespace lava
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

  SubmitInfo & SubmitInfo::operator=( SubmitInfo const& rhs )
  {
    waitSemaphores = rhs.waitSemaphores;
    waitDstStageMasks = rhs.waitDstStageMasks;
    commandBuffers = rhs.commandBuffers;
    signalSemaphores = rhs.signalSemaphores;
    return *this;
  }


  SparseMemoryBind::SparseMemoryBind( vk::DeviceMemory mem, 
    vk::DeviceSize memOffset, vk::DeviceSize size_, 
    vk::DeviceSize resourceOffset_ )
    : memory( mem )
    , memoryOffset( memOffset )
    , size( size_ )
    , resourceOffset( resourceOffset_ )
  {
    /*if (DEBUG_MODE)
    {
        System.out.println("Creating sparse buffer");
    }
    
    VkSparseMemoryBind.Buffer memoryBinds = VkSparseMemoryBind.calloc(1)
                                                              .memory(buffer.getMemoryBlock().getMemory())
                                                              .memoryOffset(buffer.getMemoryBlock().getOffset())
                                                              .size(buffer.getMemoryBlock().getSize())
                                                              .resourceOffset(0);
    
    VkSparseBufferMemoryBindInfo.Buffer bindInfo = VkSparseBufferMemoryBindInfo.calloc(1)
                                                                               .buffer(buffer.getBufferHandle())
                                                                               .pBinds(memoryBinds);
    
    VkBindSparseInfo sparseInfo = VkBindSparseInfo.calloc()
                                                  .sType(VK_STRUCTURE_TYPE_BIND_SPARSE_INFO)
                                                  .pBufferBinds(bindInfo);
    
    vkQueueBindSparse(deviceQueue, sparseInfo, VK_NULL_HANDLE);
    
    memoryBinds.free();
    sparseInfo.free();
    bindInfo.free();*/
  }
  SparseMemoryBind::SparseMemoryBind( const SparseMemoryBind& other )
    : memory( other.memory )
    , memoryOffset( other.memoryOffset )
    , size( other.size )
    , resourceOffset( other.resourceOffset )
  {
  }
  SparseMemoryBind& SparseMemoryBind::operator=( const SparseMemoryBind& other )
  {
    memory = other.memory;
    memoryOffset = other.memoryOffset;
    size = other.size;
    resourceOffset = other.resourceOffset;
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




  void Queue::submit( vk::ArrayProxy<const SubmitInfo> submitInfos, 
    const std::shared_ptr<Fence>& fenceIn )
  {
    // create a new fence if none has been passed to track completion of the submit.
    std::shared_ptr<Fence> fence = fenceIn ? fenceIn : _device->createFence( false );

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

    _queue.submit( to_submit, *fence );
  }

  void Queue::submit( const std::shared_ptr<CommandBuffer>& commandBuffer, 
    const std::shared_ptr<Fence>& fence )
  {
    submit( SubmitInfo( nullptr, nullptr, commandBuffer, nullptr ), fence );
  }

  std::vector<vk::Result> Queue::present( 
    vk::ArrayProxy<const std::shared_ptr<Semaphore>> waitSemaphores,
    vk::ArrayProxy<const std::shared_ptr<Swapchain>> swapchains, 
    vk::ArrayProxy<const uint32_t> imageIndices )
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
    _queue.presentKHR( vk::PresentInfoKHR( 
      waitSemaphoreData.size( ), waitSemaphoreData.data( ), 
      swapchainData.size( ), swapchainData.data( ),
      imageIndices.data( ), results.data( ) ) );
    return results;
  }

  void Queue::waitIdle( )
  {
    _queue.waitIdle( );
  }

  void Queue::submitAndWait( std::shared_ptr<CommandBuffer>& cmd )
  {
    std::shared_ptr<Fence> fence = _device->createFence( false );
    this->submit( cmd, fence );

    std::vector<vk::Fence> vkFences;
    vkFences.push_back( *fence );
    /*vkFences.reserve( fences.size( ) );
    for ( auto const& f : fences )
    {
      vkFences.push_back( *f );
    }*/

    // Wait for the fence to signal that command buffer has finished executing
    static_cast< vk::Device >( *_device ).waitForFences( vkFences, VK_TRUE, DEFAULT_FENCE_TIMEOUT );
  }

  Queue::Queue( const DeviceRef& device, vk::Queue queue, uint32_t queueFamilyIndex )
    : VulkanResource( device )
    , _queue( queue )
    , _queueFamilyIndex( queueFamilyIndex )
  {
  }
}