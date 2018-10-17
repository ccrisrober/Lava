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

#include "CommandBuffer.h"

#include <pompeii/Buffer.h>
#include <pompeii/Image.h>
#include <pompeii/Device.h>
#include <pompeii/Event.h>
#include <pompeii/Framebuffer.h>
#include <pompeii/Pipeline.h>
#include <pompeii/PhysicalDevice.h>
#include <pompeii/RenderPass.h>
#include <pompeii/QueryPool.h>

namespace pompeii
{
  ImageMemoryBarrier::ImageMemoryBarrier(
    vk::AccessFlags srcAccessMask_, vk::AccessFlags dstAccessMask_,
    vk::ImageLayout oldLayout_, vk::ImageLayout newLayout_,
    uint32_t srcQueueFamilyIndex_, uint32_t dstQueueFamilyIndex_,
    const std::shared_ptr<Image>& image_,
    const vk::ImageSubresourceRange& subresourceRange_ )
    : srcAccessMask( srcAccessMask_ )
    , dstAccessMask( dstAccessMask_ )
    , oldLayout( oldLayout_ )
    , newLayout( newLayout_ )
    , srcQueueFamilyIndex( srcQueueFamilyIndex_ )
    , dstQueueFamilyIndex( dstQueueFamilyIndex_ )
    , image( image_ )
    , subresourceRange( subresourceRange_ )
  {
  }

  ImageMemoryBarrier::ImageMemoryBarrier( ImageMemoryBarrier const& rhs )
    : ImageMemoryBarrier( rhs.srcAccessMask, rhs.dstAccessMask,
      rhs.oldLayout, rhs.newLayout,
      rhs.srcQueueFamilyIndex, rhs.dstQueueFamilyIndex,
      rhs.image, rhs.subresourceRange )
  {
  }

  ImageMemoryBarrier & ImageMemoryBarrier::operator=(
    ImageMemoryBarrier const& rhs )
  {
    srcAccessMask = rhs.srcAccessMask;
    dstAccessMask = rhs.dstAccessMask;
    oldLayout = rhs.oldLayout;
    newLayout = rhs.newLayout;
    srcQueueFamilyIndex = rhs.srcQueueFamilyIndex;
    dstQueueFamilyIndex = rhs.dstQueueFamilyIndex;
    image = rhs.image;
    subresourceRange = rhs.subresourceRange;

    return *this;
  }
  CommandPool::CommandPool( const std::shared_ptr<Device>& device,
    vk::CommandPoolCreateFlags flags, uint32_t familyIndex )
    : VulkanResource( device )
#if !defined(NDEBUG)
    , _createFlags( flags )
    , _familyIndex( familyIndex )
#endif
  {
    //assert( familyIndex < device->getQueueFamilyCount( ) );
    
    vk::CommandPoolCreateInfo cci( flags, familyIndex );
    _commandPool = static_cast< vk::Device >( *_device )
      .createCommandPool( cci );
  }
  CommandPool::~CommandPool( void )
  {
    assert( _commandBuffers.empty( ) );   // due to our ref-counting, this CommandPool can only go when all its CommandBuffers are already gone!
                                           // that, means, none of the CommandBuffers are pending execution !!
    static_cast< vk::Device >( *_device ).destroyCommandPool( _commandPool );
  }
  void CommandPool::reset( vk::CommandPoolResetFlags flags )
  {
#if !defined(NDEBUG)
    for ( std::vector<std::shared_ptr<CommandBuffer>>::iterator it =
      _commandBuffers.begin( ); it != _commandBuffers.end( ); ++it )
    {
      ( *it )->onReset( );
    }
#endif
    static_cast< vk::Device >( *_device ).resetCommandPool( _commandPool, flags );
  }

  std::shared_ptr<CommandBuffer> CommandPool::allocateCommandBuffer(
    vk::CommandBufferLevel level )
  {
    std::shared_ptr<pompeii::CommandBuffer> commandBuffer = 
      std::make_shared<CommandBuffer>( shared_from_this( ), level );
#if !defined(NDEBUG)
    _commandBuffers.push_back( commandBuffer );
#endif
    return commandBuffer;
  }

  std::vector< std::shared_ptr<CommandBuffer> > 
    CommandPool::allocateCommandBuffers(
    uint32_t count, vk::CommandBufferLevel level )
  {
	std::vector<std::shared_ptr<CommandBuffer>> cmdBuffs;

	vk::Device dev(*getDevice());
	vk::CommandBufferAllocateInfo info(_commandPool, level, count );
	std::vector<vk::CommandBuffer> commandBuffers =
		dev.allocateCommandBuffers(info);
	assert(!commandBuffers.empty());

	for (const auto& cmd : commandBuffers)
	{
      auto commandBuffer = std::shared_ptr<CommandBuffer>(
		  new CommandBuffer( cmd, shared_from_this(), level ) );
	  cmdBuffs.push_back( commandBuffer );
#if !defined(NDEBUG)
	  _commandBuffers.push_back(commandBuffer);
#endif
	}
	return cmdBuffs;
  }
  
#if !defined(NDEBUG)
  uint32_t CommandPool::getFamilyIndex( void ) const
  {
    return _familyIndex;
  }

  bool CommandPool::individuallyResetCommandBuffers( void ) const
  {
    return !!( _createFlags & vk::CommandPoolCreateFlagBits::eResetCommandBuffer );
  }

  bool CommandPool::shortLivedCommandBuffers( void ) const
  {
    return !!( _createFlags & vk::CommandPoolCreateFlagBits::eTransient );
  }

  bool CommandPool::supportsCompute( void ) const
  {
    return !!( _device->getPhysicalDevice( )
      ->getQueueFamilyProperties( )[ _familyIndex ]
      .queueFlags & vk::QueueFlagBits::eCompute );
  }

  bool CommandPool::supportsGraphics( void ) const
  {
    return !!( _device->getPhysicalDevice( )->
      getQueueFamilyProperties( )[ _familyIndex ]
      .queueFlags & vk::QueueFlagBits::eGraphics );
  }

  bool CommandPool::supportsTransfer( void ) const
  {
    return !!( _device->getPhysicalDevice( )->
      getQueueFamilyProperties( )[ _familyIndex ]
      .queueFlags & vk::QueueFlagBits::eTransfer );
  }
#endif

  CommandBuffer::CommandBuffer(const vk::CommandBuffer& cmd,
	  const std::shared_ptr<CommandPool>& cmdPool,
	  vk::CommandBufferLevel level)
    : _commandPool(cmdPool)
#if !defined(NDEBUG)
	, _inRenderPass(false)
	, _isRecording(false)
	, _isResetFromCommandPool(true)
	, _level(level)
//    , _state( State::Ready )
#endif
  {
#if !defined(NDEBUG)
	for (size_t i = 0; i <= VK_QUERY_TYPE_RANGE_SIZE; ++i)
	{
	  _queryInfo[i].active = false;
	}
#endif
	_commandBuffer = cmd;
  }
  CommandBuffer::CommandBuffer( const std::shared_ptr<CommandPool>& cmdPool,
    vk::CommandBufferLevel level )
    : _commandPool( cmdPool )
#if !defined(NDEBUG)
    , _inRenderPass( false )
    , _isRecording( false )
    , _isResetFromCommandPool( true )
    , _level( level )
//    , _state( State::Ready )
#endif
  {
#if !defined(NDEBUG)
    for ( size_t i = 0; i <= VK_QUERY_TYPE_RANGE_SIZE; ++i )
    {
      _queryInfo[ i ].active = false;
    }
#endif
    vk::Device dev( *_commandPool->getDevice( ) );
    vk::CommandBufferAllocateInfo info( *_commandPool, level, 1 );
    std::vector<vk::CommandBuffer> commandBuffers = 
		dev.allocateCommandBuffers( info );
    assert( !commandBuffers.empty( ) );
    _commandBuffer = commandBuffers[ 0 ];
  }

  CommandBuffer::~CommandBuffer( void )
  {
    /*if ( _state == State::Submitted )
    {
      // TODO: Wait for finish
    }
    else if ( _state != State::Ready )
    {

    }*/
    vk::Device dev( *_commandPool->getDevice( ) );
    dev.freeCommandBuffers( *_commandPool, _commandBuffer );
  }

  void CommandBuffer::begin( vk::CommandBufferUsageFlags flags,
    const std::shared_ptr<RenderPass>& renderPass, uint32_t subpass,
    const std::shared_ptr<Framebuffer>& framebuffer,
    vk::Bool32 occlusionQueryEnable, vk::QueryControlFlags queryFlags,
    vk::QueryPipelineStatisticFlags pipelineStatistics )
  {
    //assert( _state == State::Ready );

#if !defined(NDEBUG)
    // TBD: should we introduce some FirstLevelCommandBuffer and SecondLevelCommandBuffer, where FirstLevel can have a much simpler begin function?
    //      renderPass and subPass are meaningless for FirstLevelCommandBuffer
    //      occlusionQueryEnable, queryFlags, and pipelineStatistics are ignored for FirstLevelCommandBuffer
    assert( !_isRecording );
    assert( _commandPool->individuallyResetCommandBuffers( ) || _isResetFromCommandPool );
    // From the spec:
    //    If commandBuffer is a secondary command buffer and either the occlusionQueryEnable member of 
    //    pBeginInfo is VK_FALSE, or the precise occlusion queries feature is not enabled, the queryFlags member
    //    of pBeginInfo must not contain VK_QUERY_CONTROL_PRECISE_BIT
    // -> what's "precise occlusion queries feature" ??
    assert( !renderPass || ( ( _level == vk::CommandBufferLevel::eSecondary ) && 
      ( flags & vk::CommandBufferUsageFlagBits::eRenderPassContinue ) ) );
    // From the spec:
    //    subpass is the index of the subpass within renderPass that the VkCommandBuffer will be rendering against if
    //    this is a secondary command buffer that was allocated with the VK_COMMAND_BUFFER_USAGE_RENDER_
    //    PASS_CONTINUE_BIT set.
    // -> where's the subpass within renderPass ??
    assert( !framebuffer || ( flags & vk::CommandBufferUsageFlagBits::eRenderPassContinue ) );
    assert( !( renderPass && framebuffer ) || ( renderPass->getDevice( ) == framebuffer->getDevice( ) ) );
    // From the spec:
    //    If the inherited queries feature is not enabled, occlusionQueryEnable must be VK_FALSE
    //    If the inherited queries feature is enabled, queryFlags must be a valid combination of VkQueryControlFlagBits values
    // -> where's that inherited queries feature? The VkPhysicalDeviceFeatures I currently have doesn't have that entry
    // From the spec:
    //    If flags contains VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, renderpass
    //    must not be VK_NULL_HANDLE, and must be compatible with the render pass for the render pass instance
    //    which this secondary command buffer will be executed in - see Section 8.2
    // -> just check first half, as the compatibility statement needs some extra knowledge (?)
    assert( !( flags & vk::CommandBufferUsageFlagBits::eRenderPassContinue ) || renderPass );
    // From the spec:
    //    If flags contains VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT, and
    //    framebuffer is not VK_NULL_HANDLE, framebuffer must match the VkFramebuffer that is specified
    //    by vkCmdBeginRenderPass for the render pass instance which this secondary command buffer will be
    //    executed in
    // -> needs a check in beginRenderPass ?
    /*assert( !( ( flags & vk::CommandBufferUsageFlagBits::eRenderPassContinue ) && 
      framebuffer ) || ( renderPass && framebuffer->get<RenderPass>( )->isCompatible( renderPass ) ) );*/
    // From the spec:
    //    If renderPass is not VK_NULL_HANDLE, subpass must refer to a valid subpass index within
    //    renderPass, specifically the index of the subpass which this secondary command buffer will be executed in
    //  -> VkRenderPass has no subpass index ?!?

    _flags = flags;
    _isRecording = true;
    _isResetFromCommandPool = false;
    _occlusionQueryEnable = occlusionQueryEnable;
    _stageFlags = vk::PipelineStageFlags( );
#endif

    _renderPass = renderPass;
    _framebuffer = framebuffer;

    vk::CommandBufferInheritanceInfo inheritanceInfo;
    vk::CommandBufferBeginInfo beginInfo( flags, &inheritanceInfo );

    inheritanceInfo.renderPass = renderPass ? *renderPass : vk::RenderPass( );
    inheritanceInfo.subpass = subpass;
    inheritanceInfo.framebuffer = framebuffer ? *framebuffer : vk::Framebuffer( );
    inheritanceInfo.occlusionQueryEnable = occlusionQueryEnable;
    inheritanceInfo.queryFlags = queryFlags;
    inheritanceInfo.pipelineStatistics = pipelineStatistics;

    _commandBuffer.begin( beginInfo );

    //_state = State::Recording;
  }


  void CommandBuffer::end( void )
  {
#if !defined(NDEBUG)
    assert( _isRecording );
    _isRecording = false;
    // From the spec:
    //    vkEndCommandBuffer must not be called inside a render pass instance
    //    All queries made active during the recording of commandBuffer must have been made inactive
#endif
    //assert( _state == State::Recording );

    _commandBuffer.end( );

    //_state = State::RecordingDone;
  }

  void CommandBuffer::resetEvent( const std::shared_ptr<Event>& ev, 
    vk::PipelineStageFlags stageMask )
  {
    assert( !!stageMask );
    assert( _isRecording );
    assert( _commandPool->supportsGraphics( ) || _commandPool->supportsCompute( ) );
    assert( !_inRenderPass );
    assert( _commandPool->getDevice( ) == ev->getDevice( ) );
    // From the spec:
    //    If the geometry shaders feature is not enabled, stageMask must not contain VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
    //    If the tessellation shaders feature is not enabled, stageMask must not contain VK_PIPELINE_STAGE_
    //    TESSELLATION_CONTROL_SHADER_BIT or VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
    _commandBuffer.resetEvent( *ev, stageMask );
  }

  void CommandBuffer::setEvent( const std::shared_ptr<Event>& ev, 
    vk::PipelineStageFlags stageMask )
  {
    assert( !!stageMask );
    assert( _isRecording );
    assert( _commandPool->supportsGraphics( ) || _commandPool->supportsCompute( ) );
    assert( !_inRenderPass );
    assert( _commandPool->getDevice( ) == ev->getDevice( ) );
    // From the spec:
    //    If the geometry shaders feature is not enabled, stageMask must not contain VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
    //    If the tessellation shaders feature is not enabled, stageMask must not contain VK_PIPELINE_STAGE_
    //    TESSELLATION_CONTROL_SHADER_BIT or VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
#if !defined(NDEBUG)
    _stageFlags |= stageMask;
#endif
    _commandBuffer.setEvent( *ev, stageMask );
  }

  void CommandBuffer::waitEvents(
    vk::ArrayProxy<const std::shared_ptr<Event>> events,
    vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
    vk::ArrayProxy<const vk::MemoryBarrier> memoryBarriers,
    vk::ArrayProxy<const vk::BufferMemoryBarrier> bufferMemoryBarriers,
    vk::ArrayProxy<const vk::ImageMemoryBarrier> imageMemoryBarriers
  )
  {
    assert( !!srcStageMask );
    assert( !!dstStageMask );
    assert( _isRecording );
    assert( _commandPool->supportsGraphics( ) || _commandPool->supportsCompute( ) );
    assert( !events.empty( ) );
    assert( _stageFlags == srcStageMask );

    // From the spec:
    //    If vkSetEvent was used to signal any of the events in pEvents, srcStageMask must include the VK_
    //    PIPELINE_STAGE_HOST_BIT flag
    // -> is there any constraint on when that vkSetEvent had happened?? would need to add VK_PIPELINE_STAGE_HOST_BIT to _stageFlags
    // From the spec:
    //    If the geometry shaders feature is not enabled, srcStageMask must not contain VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
    //    If the geometry shaders feature is not enabled, dstStageMask must not contain VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
    //    If the tessellation shaders feature is not enabled, srcStageMask must not contain VK_PIPELINE_STAGE_
    //    TESSELLATION_CONTROL_SHADER_BIT or VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
    //    If the tessellation shaders feature is not enabled, dstStageMask must not contain VK_PIPELINE_STAGE_
    //    TESSELLATION_CONTROL_SHADER_BIT or VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT
    // From the spec:
    //    If the value of memoryBarrierCount is not 0, any given element of ppMemoryBarriers must point to a
    //    valid vk::MemoryBarrier, vk::BufferMemoryBarrier or vk::ImageMemoryBarrier structure
    // From the spec:
    //    If pEvents includes one or more events that will be signaled by vkSetEvent after commandBuffer has
    //    been submitted to a queue, then vkCmdWaitEvents must not be called inside a render pass instance

    std::vector<vk::Event> evts;
    for ( const auto& e : events )
    {
      assert( _commandPool->getDevice( ) == e->getDevice( ) );
      evts.push_back( *e );
    }

    _commandBuffer.waitEvents(
      evts, srcStageMask, dstStageMask, memoryBarriers,
      bufferMemoryBarriers, imageMemoryBarriers
    );
  }

  void CommandBuffer::beginRenderPass( const vk::RenderPassBeginInfo& beginInfo, 
    vk::SubpassContents contents )
  {
#if !defined(NDEBUG)
    assert( _level == vk::CommandBufferLevel::ePrimary );
    _inRenderPass = true;
#endif
    _commandBuffer.beginRenderPass( beginInfo, contents );
  }

  void CommandBuffer::beginRenderPass( const std::shared_ptr<RenderPass>& rp,
    const std::shared_ptr<Framebuffer>& framebuffer, const vk::Rect2D& area,
    vk::ArrayProxy<const vk::ClearValue> clearValues, vk::SubpassContents cnts )
  {
    //assert( _state == State::Recording );
#if !defined(NDEBUG)
    assert( _level == vk::CommandBufferLevel::ePrimary );
    _inRenderPass = true;
#endif
    _renderPass = rp;
    _framebuffer = framebuffer;

    vk::RenderPassBeginInfo renderPassBeginInfo;

    renderPassBeginInfo.renderPass = *rp;
    renderPassBeginInfo.framebuffer = *framebuffer;
    renderPassBeginInfo.renderArea = area;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>( clearValues.size( ) );
    renderPassBeginInfo.pClearValues = reinterpret_cast<vk::ClearValue const*>( clearValues.data( ) );

    _commandBuffer.beginRenderPass( renderPassBeginInfo, cnts );

    //_state = State::RecordingRenderPass;
  }

  void CommandBuffer::fillBuffer( const std::shared_ptr<pompeii::Buffer>& dstBuffer,
    vk::DeviceSize dstOffset, vk::DeviceSize fillSize, uint32_t data )
  {
    _commandBuffer.fillBuffer( *dstBuffer, dstOffset, fillSize, data );
  }

  void CommandBuffer::nextSubpass( vk::SubpassContents contents )
  {
    assert( _level == vk::CommandBufferLevel::ePrimary );
    _commandBuffer.nextSubpass( contents );
  }

  void CommandBuffer::endRenderPass( void )
  {
    //assert( _state == State::RecordingRenderPass );

#if !defined(NDEBUG)
    assert( _level == vk::CommandBufferLevel::ePrimary );
    _inRenderPass = false;
#endif
    // TODO actually a commandBuffer should keep a std::vector for those
    _renderPass.reset( );
    _framebuffer.reset( );

    _commandBuffer.endRenderPass( );

    //_state = State::Recording;
  }

  void CommandBuffer::executeCommands(
    const std::vector<std::shared_ptr<pompeii::CommandBuffer>>& secondaryCmds )
  {
    assert( _isRecording );
    assert( _commandPool->supportsCompute( ) || _commandPool->supportsGraphics( ) || _commandPool->supportsTransfer( ) );
    assert( _level == vk::CommandBufferLevel::ePrimary );
    assert( !secondaryCmds.empty( ) );
    // From the Spec:
    //    If the inherited queries feature is not enabled, commandBuffer must not have any queries active
    // -> where's that inherited queries feature? The VkPhysicalDeviceFeatures I currently have doesn't have that entry

    _secondaryCommandBuffers.insert( 
      _secondaryCommandBuffers.end( ), secondaryCmds.begin( ), secondaryCmds.end( ) );

    std::vector<vk::CommandBuffer> commands;
    for ( auto const& it : secondaryCmds )
    {
      /*assert( _commandPool->getDevice( ) == it->_commandPool->getDevice( ) );
      assert( it->_level == vk::CommandBufferLevel::eSecondary );
      assert( ( it->_flags & vk::CommandBufferUsageFlagBits::eSimultaneousUse ) || 
        ( std::count( _secondaryCommandBuffers.begin( ), _secondaryCommandBuffers.end( ), it ) == 1 ) );
      assert( !it->getPrimaryCommandBuffer( ) );
      assert( !it->isRecording( ) );
      assert( !_inRenderPass || ( it->_flags & 
        vk::CommandBufferUsageFlagBits::eRenderPassContinue ) );
      assert( !_queryInfo[ VK_QUERY_TYPE_OCCLUSION ].active || 
        ( it->_occlusionQueryEnable && 
        ( it->_queryInfo[ VK_QUERY_TYPE_OCCLUSION ].flags == 
          _queryInfo[ VK_QUERY_TYPE_OCCLUSION ].flags ) ) );
      assert( !_queryInfo[ VK_QUERY_TYPE_PIPELINE_STATISTICS ].active || 
        ( it->_queryInfo[ VK_QUERY_TYPE_PIPELINE_STATISTICS ].flags == 
          _queryInfo[ VK_QUERY_TYPE_PIPELINE_STATISTICS ].flags ) );
#if !defined(NDEBUG)
      for ( size_t i = 0; i<VK_QUERY_TYPE_RANGE_SIZE; i++ )
      {
        assert( !_queryInfo[ i ].active || !it->_queryInfo[ i ].contained );
      }
#endif*/

      commands.push_back( *it );

/*#if !defined(NDEBUG)
      it->setPrimaryCommandBuffer( shared_from_this( ) );
#endif*/
    }
    _commandBuffer.executeCommands( commands );
  }


  void CommandBuffer::clearAttachments(
    vk::ArrayProxy<const vk::ClearAttachment> attachments,
    vk::ArrayProxy<const vk::ClearRect> rects )
  {
    _commandBuffer.clearAttachments( attachments, rects );
  }

  void CommandBuffer::clearColorImage( const std::shared_ptr<Image>& img,
    vk::ImageLayout imageLayout, const vk::ClearColorValue& color,
    vk::ArrayProxy<const vk::ImageSubresourceRange> ranges )
  {
    _commandBuffer.clearColorImage( *img, imageLayout, color, ranges );
  }

  void CommandBuffer::clearDepthStencilImage( const std::shared_ptr<Image>& img,
    vk::ImageLayout imageLayout, float depth, uint32_t stencil,
    vk::ArrayProxy<const vk::ImageSubresourceRange> ranges )
  {
    vk::ClearDepthStencilValue depthStencil{ depth, stencil };
    _commandBuffer.clearDepthStencilImage( *img, imageLayout, depthStencil,
      ranges );
  }

  void CommandBuffer::resolveImage( const std::shared_ptr<Image>& srcImage, 
    const std::shared_ptr<Image>& dstImage,
    const vk::ImageResolve& region )
  {
	  _commandBuffer.resolveImage(
		  *srcImage, srcImage->layout, 
		  *dstImage, dstImage->layout, 
		  region);
  }

  void CommandBuffer::blitImage( const std::shared_ptr<Image>& srcImage,
    vk::ImageLayout srcImageLayout, const std::shared_ptr<Image>& dstImage,
    vk::ImageLayout dstImageLayout, vk::ArrayProxy<const vk::ImageBlit> regions,
    vk::Filter filter )
  {
    _commandBuffer.blitImage( *srcImage, srcImageLayout, *dstImage,
      dstImageLayout, regions, filter );
  }

  void CommandBuffer::beginQuery(
    const std::shared_ptr<pompeii::QueryPool>& queryPool, uint32_t slot,
    vk::QueryControlFlags flags )
  {
#if !defined(NDEBUG)
    int queryInfoIndex = int( queryPool->getQueryType( ) ) - VK_QUERY_TYPE_BEGIN_RANGE;
    assert( !_queryInfo[ queryInfoIndex ].active );
    _queryInfo[ queryInfoIndex ].active = true;
    _queryInfo[ queryInfoIndex ].contained = true;
    _queryInfo[ queryInfoIndex ].flags = flags;
#endif
    _commandBuffer.beginQuery( *queryPool, slot, flags );
  }

  void CommandBuffer::beginQuery( const std::shared_ptr<pompeii::QueryPool>& queryPool,
    uint32_t slot, bool precise )
  {
    vk::QueryControlFlags flags;
    if (precise)
    {
      flags |= vk::QueryControlFlagBits::ePrecise;
    }
#if !defined(NDEBUG)
    int queryInfoIndex = int( queryPool->getQueryType( ) ) - VK_QUERY_TYPE_BEGIN_RANGE;
    assert( !_queryInfo[ queryInfoIndex ].active );
    _queryInfo[ queryInfoIndex ].active = true;
    _queryInfo[ queryInfoIndex ].contained = true;
    _queryInfo[ queryInfoIndex ].flags = flags;
#endif
    _commandBuffer.beginQuery( *queryPool, slot, flags );
  }

  void CommandBuffer::copyQueryPoolResults(
    const std::shared_ptr<pompeii::QueryPool>& queryPool, uint32_t startQuery,
    uint32_t queryCount, const std::shared_ptr<pompeii::Buffer>& dstBuffer,
    vk::DeviceSize dstOffset, vk::DeviceSize dstStride,
    vk::QueryResultFlags flags )
  {
    _commandBuffer.copyQueryPoolResults( *queryPool, startQuery, queryCount,
      *dstBuffer, dstOffset, dstStride, flags );
  }

  void CommandBuffer::endQuery(
    const std::shared_ptr<pompeii::QueryPool>& queryPool, uint32_t slot )
  {
#if !defined(NDEBUG)
    assert( _queryInfo[ int( queryPool->getQueryType( ) ) - 
      VK_QUERY_TYPE_BEGIN_RANGE ].active );
    _queryInfo[ int( queryPool->getQueryType( ) ) - 
      VK_QUERY_TYPE_BEGIN_RANGE ].active = false;
#endif
    _commandBuffer.endQuery( *queryPool, slot );
  }

  void CommandBuffer::resetQueryPool(
    const std::shared_ptr<pompeii::QueryPool>& queryPool, uint32_t startQuery,
    uint32_t queryCount )
  {
    _commandBuffer.resetQueryPool( *queryPool, startQuery, queryCount );
  }

  void CommandBuffer::writeTimestamp( vk::PipelineStageFlagBits pipelineStage,
    const std::shared_ptr<pompeii::QueryPool>& queryPool, uint32_t entry )
  {
    _commandBuffer.writeTimestamp( pipelineStage, *queryPool, entry );
  }

  void CommandBuffer::setViewportScissors( uint32_t width, uint32_t height )
  {
    setScissor( 0, vk::Rect2D( { 0, 0 }, { width, height } ) );
    setViewport( 0, vk::Viewport( 0.0f, 0.0f, ( float ) width, ( float ) height,
      0.0f, 1.0f ) );
  }
  void CommandBuffer::setViewportScissors( const vk::Extent2D& dimensions )
  {
    setViewportScissors( dimensions.width, dimensions.height );
  }
  void CommandBuffer::setScissor( uint32_t first,
    vk::ArrayProxy<const vk::Rect2D> scissors )
  {
    _commandBuffer.setScissor( first, scissors );
  }
  void CommandBuffer::setScissor( const vk::Rect2D scissor )
  {
	this->setScissor( 0, scissor );
  }
  void CommandBuffer::setViewport( uint32_t first,
    vk::ArrayProxy<const vk::Viewport> viewports )
  {
    _commandBuffer.setViewport( first, viewports );
  }
  void CommandBuffer::setViewport( const vk::Viewport viewport )
  {
	this->setViewport( 0, viewport);
  }
  void CommandBuffer::setDepthBias( float depthBias, float depthBiasClamp,
    float slopeScaledDepthBias )
  {
    _commandBuffer.setDepthBias( depthBias, depthBiasClamp,
      slopeScaledDepthBias );
  }
  void CommandBuffer::setDepthBounds( float minDepthBounds,
    float maxDepthBounds )
  {
    _commandBuffer.setDepthBounds( minDepthBounds, maxDepthBounds );
  }

  void CommandBuffer::setLineWidth( float lineWidth )
  {
    _commandBuffer.setLineWidth( lineWidth );
  }

  void CommandBuffer::setBlendConstants( const float blendConst[ 4 ] )
  {
    _commandBuffer.setBlendConstants( blendConst );
  }

  void CommandBuffer::setStencilCompareMask( vk::StencilFaceFlags faceMask,
    uint32_t stencilCompareMask )
  {
    _commandBuffer.setStencilCompareMask( faceMask, stencilCompareMask );
  }

  void CommandBuffer::setStencilReference( vk::StencilFaceFlags faceMask,
    uint32_t stencilReference )
  {
    _commandBuffer.setStencilReference( faceMask, stencilReference );
  }

  void CommandBuffer::setStencilWriteMask( vk::StencilFaceFlags faceMask,
    uint32_t stencilWriteMask )
  {
    _commandBuffer.setStencilWriteMask( faceMask, stencilWriteMask );
  }

  void CommandBuffer::dispatch( uint32_t x, uint32_t y, uint32_t z )
  {
    _commandBuffer.dispatch( x, y, z );
  }
  void CommandBuffer::draw( uint32_t vertexCount, uint32_t instanceCount,
    uint32_t firstVertex, uint32_t firstInst )
  {
    _commandBuffer.draw( vertexCount, instanceCount, firstVertex, firstInst );
  }

  void CommandBuffer::drawIndirect( const std::shared_ptr<Buffer>& buffer,
    vk::DeviceSize offset, uint32_t count, uint32_t stride )
  {
    _commandBuffer.drawIndirect( *buffer, offset, count, stride );
  }

  void CommandBuffer::drawIndexed( uint32_t indexCount, uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset, uint32_t firstInstance )
  {
    _commandBuffer.drawIndexed( indexCount, instanceCount, firstIndex,
      vertexOffset, firstInstance );
  }

  void CommandBuffer::drawIndexedIndirect( const std::shared_ptr<Buffer>& buffer,
    vk::DeviceSize offset, uint32_t count, uint32_t stride )
  {
    _commandBuffer.drawIndexedIndirect( *buffer, offset, count, stride );
  }

  void CommandBuffer::draw( int vertexCount )
  {
    this->draw( vertexCount, 1, 0, 0 );
  }

  void CommandBuffer::drawIndexed( int indexCount )
  {
    this->drawIndexed( indexCount, 1, 0, 0, 0 );
  }

  void CommandBuffer::reset( vk::CommandBufferResetFlagBits flags )
  {
    assert( _commandPool->individuallyResetCommandBuffers( ) );
    assert( !isRecording( ) );
    _commandBuffer.reset( flags );
    
    _renderPass.reset( );
    _framebuffer.reset( );
    _secondaryCommandBuffers.clear( );
#if !defined(NDEBUG)
    _stageFlags = vk::PipelineStageFlags( );
#endif
    //_state = State::Ready; // TODO ?
  }

  void CommandBuffer::pushConstant( 
    const std::shared_ptr<PipelineLayout>& layout,
    vk::ShaderStageFlagBits stageFlags, void * data )
  {
    _commandBuffer.pushConstants( *layout, stageFlags, 0, sizeof( data ), data );
  }

  void CommandBuffer::bindVertexBuffer( uint32_t startBinding,
    const std::shared_ptr<Buffer>& buffer, vk::DeviceSize offset )
  {
    _commandBuffer.bindVertexBuffers( startBinding,
      static_cast< vk::Buffer >( *buffer ), offset );
  }

  void CommandBuffer::bindIndexBuffer( const std::shared_ptr<Buffer>& buffer,
    vk::DeviceSize offset, vk::IndexType indexType )
  {
    _commandBuffer.bindIndexBuffer( *buffer, offset, indexType );
  }

  void CommandBuffer::bindIndexBuffer(
    const std::shared_ptr<IndexBuffer>& buffer, vk::DeviceSize offset )
  {
    _commandBuffer.bindIndexBuffer( *buffer, offset, buffer->getIndexType( ) );
  }

  void CommandBuffer::bindVertexBuffers( uint32_t startBinding,
    vk::ArrayProxy<const std::shared_ptr<Buffer>> buffers,
    vk::ArrayProxy<const vk::DeviceSize> offsets )
  {
    assert( buffers.size( ) == offsets.size( ) );

    _bindVertexBuffers.clear( );
    for ( auto & buffer : buffers )
    {
      _bindVertexBuffers.push_back( *buffer );
    }

    _commandBuffer.bindVertexBuffers( startBinding, _bindVertexBuffers,
      offsets );
  }


  void CommandBuffer::copyBuffer( const std::shared_ptr<Buffer>& srcBuffer, 
    const std::shared_ptr<Buffer>& dstBuffer, 
    vk::ArrayProxy<const vk::BufferCopy> regions )
  {
    _commandBuffer.copyBuffer( *srcBuffer, *dstBuffer, regions );
  }

  void CommandBuffer::copyBufferToImage( const std::shared_ptr<Buffer>& srcBuffer, 
    const std::shared_ptr<Image>& dstImage, vk::ImageLayout dstImageLayout, 
    vk::ArrayProxy<const vk::BufferImageCopy> regions )
  {
    _commandBuffer.copyBufferToImage( *srcBuffer, *dstImage, dstImageLayout,
      regions );
  }

  void CommandBuffer::copyImage( const std::shared_ptr<Image>& srcImage,
    vk::ImageLayout srcImageLayout, const std::shared_ptr<Image>& dstImage,
    vk::ImageLayout dstImageLayout, vk::ArrayProxy<const vk::ImageCopy> regions )
  {
    _commandBuffer.copyImage( *srcImage, srcImageLayout, *dstImage,
      dstImageLayout, regions );
  }

  void CommandBuffer::copyImageToBuffer( const std::shared_ptr<Image>& srcImage,
    vk::ImageLayout srcImageLayout, const std::shared_ptr<Buffer>& dstBuffer,
    vk::ArrayProxy<const vk::BufferImageCopy> regions )
  {
    _commandBuffer.copyImageToBuffer( *srcImage, srcImageLayout, *dstBuffer,
      regions );
  }

  void CommandBuffer::pipelineBarrier( vk::PipelineStageFlags srcStageMask,
    vk::PipelineStageFlags destStageMask, vk::DependencyFlags depFlags,
    vk::ArrayProxy<const vk::MemoryBarrier> barriers,
    vk::ArrayProxy<const vk::BufferMemoryBarrier> bufferMemoryBarriers,
    vk::ArrayProxy<const ImageMemoryBarrier> imageMemoryBarriers )
  {
    std::vector<vk::ImageMemoryBarrier> imbs;
    imbs.reserve( imageMemoryBarriers.size( ) );
    for ( auto const& imb : imageMemoryBarriers )
    {
      imbs.push_back( vk::ImageMemoryBarrier(
        imb.srcAccessMask, imb.dstAccessMask,
        imb.oldLayout, imb.newLayout,
        imb.srcQueueFamilyIndex, imb.dstQueueFamilyIndex,
        imb.image ? static_cast<vk::Image>( *imb.image ) : nullptr,
        imb.subresourceRange ) );
    }

    _commandBuffer.pipelineBarrier( srcStageMask, destStageMask, depFlags,
      barriers, bufferMemoryBarriers, imbs );
  }

  void CommandBuffer::pipelineBarrier(
    vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags destStageMask,
    const vk::ArrayProxy< std::shared_ptr< Buffer > >& buffers, 
    const vk::BufferMemoryBarrier& barrier, vk::DependencyFlags dependencyFlags )
  {
    std::vector< vk::BufferMemoryBarrier > bmbs;
    bmbs.reserve( buffers.size( ) );
    for( auto buff: buffers )
    {
      vk::BufferMemoryBarrier bmb;
      bmb.srcAccessMask = barrier.srcAccessMask;
      bmb.dstAccessMask = barrier.dstAccessMask;
      bmb.srcQueueFamilyIndex = barrier.srcQueueFamilyIndex;
      bmb.dstQueueFamilyIndex = barrier.dstQueueFamilyIndex;
      bmb.srcAccessMask = barrier.srcAccessMask;
      bmb.buffer = static_cast< vk::Buffer >( *buff );
      bmb.offset = barrier.offset;
      bmb.size = buff->getSize( );
      bmbs.push_back( bmb );
    }

    _commandBuffer.pipelineBarrier( srcStageMask, destStageMask, dependencyFlags,
      { }, bmbs, { } );
  }

  void CommandBuffer::pipelineBarrier(
    vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags destStageMask,
    const vk::ArrayProxy< std::shared_ptr< Image > >& images, 
    const ImageMemoryBarrier& imb, vk::DependencyFlags dependencyFlags )
  {
    std::vector<vk::ImageMemoryBarrier> imbs;
    imbs.reserve( images.size( ) );
    for ( auto const& image : images )
    {
      imbs.push_back( vk::ImageMemoryBarrier(
        imb.srcAccessMask, imb.dstAccessMask,
        imb.oldLayout, imb.newLayout,
        imb.srcQueueFamilyIndex, imb.dstQueueFamilyIndex,
        imb.image ? static_cast<vk::Image>( *image ) : nullptr,
        imb.subresourceRange ) );
    }

    _commandBuffer.pipelineBarrier( srcStageMask, destStageMask, dependencyFlags,
      { }, { }, imbs );
  }

  void CommandBuffer::onReset( void )
  {
    _renderPass.reset( );
    _framebuffer.reset( );
    _secondaryCommandBuffers.clear( );

#if !defined(NDEBUG)
    _isResetFromCommandPool = true;
#endif
  }

  void CommandBuffer::bindDescriptorSets(
    vk::PipelineBindPoint pipelineBindPoint,
    const std::shared_ptr<PipelineLayout>& pipelineLayout, uint32_t firstSet,
    vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> descriptorSets,
	  vk::ArrayProxy<const uint32_t> dynamicOffsets)
  {
    _bindDescriptorSets.clear( );
    for ( auto& descriptor : descriptorSets )
    {
      _bindDescriptorSets.push_back( *descriptor );
    }

    _commandBuffer.bindDescriptorSets( pipelineBindPoint, *pipelineLayout,
      firstSet, _bindDescriptorSets, dynamicOffsets );
  }

  void CommandBuffer::bindPipeline( vk::PipelineBindPoint bindingPoint,
    const std::shared_ptr<Pipeline>& pipeline )
  {
    _commandBuffer.bindPipeline( bindingPoint, *pipeline );
  }
  void CommandBuffer::bindGraphicsPipeline( const std::shared_ptr<Pipeline>& p )
  {
    _commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, *p );
  }
  void CommandBuffer::bindComputePipeline( const std::shared_ptr<Pipeline>& p )
  {
    _commandBuffer.bindPipeline( vk::PipelineBindPoint::eCompute, *p );
  }

  /*void CommandBuffer::pushDescriptorSetKHR( vk::PipelineBindPoint bindpoint,
    std::shared_ptr<PipelineLayout> pipLayout, uint32_t set,
    vk::ArrayProxy<WriteDescriptorSet> descriptorWrites )
  {*/
    /*if ( !vkCmdPushDescriptorSetKHR )
    {
      VkDevice device = static_cast< VkDevice >
        ( static_cast<vk::Device>( *_commandPool->getDevice( ) ) );
      vkCmdPushDescriptorSetKHR =
        ( PFN_vkCmdPushDescriptorSetKHR ) vkGetDeviceProcAddr(
          device, "vkCmdPushDescriptorSetKHR" );
    }
    if ( !vkCmdPushDescriptorSetKHR )
    {
      throw;
    }*/
    /*std::vector<std::unique_ptr<vk::DescriptorImageInfo>> diis;
    diis.reserve( descriptorWrites.size( ) );

    std::vector<std::unique_ptr<vk::DescriptorBufferInfo>> dbis;
    dbis.reserve( descriptorWrites.size( ) );

    std::vector<vk::WriteDescriptorSet> writes;
    writes.reserve( descriptorWrites.size( ) );
    for ( const auto& w : descriptorWrites )
    {
      diis.push_back( std::unique_ptr<vk::DescriptorImageInfo>(
        w.imageInfo ? new vk::DescriptorImageInfo(
          w.imageInfo->sampler ?
          static_cast<vk::Sampler>( *w.imageInfo->sampler ) : nullptr,
          w.imageInfo->imageView ?
          static_cast<vk::ImageView>( *w.imageInfo->imageView ) : nullptr,
          w.imageInfo->imageLayout ) : nullptr ) );
      dbis.push_back( std::unique_ptr<vk::DescriptorBufferInfo>(
        w.bufferInfo ? new vk::DescriptorBufferInfo( w.bufferInfo->buffer ?
          static_cast<vk::Buffer>( *w.bufferInfo->buffer ) : nullptr,
          w.bufferInfo->offset, w.bufferInfo->range ) : nullptr ) );
      vk::WriteDescriptorSet write(
        nullptr,
        w.dstBinding,
        w.dstArrayElement,
        w.descriptorCount,
        w.descriptorType,
        diis.back( ).get( ),
        dbis.back( ).get( )
      );

      if ( w.texelBufferView )
      {
        auto bufferView = static_cast< vk::BufferView >( *w.texelBufferView );
        // TODO (LINUX FAILED) auto bb = static_cast< VkBufferView >( bufferView );
        write.setPTexelBufferView( &bufferView );
      }

      writes.push_back( std::move( write ) );
    }*/
    /*VkCommandBuffer m_commandBuffer = static_cast< VkCommandBuffer >( _commandBuffer );
    vk::PipelineLayout layout = *pipLayout;
    std::vector<VkWriteDescriptorSet> vkwds( descriptorWrites.size( ) );
    vkwds[ 0 ] = static_cast< VkWriteDescriptorSet >( writes.at( 0 ) );
    vkwds[ 1 ] = static_cast< VkWriteDescriptorSet >( writes.at( 1 ) );
    vkCmdPushDescriptorSetKHR( m_commandBuffer, 
      static_cast<VkPipelineBindPoint>( bindpoint ), 
      static_cast<VkPipelineLayout>( layout ), 
      set, descriptorWrites.size( ), 
      vkwds.data( ) );*/
    /*if(vkCmdPushDescriptorSetKHR == NULL)
    {
      throw;
    }
    _commandBuffer.pushDescriptorSetKHR( bindpoint, *pipLayout, set, writes );
  }*/
}