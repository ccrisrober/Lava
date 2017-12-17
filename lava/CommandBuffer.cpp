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
  std::shared_ptr<CommandBuffer> CommandPool::allocateCommandBuffer( 
    vk::CommandBufferLevel level )
  {
    std::shared_ptr<CommandBuffer> commandBuffer =
      std::make_shared<CommandBuffer>( shared_from_this( ), level );
    _commandBuffers.push_back( commandBuffer.get( ) );
    return( commandBuffer );
  }


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
  {}

  ImageMemoryBarrier::ImageMemoryBarrier( ImageMemoryBarrier const& rhs )
    : ImageMemoryBarrier( rhs.srcAccessMask, rhs.dstAccessMask, 
      rhs.oldLayout, rhs.newLayout, rhs.srcQueueFamilyIndex, rhs.dstQueueFamilyIndex, 
      rhs.image, rhs.subresourceRange )
  {}

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

  CommandBuffer::CommandBuffer( const std::shared_ptr<CommandPool>& cmdPool, 
    vk::CommandBufferLevel level )
    : _commandPool( cmdPool )
  {
    vk::CommandBufferAllocateInfo info( *_commandPool, level, 1 );
    std::vector<vk::CommandBuffer> commandBuffers =
      static_cast< vk::Device >( *_commandPool->getDevice( ) )
        .allocateCommandBuffers( info );
    assert( !commandBuffers.empty( ) );
    _commandBuffer = commandBuffers[ 0 ];

    _isRecording = false;
  }
  CommandBuffer::~CommandBuffer( void )
  {
  }

  void CommandBuffer::clearAttachments( 
    vk::ArrayProxy<const vk::ClearAttachment> attachments,
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
    renderPassBeginInfo.pClearValues = 
      reinterpret_cast< vk::ClearValue const* >( clearValues.data( ) );

    _commandBuffer.beginRenderPass( renderPassBeginInfo, contents );
  }
  void CommandBuffer::endRenderPass( )
  {
    _renderPass.reset( );
    _framebuffer.reset( );

    _commandBuffer.endRenderPass( );
  }

//#ifdef VK_HEADER_VERSION >= 46
  void CommandBuffer::pushDescriptorSetKHR(
    vk::PipelineBindPoint pipelineBindPoint,
    const std::shared_ptr<PipelineLayout>& pipelineLayout, uint32_t firstSet,
    vk::ArrayProxy<WriteDescriptorSet> descriptorWrites )
  {
    std::vector<std::unique_ptr<vk::DescriptorImageInfo>> diis;
    diis.reserve( descriptorWrites.size( ) );

    std::vector<std::unique_ptr<vk::DescriptorBufferInfo>> dbis;
    dbis.reserve( descriptorWrites.size( ) );

    std::vector<vk::WriteDescriptorSet> writes;
    writes.reserve( descriptorWrites.size( ) );
    for ( const auto& w : descriptorWrites )
    {
      diis.push_back( std::unique_ptr<vk::DescriptorImageInfo>(
        w.imageInfo ? new vk::DescriptorImageInfo( 
          w.imageInfo->sampler ? *w.imageInfo->sampler : nullptr,
          w.imageInfo->imageView ? static_cast<vk::ImageView>( *w.imageInfo->imageView ) : nullptr,
          w.imageInfo->imageLayout )
        : nullptr ) );
      dbis.push_back( std::unique_ptr<vk::DescriptorBufferInfo>(
        w.bufferInfo ? new vk::DescriptorBufferInfo( w.bufferInfo->buffer ?
          static_cast<vk::Buffer>( *w.bufferInfo->buffer ) : nullptr,
          w.bufferInfo->offset, w.bufferInfo->range )
        : nullptr ) );
      vk::WriteDescriptorSet write(
        w.dstSet ? static_cast<vk::DescriptorSet>( *w.dstSet ) : nullptr,
        w.dstBinding,
        w.dstArrayElement,
        w.descriptorCount,
        w.descriptorType,
        diis.back( ).get( ),
        dbis.back( ).get( )
      );

      if ( w.texelBufferView )
      {
        vk::BufferView bufferView = static_cast< vk::BufferView >( *w.texelBufferView );
        write.setPTexelBufferView( &bufferView );
      }

      writes.push_back( std::move( write ) );
    }
    _commandBuffer.pushDescriptorSetKHR( pipelineBindPoint, *pipelineLayout, firstSet, writes );
  }
//#endif

  void CommandBuffer::blitImage( const std::shared_ptr<Image>& srcImage, 
    vk::ImageLayout srcImageLayout, const std::shared_ptr<Image>& dstImage, 
    vk::ImageLayout dstImageLayout, vk::ArrayProxy<const vk::ImageBlit> regions, 
    vk::Filter filter )
  {
    _commandBuffer.blitImage( *srcImage, srcImageLayout, *dstImage, 
      dstImageLayout, regions, filter );
  }

  void CommandBuffer::bindDescriptorSets(
    vk::PipelineBindPoint pipelineBindPoint, 
    const std::shared_ptr<PipelineLayout>& pipelineLayout, uint32_t firstSet, 
    vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> descriptorSets, 
    vk::ArrayProxy<const uint32_t> dynamicOffsets)
  {
    _bindDescriptorSets.clear( );
    for( auto& descriptor : descriptorSets )
    {
      _bindDescriptorSets.push_back(*descriptor);
    }

    _commandBuffer.bindDescriptorSets(pipelineBindPoint, *pipelineLayout, 
      firstSet, _bindDescriptorSets, dynamicOffsets);
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
  void CommandBuffer::setBlendConstants( const float blendConst[ 4 ] )
  {
    _commandBuffer.setBlendConstants( blendConst );
  }
  void CommandBuffer::beginOcclusionQuery( vk::QueryPool queryPool,
    uint32_t query, vk::QueryControlFlags flags )
  {
    _commandBuffer.beginQuery( queryPool, query, flags );
  }
  void CommandBuffer::endOcclusionQuery( vk::QueryPool queryPool,
    uint32_t query )
  {
    _commandBuffer.endQuery( queryPool, query );
  }

  void CommandBuffer::executeCommands( 
    const std::vector<std::shared_ptr<lava::CommandBuffer>>& secondaryCmds )
  {
    std::vector< vk::CommandBuffer > v;
    std::transform( secondaryCmds.begin( ), secondaryCmds.end( ),
      std::back_inserter( v ), []( std::shared_ptr<lava::CommandBuffer> c )
    {
      return static_cast< vk::CommandBuffer >( *c );
    } );
    _commandBuffer.executeCommands( v );
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
  void CommandBuffer::bindGraphicsPipeline( const std::shared_ptr<Pipeline>& pipeline )
  {
    _commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, *pipeline );
  }
  void CommandBuffer::bindComputePipeline( const std::shared_ptr<Pipeline>& pipeline )
  {
    _commandBuffer.bindPipeline( vk::PipelineBindPoint::eCompute, *pipeline );
  }
  /*void CommandBuffer::bindDescriptorSets( vk::PipelineBindPoint pipelineBindPoint, 
    std::shared_ptr<PipelineLayout> const & pipelineLayout, uint32_t firstSet, 
    vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> descriptorSets, 
    vk::ArrayProxy<const uint32_t> dynamicOffsets )
  {
    _bindDescriptorSets.clear( );
    for ( auto & descriptor : descriptorSets )
    {
      _bindDescriptorSets.push_back( *descriptor );
    }

    _commandBuffer.bindDescriptorSets( pipelineBindPoint, *pipelineLayout, firstSet, _bindDescriptorSets, dynamicOffsets );
  }*/
  void CommandBuffer::setViewportScissors( uint32_t width, uint32_t height )
  {
    setScissor( 0, vk::Rect2D( { 0, 0 }, { width, height } ) );
    setViewport( 0, vk::Viewport( 0.0f, 0.0f, ( float ) width, ( float ) height, 0.0f, 1.0f ) );
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
  void CommandBuffer::setViewport( uint32_t first, 
    vk::ArrayProxy<const vk::Viewport> viewports )
  {
    _commandBuffer.setViewport( first, viewports );
  }
  void CommandBuffer::dispatch( uint32_t x, uint32_t y, uint32_t z )
  {
    _commandBuffer.dispatch( x, y, z );
  }
  void CommandBuffer::draw( uint32_t vertexCount, uint32_t instanceCount, 
    uint32_t firstVertex,
    uint32_t firstInstance )
  {
    _commandBuffer.draw( vertexCount, instanceCount, firstVertex, firstInstance );
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
    _commandBuffer.copyBufferToImage( *srcBuffer, *dstImage, dstImageLayout, regions );
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
    _commandBuffer.copyImageToBuffer( *srcImage, srcImageLayout, *dstBuffer, regions );
  }

  void CommandBuffer::bindVertexBuffer( uint32_t startBinding, 
    const std::shared_ptr<Buffer>& buffer, vk::DeviceSize offset )
  {
    _commandBuffer.bindVertexBuffers( startBinding, 
      static_cast< vk::Buffer >( *buffer ), offset );
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

    _commandBuffer.bindVertexBuffers( startBinding, _bindVertexBuffers, offsets );
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
  void CommandBuffer::reset( void )
  {
    _commandBuffer.reset( { } );
  }
  void CommandBuffer::beginSimple( vk::CommandBufferUsageFlags flags, vk::CommandBufferInheritanceInfo* inheritInfo )
  {
    assert( !_isRecording );
    vk::CommandBufferBeginInfo cbbi( flags, inheritInfo );

    _commandBuffer.begin( cbbi );
    _isRecording = true;
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


  void CommandBuffer::pipelineBarrier( vk::PipelineStageFlags srcStageMask, 
    vk::PipelineStageFlags destStageMask, vk::DependencyFlags dependencyFlags,
    vk::ArrayProxy<const vk::MemoryBarrier> barriers, 
    vk::ArrayProxy<const vk::BufferMemoryBarrier> bufferMemoryBarriers,
    vk::ArrayProxy<const ImageMemoryBarrier> imageMemoryBarriers )
  {
    std::vector<vk::ImageMemoryBarrier> imbs;
    imbs.reserve( imageMemoryBarriers.size( ) );
    for ( auto const& imb : imageMemoryBarriers )
    {
      imbs.push_back( vk::ImageMemoryBarrier( imb.srcAccessMask, imb.dstAccessMask, 
        imb.oldLayout, imb.newLayout, imb.srcQueueFamilyIndex, imb.dstQueueFamilyIndex,
        imb.image ? static_cast<vk::Image>( *imb.image ) : nullptr, imb.subresourceRange ) );
    }

    _commandBuffer.pipelineBarrier( srcStageMask, destStageMask, dependencyFlags,
      barriers, bufferMemoryBarriers, imbs );
  }
}