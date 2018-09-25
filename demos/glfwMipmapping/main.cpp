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

#include <iostream>

#include <glfwPompeii/glfwPompeii.h>
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;

#include <routes.h>

class MipmapTexture2D : public pompeii::Texture
{
public:
  MipmapTexture2D( const std::shared_ptr<Device>& device, 
    const std::string& filename, vk::Format format, 
    const std::shared_ptr<CommandPool>& cmdPool,
    const std::shared_ptr<Queue>& queue )
    : pompeii::Texture( device )
  {
    unsigned int channels;
    unsigned char* pixels = pompeii::utils::loadImageTexture(
      filename, width, height, channels );
    channels = 4;

    vk::FormatProperties formatProperties;

    // calculate num of mip maps
    // numLevels = 1 + floor(log2(max(w, h, d)))
    // Calculated as log2(max(width, height, depth))c + 1 (see specs)
    mipLevels = floor( log2( std::max( width, height ) ) ) + 1;

    // Get device properites for the requested texture format
    formatProperties = _device->getPhysicalDevice( )->getFormatProperties( format );

    // Mip-chain generation requires support for blit source and destination
    assert( formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc );
    assert( formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst );

    vk::DeviceSize texSize = width * height * channels;

    // Create a host-visible staging buffer that contains the raw image data
    std::shared_ptr<Buffer> stagingBuffer = _device->createBuffer( texSize,
      vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive, { },
      vk::MemoryPropertyFlagBits::eHostVisible |
      vk::MemoryPropertyFlagBits::eHostCoherent );

    // Copy texture data into staging buffer
    stagingBuffer->writeData( 0, texSize, pixels );

    free( pixels );

    // Create optimal tiled target image
    image = _device->createImage( { }, vk::ImageType::e2D, format,
      vk::Extent3D( width, height, 1 ), mipLevels, 1,
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, 
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | 
        vk::ImageUsageFlagBits::eSampled,
      vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined,
      vk::MemoryPropertyFlagBits::eDeviceLocal );

    auto copyCmd = cmdPool->allocateCommandBuffer( );
    copyCmd->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

    vk::ImageSubresourceRange subresourceRange;
    subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresourceRange.levelCount = 1;
    subresourceRange.layerCount = 1;

    // Optimal image will be used as destination for the copy, so we must transfer 
    //  from our initial undefined image layout to the transfer destination layout
    utils::transitionImageLayout(
      copyCmd,
      image,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eTransferDstOptimal,
      subresourceRange
    );

    // Copy the first mip of the chain, remaining mips will be generated
    vk::BufferImageCopy bufferCopyRegion;
    bufferCopyRegion.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    bufferCopyRegion.imageSubresource.mipLevel = 0;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount = 1;
    bufferCopyRegion.imageExtent.width = width;
    bufferCopyRegion.imageExtent.height = height;
    bufferCopyRegion.imageExtent.depth = 1;

    copyCmd->copyBufferToImage( stagingBuffer, 
      image, vk::ImageLayout::eTransferDstOptimal, bufferCopyRegion );

    // Transition first mip level to transfer source for read during blit
    imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Transition first mip level to transfer source for read during blit
    utils::transitionImageLayout(
      copyCmd,
      image,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eTransferSrcOptimal,
      subresourceRange );

    // Send command buffer
    copyCmd->end( );

    queue->submitAndWait( copyCmd );

    // Clean up staging resources
    stagingBuffer.reset( );


    auto blitCmd = cmdPool->allocateCommandBuffer( );
    blitCmd->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

    ImageMemoryBarrier barrier;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    for ( uint32_t i = 1; i < mipLevels; i++ )
    {
      barrier.subresourceRange.baseMipLevel = i - 1;
      barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
      barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;



      blitCmd->pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
        { }, { }, { },
        barrier );

      vk::ImageBlit blit;
      blit.srcOffsets[ 0 ] = { 0, 0, 0 };
      blit.srcOffsets[ 1 ] = { mipWidth, mipHeight, 1 };
      blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      blit.srcSubresource.mipLevel = i - 1;
      blit.srcSubresource.baseArrayLayer = 0;
      blit.srcSubresource.layerCount = 1;
      blit.dstOffsets[ 0 ] = { 0, 0, 0 };
      blit.dstOffsets[ 1 ] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
      blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      blit.dstSubresource.mipLevel = i;
      blit.dstSubresource.baseArrayLayer = 0;
      blit.dstSubresource.layerCount = 1;

      blitCmd->blitImage( image, vk::ImageLayout::eTransferSrcOptimal,
        image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear );

      barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
      barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
      barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;


      blitCmd->pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
        { }, { }, { },
        barrier );

      if ( mipWidth > 1 ) mipWidth /= 2;
      if ( mipHeight > 1 ) mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    

    blitCmd->pipelineBarrier(
      vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, 
      { }, { }, { },
      barrier );

    // Generate the mip chain
    // ---------------------------------------------------------------
    // We copy down the whole mip chain doing a blit from mip-1 to mip
    // An alternative way would be to always blit from the first mip level 
    //  and sample that one down
    /*auto blitCmd = cmdPool->allocateCommandBuffer( );
    blitCmd->begin( vk::CommandBufferUsageFlagBits::eOneTimeSubmit );

    // Copy down mips from n-1 to n
    for ( uint32_t i = 1; i < mipLevels; ++i )
    {
      vk::ImageBlit imageBlit;

      // Source
      imageBlit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageBlit.srcSubresource.layerCount = 1;
      imageBlit.srcSubresource.mipLevel = i - 1;
      imageBlit.srcOffsets[ 1 ].x = int32_t( width >> ( i - 1 ) );
      imageBlit.srcOffsets[ 1 ].y = int32_t( height >> ( i - 1 ) );
      imageBlit.srcOffsets[ 1 ].z = 1;

      // Destination
      imageBlit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageBlit.dstSubresource.layerCount = 1;
      imageBlit.dstSubresource.mipLevel = i;
      imageBlit.dstOffsets[ 1 ].x = int32_t( width >> i );
      imageBlit.dstOffsets[ 1 ].y = int32_t( height >> i );
      imageBlit.dstOffsets[ 1 ].z = 1;

      vk::ImageSubresourceRange mipSubRange;
      mipSubRange.aspectMask = vk::ImageAspectFlagBits::eColor;
      mipSubRange.baseMipLevel = i;
      mipSubRange.levelCount = 1;
      mipSubRange.layerCount = 1;

      // Transiton current mip level to transfer dest
      utils::transitionImageLayout(
        blitCmd,
        image,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        mipSubRange,
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eHost );

      // Blit from previous level
      blitCmd->blitImage( 
        image, vk::ImageLayout::eTransferSrcOptimal, 
        image, vk::ImageLayout::eTransferDstOptimal, 
        imageBlit, vk::Filter::eLinear );

      // Transiton current mip level to transfer source for read in next iteration
      utils::transitionImageLayout(
        blitCmd,
        image,
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eTransferSrcOptimal,
        mipSubRange,
        vk::PipelineStageFlagBits::eHost,
        vk::PipelineStageFlagBits::eTransfer );
    }
    // Send command buffer
    blitCmd->end( );

    queue->submitAndWait( blitCmd );*/

    /*utility::MipMapGenerationCmdBuffer* mmg =
      new utility::MipMapGenerationCmdBuffer( cmdPool, image, width, height, mipLevels, 
        vk::ImageLayout::eUndefined, vk::AccessFlags( ), vk::PipelineStageFlagBits::eTransfer,
        vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits::eShaderRead, vk::PipelineStageFlagBits::eFragmentShader );

    delete mmg;*/

    sampler = _device->createSampler( vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat,
      vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
      0.0f, true, 1.0f, false, vk::CompareOp::eAlways, 
      0.0f, VK_LOD_CLAMP_NONE, //static_cast<float>( mipLevels ),
      vk::BorderColor::eFloatOpaqueWhite, false );

    // Create image view
    view = image->createImageView( vk::ImageViewType::e2D, format );

    updateDescriptor( );
  }
};

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  virtual void initResources( void ) override
  {
    auto device = _window->device( );

    tex = std::shared_ptr<MipmapTexture2D>( new MipmapTexture2D( device, POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "T_Pedestal_512.png" ), vk::Format::eR8G8B8A8Unorm, _window->gfxCommandPool( ),
      _window->gfxQueue( ) ) );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );
    pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

    std::shared_ptr<DescriptorPool> descriptorPool =
      device->createDescriptorPool( 1, {
        { vk::DescriptorType::eCombinedImageSampler, 1 }
      } );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        tex->descriptor, nullptr
      )
    };
    _window->device( )->updateDescriptorSets( wdss, { } );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fullquad_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "fullquad_mipmap_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput( { }, { } );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { },
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f
    );
    PipelineMultisampleStateCreateInfo multisample(
      vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false
    );

    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
      vk::StencilOp::eKeep, vk::StencilOp::eKeep,
      vk::CompareOp::eAlways, 0, 0, 0
    );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { },
      true, true, vk::CompareOp::eLessOrEqual, false, false,
      stencilOpState, stencilOpState, 0.0f, 0.0f
    );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );
    PipelineColorBlendStateCreateInfo colorBlend( false,
      vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
    );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( ) );
  }

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;

    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( 0.0f, 0.0f, 0.0f );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );

    cmd->setViewportScissors( extent );
    cmd->draw( 4, 1, 0, 0 );

    cmd->endRenderPass( );

    _window->frameReady( );
  }

  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< Texture > tex;
};

class VulkanWindow : public glfw::VulkanWindow
{
public:
  explicit VulkanWindow( int width, int height,
    const std::string& title, bool enableLayers )
    : glfw::VulkanWindow( width, height, title, enableLayers )
  {
  }
  virtual glfw::VulkanWindowRenderer* createRenderer( void ) override
  {
    return new MainWindowRenderer( this );
  }
};


int main( int, char** )
{
  VulkanWindow app( 500, 500, "GLFWRenderer", true );
  app.show( );
  return EXIT_SUCCESS;
}