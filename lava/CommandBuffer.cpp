#include "CommandBuffer.h"

#include "Device.h"
#include "Image.h"
#include "Buffer.h"

namespace lava
{
  CommandPool::CommandPool( const DeviceRef& device, vk::CommandPoolCreateFlags flags,
    uint32_t familyIndex )
    : VulkanResource( device )
  {
    vk::CommandPoolCreateInfo cci( flags, familyIndex );
    _commandPool = static_cast< vk::Device >( *_device ).createCommandPool( cci );
  }
  CommandPool::~CommandPool( )
  {
    static_cast< vk::Device >( *_device ).destroyCommandPool( _commandPool );
  }
  std::shared_ptr<CommandBuffer> CommandPool::allocateCommandBuffer( vk::CommandBufferLevel level )
  {
    std::shared_ptr<CommandBuffer> commandBuffer =
      std::make_shared<CommandBuffer>( shared_from_this( ), level );
    _commandBuffers.push_back( commandBuffer.get( ) );
    return( commandBuffer );
  }
  CommandBuffer::CommandBuffer( const std::shared_ptr<CommandPool>& cmdPool, vk::CommandBufferLevel level )
    : _commandPool( cmdPool )
  {
    vk::CommandBufferAllocateInfo info( *_commandPool, level, 1 );
    std::vector<vk::CommandBuffer> commandBuffers =
      static_cast< vk::Device >( *_commandPool->getDevice( ) ).allocateCommandBuffers( info );
    assert( !commandBuffers.empty( ) );
    _commandBuffer = commandBuffers[ 0 ];

    _isRecording = false;
  }
  CommandBuffer::~CommandBuffer( void )
  {
  }

  void CommandBuffer::clearAttachments( vk::ArrayProxy<const vk::ClearAttachment> attachments,
    vk::ArrayProxy<const vk::ClearRect> rects )
  {
    _commandBuffer.clearAttachments( attachments, rects );
  }

  void CommandBuffer::clearColorImage( const std::shared_ptr<Image>& image,
    vk::ImageLayout imageLayout, const vk::ClearColorValue& color,
    vk::ArrayProxy<const vk::ImageSubresourceRange> ranges )
  {
    _commandBuffer.clearColorImage( *image, imageLayout, color, ranges );
  }

  void CommandBuffer::clearDepthStencilImage( const std::shared_ptr<Image>& image,
    vk::ImageLayout imageLayout, float depth, uint32_t stencil,
    vk::ArrayProxy<const vk::ImageSubresourceRange> ranges )
  {
    vk::ClearDepthStencilValue depthStencil{ depth, stencil };
    _commandBuffer.clearDepthStencilImage( *image, imageLayout, depthStencil, ranges );
  }

  void CommandBuffer::beginRenderPass( const std::shared_ptr<RenderPass>& renderPass,
    const std::shared_ptr<Framebuffer>& framebuffer, const vk::Rect2D& area,
    vk::ArrayProxy<const vk::ClearValue> clearValues, vk::SubpassContents contents )
  {
    _renderPass = renderPass;
    _framebuffer = framebuffer;

    vk::RenderPassBeginInfo renderPassBeginInfo;

    renderPassBeginInfo.renderPass = *renderPass;
    renderPassBeginInfo.framebuffer = *framebuffer;
    renderPassBeginInfo.renderArea = area;
    renderPassBeginInfo.clearValueCount = clearValues.size( );
    renderPassBeginInfo.pClearValues = reinterpret_cast< vk::ClearValue const* >( clearValues.data( ) );

    _commandBuffer.beginRenderPass( renderPassBeginInfo, contents );
  }
  void CommandBuffer::endRenderPass( )
  {
    _renderPass.reset( );
    _framebuffer.reset( );

    _commandBuffer.endRenderPass( );
  }
  void CommandBuffer::setStencilCompareMask( vk::StencilFaceFlags faceMask, uint32_t stencilCompareMask )
  {
    _commandBuffer.setStencilCompareMask( faceMask, stencilCompareMask );
  }
  void CommandBuffer::setStencilReference( vk::StencilFaceFlags faceMask, uint32_t stencilReference )
  {
    _commandBuffer.setStencilReference( faceMask, stencilReference );
  }
  void CommandBuffer::setStencilWriteMask( vk::StencilFaceFlags faceMask, uint32_t stencilWriteMask )
  {
    _commandBuffer.setStencilWriteMask( faceMask, stencilWriteMask );
  }
  void CommandBuffer::setLineWidth( float lineWidth )
  {
    _commandBuffer.setLineWidth( lineWidth );
  }
  void CommandBuffer::bindPipeline( vk::PipelineBindPoint bindingPoint,
    const std::shared_ptr<Pipeline>& pipeline )
  {
    _commandBuffer.bindPipeline( bindingPoint, *pipeline );
  }
  void CommandBuffer::setScissor( uint32_t first, vk::ArrayProxy<const vk::Rect2D> scissors )
  {
    _commandBuffer.setScissor( first, scissors );
  }
  void CommandBuffer::setViewport( uint32_t first, vk::ArrayProxy<const vk::Viewport> viewports )
  {
    _commandBuffer.setViewport( first, viewports );
  }
  void CommandBuffer::dispatch( uint32_t x, uint32_t y, uint32_t z )
  {
    _commandBuffer.dispatch( x, y, z );
  }
  void CommandBuffer::draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
    uint32_t firstInstance )
  {
    _commandBuffer.draw( vertexCount, instanceCount, firstVertex, firstInstance );
  }

  void CommandBuffer::drawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
    int32_t vertexOffset, uint32_t firstInstance )
  {
    _commandBuffer.drawIndexed( indexCount, instanceCount, firstIndex, vertexOffset, firstInstance );
  }

  void CommandBuffer::bindVertexBuffer( uint32_t startBinding, const std::shared_ptr<Buffer>& buffer, vk::DeviceSize offset )
  {
    _commandBuffer.bindVertexBuffers( startBinding, static_cast< vk::Buffer >( *buffer ), offset );
  }

  void CommandBuffer::bindVertexBuffers( uint32_t startBinding, vk::ArrayProxy<const std::shared_ptr<Buffer>> buffers, vk::ArrayProxy<const vk::DeviceSize> offsets )
  {
    assert( buffers.size( ) == offsets.size( ) );

    _bindVertexBuffers.clear( );
    for ( auto & buffer : buffers )
    {
      _bindVertexBuffers.push_back( *buffer );
    }

    _commandBuffer.bindVertexBuffers( startBinding, _bindVertexBuffers, offsets );
  }

  void CommandBuffer::bindIndexBuffer( const std::shared_ptr<Buffer>& buffer,
    vk::DeviceSize offset, vk::IndexType indexType )
  {
    _commandBuffer.bindIndexBuffer( *buffer, offset, indexType );
  }

  void CommandBuffer::begin( vk::CommandBufferUsageFlags flags,
    const std::shared_ptr<RenderPass>& renderPass, uint32_t subpass,
    const std::shared_ptr<Framebuffer>& framebuffer, vk::Bool32 occlusionQueryEnable,
    vk::QueryControlFlags queryFlags, vk::QueryPipelineStatisticFlags pipelineStatistics )
  {
    assert( !_isRecording );
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
    _isRecording = true;
  }

  void CommandBuffer::end( )
  {
    assert( _isRecording );
    _isRecording = false;
    _commandBuffer.end( );
  }
}