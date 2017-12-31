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
#include "utils/Camera.h"

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    camera = Camera( glm::vec3( 0.0f, 2.0f, 8.0f ) );
  }

  const unsigned int SCR_WIDTH = 800;
  const unsigned int SCR_HEIGHT = 600;

  // camera
  Camera camera;
  // timing
  float deltaTime = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;

  float lastX = SCR_WIDTH / 2.0f;
  float lastY = SCR_HEIGHT / 2.0f;
  bool firstMouse = true;

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  struct
  {
    glm::vec3 viewPos;
  } uboFS;

  struct Vertex
  {
    glm::vec3 pos;
  };

  const float side = 5.0f;
  const float side2 = side / 2.0f;
  const std::vector<Vertex> vertices =
  {
    { { -side2, -side2,  side2 } },
    { {  side2, -side2,  side2 } },
    { { -side2,  side2,  side2 } },
    { {  side2,  side2,  side2 } },

    { { -side2, -side2, -side2 } },
    { {  side2, -side2, -side2 } },
    { { -side2,  side2, -side2 } },
    { {  side2,  side2, -side2 } },

    { {  side2, -side2, -side2 } },
    { {  side2, -side2,  side2 } },
    { {  side2,  side2, -side2 } },
    { {  side2,  side2,  side2 } },

    { { -side2, -side2, -side2 } },
    { { -side2, -side2,  side2 } },
    { { -side2,  side2, -side2 } },
    { { -side2,  side2,  side2 } },

    { { -side2,  side2, -side2 } },
    { { -side2,  side2,  side2 } },
    { {  side2,  side2, -side2 } },
    { {  side2,  side2,  side2 } },

    { { -side2, -side2, -side2 } },
    { { -side2, -side2,  side2 } },
    { {  side2, -side2, -side2 } },
    { {  side2, -side2,  side2 } }
  };
  const std::vector<uint16_t> indices =
  {
     0,  1,  2,     1,  3,  2,
     4,  6,  5,     5,  6,  7,
     8, 10,  9,     9, 10, 11,
    12, 13, 14,    13, 15, 14,
    16, 17, 18,    17, 19, 18,
    20, 22, 21,    21, 22, 23,
  };

  void initResources( void ) override
  {
    auto device = _window->device( );

    // Vertex buffer
    {
      uint32_t vertexBufferSize = vertices.size( ) * sizeof( Vertex );
      auto stagingBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive,
        nullptr, vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, vertexBufferSize, vertices.data( ) );

      skybox.vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive,
        nullptr, vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );
      cmd->beginSimple( );
      stagingBuffer->copy( cmd, skybox.vertexBuffer, 0, 0, vertexBufferSize );
      cmd->end( );

      _window->graphicQueue( )->submitAndWait( cmd );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );

      auto stagingBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive,
        nullptr, vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent );
      stagingBuffer->writeData( 0, indexBufferSize, indices.data( ) );

      skybox.indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive,
        nullptr, vk::MemoryPropertyFlagBits::eDeviceLocal );

      auto cmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );
      cmd->beginSimple( );
      stagingBuffer->copy( cmd, skybox.indexBuffer, 0, 0, indexBufferSize );
      cmd->end( );

      _window->graphicQueue( )->submitAndWait( cmd );
    }

    // MVP buffer
    uniformMVP = device->createUniformBuffer( sizeof( uboVS ) );

    uniformViewPos = device->createUniformBuffer( sizeof( uboFS ) );

    geometry = std::make_shared<lava::extras::Geometry>( device,
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "wolf.obj_" ) );

    std::array< std::string, 6 > cubeImages =
    {
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/skyCubemap/right.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/skyCubemap/left.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/skyCubemap/top.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/skyCubemap/bottom.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/skyCubemap/back.jpg" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/skyCubemap/front.jpg" ),
    };
    tex = std::make_shared<TextureCubemap>( device, cubeImages,
      _window->graphicsCommandPool( ), _window->graphicQueue( ),
      vk::Format::eR8G8B8A8Unorm );


    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 3 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 )
    };

    descriptorPool = device->createDescriptorPool( 2, poolSize );

    {
      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex
        ),
        DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        ),
        DescriptorSetLayoutBinding( 2, vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eFragment
        ),
      };
      auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

      model.pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

      // init pipeline
      auto vertexStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "reflect_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "reflect_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput(
        vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
          vk::VertexInputRate::eVertex ),
          {
            vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat,
            offsetof( lava::extras::Vertex, position )
            ),
        vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat,
          offsetof( lava::extras::Vertex, normal )
        )
          }
      );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {},
        vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( 1, 1 );
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
      PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
        false, 0.0f, nullptr, false, false );
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
        0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true,
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState,
        0.0f, 0.0f );
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


      model.pipelines.reflect = device->createGraphicsPipeline(
        _window->pipelineCache, {}, { vertexStage, fragmentStage },
        vertexInput, assembly, nullptr, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic, model.pipelineLayout,
        _window->defaultRenderPass( ) );


      vertexStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "refract_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      fragmentStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "refract_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      model.pipelines.refract = device->createGraphicsPipeline(
        _window->pipelineCache, {}, { vertexStage, fragmentStage },
        vertexInput, assembly, nullptr, viewport, rasterization, multisample,
        depthStencil, colorBlend, dynamic, model.pipelineLayout,
        _window->defaultRenderPass( ) );

      // Init descriptor set
      model.descriptorSet = device->allocateDescriptorSet( descriptorPool, 
        descriptorSetLayout );
      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet( model.descriptorSet, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo( uniformMVP, 0, sizeof( uboVS ) )
        ),
        WriteDescriptorSet( model.descriptorSet, 1, 0,
          vk::DescriptorType::eCombinedImageSampler, 1,
          tex->descriptor, nullptr
        ),
        WriteDescriptorSet( model.descriptorSet, 2, 0,
          vk::DescriptorType::eUniformBuffer, 1, nullptr,
          DescriptorBufferInfo( uniformViewPos, 0, sizeof( uboFS ) )
        ),
      };
      device->updateDescriptorSets( wdss, {} );
    }

    {
      // Init descriptor and pipeline layouts
      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex ),
        DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment )
      };
      auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

      skybox.pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

      // init pipeline
      auto vertexStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "skybox_vert.spv" ), 
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "skybox_frag.spv" ), 
        vk::ShaderStageFlagBits::eFragment
      );
      vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ), 
        vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding, {
        vk::VertexInputAttributeDescription( 0, 0, 
        vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos ) )
      } );
      vk::PipelineInputAssemblyStateCreateInfo assembly( {}, 
        vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( 1, 1 );
      vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
      PipelineMultisampleStateCreateInfo multisample( 
        vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, 
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
        0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, 
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


      skybox.pipeline = device->createGraphicsPipeline( _window->pipelineCache, 
        { }, { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        skybox.pipelineLayout, _window->defaultRenderPass( ) );

      // Init descriptor set
      skybox.descriptorSet = device->allocateDescriptorSet( 
        descriptorPool, descriptorSetLayout );
      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet( skybox.descriptorSet, 0, 0,
          vk::DescriptorType::eUniformBuffer, 1, nullptr,
          DescriptorBufferInfo( uniformMVP, 0, sizeof( uboVS ) )
        ),
        WriteDescriptorSet( skybox.descriptorSet, 1, 0, 
          vk::DescriptorType::eCombinedImageSampler, 1,
          DescriptorImageInfo(
            vk::ImageLayout::eGeneral,
            std::make_shared<vk::ImageView>( tex->view ),
            std::make_shared<vk::Sampler>( tex->sampler )
          ), nullptr
        )
      };

      device->updateDescriptorSets( wdss, {} );
    }
  }

  void updateMVP( void )
  {
    auto size = _window->getExtent( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( 
      currentTime - startTime ).count( ) / 1000.0f;

    float currentFrame = time;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 25.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ) );
    uboVS.view = camera.GetViewMatrix( );
    uboVS.proj = glm::perspective( glm::radians( camera.Zoom ), 
      ( float ) width / ( float ) height, 0.1f, 100.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uniformMVP->writeData( 0, sizeof( uboVS ), &uboVS );

    uboFS.viewPos = camera.Position;
    uniformViewPos->writeData( 0, sizeof( uboFS ), &uboFS );
  }

  bool modeReflect = true;

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    const glm::ivec2 size = _window->swapChainImageSize( );

    if ( Input::isKeyPressed( lava::Keyboard::Key::Z ) )
    {
      modeReflect = false;
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::X ) )
    {
      modeReflect = true;
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::Space ) )
    {
      saveScreenshot( "file.ppm", size.x, size.y );
    }

    // Mouse event
    {
      if ( Input::MouseButtonPress( MouseButton::Left ) )
      {
        int xPos = Input::MouseX( );
        int yPos = Input::MouseY( );
        if ( firstMouse )
        {
          lastX = xPos;
          lastY = yPos;
          firstMouse = false;
        }

        float xoffset = xPos - lastX;
        float yoffset = lastY - yPos; // reversed since y-coordinates go from bottom to top

        lastX = xPos;
        lastY = yPos;

        camera.ProcessMouseMovement( xoffset, yoffset );
      }
      else if ( Input::MouseButtonRelease( MouseButton::Left ) )
      {
        firstMouse = true;
      }
    }

    updateMVP( );
    
    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    cmd->bindGraphicsPipeline( skybox.pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      skybox.pipelineLayout, 0, { skybox.descriptorSet }, nullptr );
    cmd->bindVertexBuffer( 0, skybox.vertexBuffer, 0 );
    cmd->bindIndexBuffer( skybox.indexBuffer, 0, vk::IndexType::eUint16 );
    cmd->setViewportScissors( _window->getExtent( ) );

    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

    if ( modeReflect )
    {
      cmd->bindGraphicsPipeline( model.pipelines.reflect );
    }
    else
    {
      cmd->bindGraphicsPipeline( model.pipelines.refract );
    }
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      model.pipelineLayout, 0, { model.descriptorSet }, nullptr );

    geometry->render( cmd );

    cmd->endRenderPass( );

    _window->frameReady( );
  }

private:
  VulkanWindow *_window;
  struct
  {
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;
    std::shared_ptr<Pipeline> pipeline;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
  } skybox;

  struct
  {
    struct
    {
      std::shared_ptr<Pipeline> refract;
      std::shared_ptr<Pipeline> reflect;
    } pipelines;
    std::shared_ptr<PipelineLayout> pipelineLayout;
    std::shared_ptr<DescriptorSet> descriptorSet;
  } model;

  std::shared_ptr<TextureCubemap> tex;
  std::shared_ptr<DescriptorPool> descriptorPool;

  std::shared_ptr<lava::extras::Geometry> geometry;
  std::shared_ptr<Buffer> uniformMVP;
  std::shared_ptr<Buffer> uniformViewPos;


  void saveScreenshot( const char* filename, uint32_t width, uint32_t height )
  {
    auto device = _window->device( );
    // Get format properties for the swapchain color format
    vk::FormatProperties formatProps;

    bool supportsBlit = true;
    // Check blit support for source and destination

    // Check if the device supports blitting from optimal images (the swapchain images are in optimal format)
    formatProps = _window->physicalDevice( )->getFormatProperties( _window->colorFormat( ) );
    if ( !( formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc ) )
    {
      std::cerr << "Device does not support blitting from optimal tiled images, "
        << "using copy instead of blit!" << std::endl;
      supportsBlit = false;
    }

    // Check if the device supports blitting to linear images
    formatProps = _window->physicalDevice( )->getFormatProperties( vk::Format::eR8G8B8A8Unorm );
    if ( !( formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst ) ) {
      std::cerr << "Device does not support blitting to linear tiled images, "
        << "using copy instead of blit!" << std::endl;
      supportsBlit = false;
    }

    // Source for the copy is the last rendered swapchain image
    auto srcImage = _window->defaultFramebuffer( )->getLastImage( );

    // Create the linear tiled destination image to copy to and to read the memory from
    std::shared_ptr<lava::Image> dstImage = device->createImage( {},
      vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm,
      vk::Extent3D( width, height, 1 ), 1, 1, vk::SampleCountFlagBits::e1,
      vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eTransferDst,
      vk::SharingMode::eExclusive, {}, vk::ImageLayout::eUndefined,
      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    );

    // Do the actual blit from the swapchain image to our host visible destination image
    auto copyCmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );

    copyCmd->beginSimple( );

    // Transition destination image to transfer destination layout
    lava::utils::insertImageMemoryBarrier( copyCmd, dstImage, {},
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
      vk::ImageBlit imageBlitRegion{};
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
      vk::ImageCopy imageCopyRegion{};
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

    _window->graphicQueue( )->submitAndWait( copyCmd );

    // Get layout of the image (including row pitch)
    vk::ImageSubresource isr;
    isr.aspectMask = vk::ImageAspectFlagBits::eColor;
    vk::SubresourceLayout subResourceLayout;

    vk::Device dev = static_cast< vk::Device > ( *device );

    dev.getImageSubresourceLayout(
      static_cast< vk::Image >( *dstImage ), &isr, &subResourceLayout
    );

    // Map image memory so we can start copying from it
    const char* data = ( const char* ) dev.mapMemory( dstImage->imageMemory, 0, VK_WHOLE_SIZE, {} );
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
        _window->colorFormat( ) ) != formatsBGR.end( ) );
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

};

class CustomVkWindow : public VulkanWindow
{
public:
  VulkanWindowRenderer* createRenderer( void ) override
  {
    return new CustomRenderer( this );
  }
};

int main( void )
{
  std::shared_ptr<Instance> instance;

  // Create instance
  vk::ApplicationInfo appInfo(
    "App Name",
    VK_MAKE_VERSION( 1, 0, 0 ),
    "FooEngine",
    VK_MAKE_VERSION( 1, 0, 0 ),
    VK_API_VERSION_1_0
  );


  std::vector<const char*> layers =
  {
#ifndef NDEBUG
    "VK_LAYER_LUNARG_standard_validation",
#endif
  };
  std::vector<const char*> extensions =
  {
    VK_KHR_SURFACE_EXTENSION_NAME,  // Surface extension
    LAVA_KHR_EXT, // OS specific surface extension
    VK_EXT_DEBUG_REPORT_EXTENSION_NAME
  };


  instance = Instance::create( vk::InstanceCreateInfo(
    { },
    &appInfo,
    layers.size( ),
    layers.data( ),
    extensions.size( ),
    extensions.data( )
  ) );

  CustomVkWindow w;
  w.setVulkanInstance( instance );
  w.resize( 500, 500 );

  w.show( );

  return 0;
}