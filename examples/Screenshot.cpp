/**
 * Copyright (c) 2017, Lava
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

#include <lava/lava.h>
using namespace lava;

#include <routes.h>

struct
{
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;
} uboVS;

class ScreenshotApp : public VulkanApp
{
public:
  std::shared_ptr<Pipeline> pipeline;

  std::shared_ptr<Buffer> uniformMVP;
  std::shared_ptr<PipelineLayout> pipelineLayout;
  std::shared_ptr<DescriptorSet> descriptorSet;

  std::shared_ptr<lava::extras::Geometry> geometry;
  std::shared_ptr<CommandPool> commandPool;

  void saveScreenshot( const char* filename, uint32_t width, uint32_t height )
  {
    // Get format properties for the swapchain color format
    vk::FormatProperties formatProps;

    bool supportsBlit = true;
    // Check blit support for source and destination

    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    formatProps = _device->_physicalDevice->getFormatProperties( _colorFormat );
    if ( !( formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc ) )
    {
      std::cerr << "Device does not support blitting from optimal tiled images, "
        << "using copy instead of blit!" << std::endl;
      supportsBlit = false;
    }

    // Check if the device supports blitting to linear images
    formatProps = _device->_physicalDevice->getFormatProperties( vk::Format::eR8G8B8A8Unorm );
    if ( !( formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst ) ) {
      std::cerr << "Device does not support blitting to linear tiled images, "
        << "using copy instead of blit!" << std::endl;
      supportsBlit = false;
    }

    // Source for the copy is the last rendered swapchain image
    auto srcImage = _defaultFramebuffer->getLastImage( );

    // Create the linear tiled destination image to copy to and to read the memory from
    std::shared_ptr<lava::Image> dstImage = _device->createImage( { },
      vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm,
      vk::Extent3D( width, height, 1 ), 1, 1, vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eTransferDst, 
      vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined, 
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );

    // Do the actual blit from the swapchain image to our host visible destination image
    std::shared_ptr<CommandBuffer> copyCmd = commandPool->allocateCommandBuffer( );

    copyCmd->beginSimple( );

    // Transition destination image to transfer destination layout
    lava::utils::insertImageMemoryBarrier( copyCmd, dstImage, { },
      vk::AccessFlagBits::eTransferWrite, 
      vk::ImageLayout::eUndefined, 
      vk::ImageLayout::eTransferDstOptimal, 
      vk::PipelineStageFlagBits::eTransfer, 
      vk::PipelineStageFlagBits::eTransfer, 
      vk::ImageSubresourceRange( 
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      )
    );
    // Transition swapchain image from present to transfer source layout
    lava::utils::insertImageMemoryBarrier( copyCmd, srcImage, 
      vk::AccessFlagBits::eMemoryRead,
      vk::AccessFlagBits::eTransferRead, 
      vk::ImageLayout::ePresentSrcKHR,
      vk::ImageLayout::eTransferSrcOptimal,
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer,
      vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      )
    );

    // If source and destination support blit we'll blit as this also does automatic format conversion (e.g. from BGR to RGB)
    if ( supportsBlit )
    {
      // Define the region to blit (we will blit the whole swapchain image)
      vk::Offset3D blitSize;
      blitSize.x = width;
      blitSize.y = height;
      blitSize.z = 1;
      vk::ImageBlit imageBlitRegion{ };
      imageBlitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageBlitRegion.srcSubresource.layerCount = 1;
      imageBlitRegion.srcOffsets[ 1 ] = blitSize;
      imageBlitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageBlitRegion.dstSubresource.layerCount = 1;
      imageBlitRegion.dstOffsets[ 1 ] = blitSize;

      // Issue the blit command
      copyCmd->blitImage( 
        srcImage, vk::ImageLayout::eTransferSrcOptimal,
        dstImage, vk::ImageLayout::eTransferDstOptimal, 
        { imageBlitRegion }, vk::Filter::eNearest
      );
    }
    else
    {
      // Otherwise use image copy (requires us to manually flip components)
      vk::ImageCopy imageCopyRegion{ };
      imageCopyRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageCopyRegion.srcSubresource.layerCount = 1;
      imageCopyRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
      imageCopyRegion.dstSubresource.layerCount = 1;
      imageCopyRegion.extent.width = width;
      imageCopyRegion.extent.height = height;
      imageCopyRegion.extent.depth = 1;

      // Issue the copy command
      copyCmd->copyImage( 
        srcImage, vk::ImageLayout::eTransferSrcOptimal,
        dstImage, vk::ImageLayout::eTransferDstOptimal,
        { imageCopyRegion }
      );
    }
    // Transition destination image to general layout, which is the required layout for mapping the image memory later on
    lava::utils::insertImageMemoryBarrier(
      copyCmd,
      dstImage,
      vk::AccessFlagBits::eTransferWrite,
      vk::AccessFlagBits::eMemoryRead,
      vk::ImageLayout::eTransferDstOptimal,
      vk::ImageLayout::eGeneral,
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer,
      vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      )
    );

    // Transition back the swap chain image after the blit is done
    lava::utils::insertImageMemoryBarrier(
      copyCmd,
      srcImage,
      vk::AccessFlagBits::eTransferRead,
      vk::AccessFlagBits::eMemoryRead,
      vk::ImageLayout::eTransferSrcOptimal,
      vk::ImageLayout::ePresentSrcKHR,
      vk::PipelineStageFlagBits::eTransfer,
      vk::PipelineStageFlagBits::eTransfer,
      vk::ImageSubresourceRange(
        vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1
      )
    );

    copyCmd->end( );

    _graphicsQueue->submitAndWait( copyCmd );

    // Get layout of the image (including row pitch)
    vk::ImageSubresource isr;
    isr.aspectMask = vk::ImageAspectFlagBits::eColor;
    vk::SubresourceLayout subResourceLayout;

    vk::Device device = static_cast< vk::Device > ( *_device );

    device.getImageSubresourceLayout(
      static_cast< vk::Image >( *dstImage ), &isr, &subResourceLayout
    );

    // Map image memory so we can start copying from it
    const char* data = ( const char* ) device.mapMemory( dstImage->imageMemory, 0, VK_WHOLE_SIZE, { } );
    data += subResourceLayout.offset;

    std::ofstream file( filename, std::ios::out | std::ios::binary );

    // ppm header
    file << "P6\n" << width << "\n" << height << "\n" << 255 << "\n";

    // If source is BGR (destination is always RGB) and we can't use blit (which does automatic conversion), we'll have to manually swizzle color components
    bool colorSwizzle = false;
    // Check if source is BGR 
    // Note: Not complete, only contains most common and basic BGR surface formats for demonstation purposes
    if ( !supportsBlit )
    {
      std::vector< vk::Format > formatsBGR = { 
        vk::Format::eB8G8R8A8Srgb, vk::Format::eB8G8R8A8Unorm, 
        vk::Format::eB8G8R8A8Snorm
      };
      colorSwizzle = ( std::find( formatsBGR.begin( ), formatsBGR.end( ), 
        _colorFormat ) != formatsBGR.end( ) );
    }

    // ppm binary pixel data
    for ( uint32_t y = 0; y < height; ++y )
    {
      unsigned int* row = ( unsigned int* ) data;
      for ( uint32_t x = 0; x < width; ++x )
      {
        if ( colorSwizzle )
        {
          file.write( ( char* ) row + 2, 1 );
          file.write( ( char* ) row + 1, 1 );
          file.write( ( char* ) row, 1 );
        }
        else
        {
          file.write( ( char* ) row, 3 );
        }
        row++;
      }
      data += subResourceLayout.rowPitch;
    }
    file.close( );

    std::cout << "Screenshot saved to disk" << std::endl;
  }

  ScreenshotApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    // create a command pool for command buffer allocation
    commandPool = _device->createCommandPool( 
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    geometry = std::make_shared<lava::extras::Geometry>( _device,
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "bunny.obj_" ) );

    // MVP buffer
    {
      uint32_t mvpBufferSize = sizeof( uboVS );
      uniformMVP = _device->createBuffer( mvpBufferSize, 
        vk::BufferUsageFlagBits::eUniformBuffer, 
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible | 
          vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs = 
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };
    auto descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );

    pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );

    // init pipeline
    auto vertexStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "perfectToon_vert.spv" ), 
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = _device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "perfectToon_frag.spv" ), 
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 
            0, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof( lava::extras::Vertex, position )
          ),
          vk::VertexInputAttributeDescription( 
            1, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof( lava::extras::Vertex, normal )
          ),
          vk::VertexInputAttributeDescription( 
            2, 0, vk::Format::eR32G32Sfloat, 
            offsetof( lava::extras::Vertex, texCoord )
          )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, 
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 ); 
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, 
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 
      0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true, 
      vk::CompareOp::eLessOrEqual, false, false, stencilOpState, 
      stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
      vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
      vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, 
      colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( {
      vk::DynamicState::eViewport, vk::DynamicState::eScissor
    } );

    pipeline = _device->createGraphicsPipeline( pipelineCache, { }, 
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport, 
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _renderPass );

    std::array<vk::DescriptorPoolSize, 1> poolSize = 
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 )
    };
    auto descriptorPool = _device->createDescriptorPool( { }, 1, poolSize );

    // Init descriptor set
    descriptorSet = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( uniformMVP, 0, sizeof( uboVS ) )
      )
    };
    _device->updateDescriptorSets( wdss, { } );

  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    uboVS.model = glm::mat4( 1.0f );
    uboVS.model = glm::rotate( uboVS.model, time * glm::radians( 90.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 5.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );

    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uniformMVP->writeData( 0, sizeof( uboVS ), &uboVS );
  }

  void doPaint( void ) override
  {
    updateUniformBuffers( );

    auto commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), 
      vk::Rect2D( { 0, 0 }, 
      _defaultFramebuffer->getExtent( ) ),
      { vk::ClearValue( ccv ), vk::ClearValue( 
        vk::ClearDepthStencilValue( 1.0f, 0 ) )
      }, vk::SubpassContents::eInline );

    commandBuffer->bindGraphicsPipeline( pipeline );
    
    commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );
    
    commandBuffer->setViewportScissors( _defaultFramebuffer->getExtent( ) );
    
    geometry->render( commandBuffer );
    commandBuffer->endRenderPass( );

    commandBuffer->end( );

    _graphicsQueue->submit( SubmitInfo{
      { _defaultFramebuffer->getPresentSemaphore( ) },
      { vk::PipelineStageFlagBits::eColorAttachmentOutput },
      commandBuffer,
      _renderComplete
    } );
  }
  void keyEvent( int key, int scancode, int action, int mods )
  {
    switch ( key )
    {
    case GLFW_KEY_ESCAPE:
      switch ( action )
      {
      case GLFW_PRESS:
      {
        uint32_t width = _window->getWidth( );
        uint32_t height = _window->getHeight( );
        saveScreenshot( "screenshot.ppm", width, height );
        getWindow( )->close( );
        break;
      }
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
};

void glfwErrorCallback( int error, const char* description )
{
  fprintf( stderr, "GLFW Error %d: %s\n", error, description );
}

int main( int argc, char** argv )
{
  try
  {
    VulkanApp* app = new ScreenshotApp( "Perfect Toon", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      //app->waitEvents( );
      app->paint( );
    }

    delete app;
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  return 0;
}