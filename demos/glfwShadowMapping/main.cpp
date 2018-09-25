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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

#define SHADOWMAP_DIM 2048

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  glm::vec3 directionalLightPos = glm::vec3( 0.0f, 5.0f, 0.0f );

  const glm::vec3 mammothScale = glm::vec3( 0.001f );
  const glm::vec3 cameraPos = glm::vec3( 0.0f, 2.5f, 15.0f );
  const float fov = 60.0f;

  glm::uvec2 fbSize = glm::uvec2( SHADOWMAP_DIM, SHADOWMAP_DIM );

  struct
  {
    std::shared_ptr<pompeii::Image> depth;
    std::shared_ptr<pompeii::ImageView> depthView;
    std::shared_ptr<pompeii::RenderPass> renderPass;
    std::shared_ptr<pompeii::Framebuffer> framebuffer;

    std::shared_ptr<pompeii::Sampler> sampler;
  } fb;

  std::shared_ptr<pompeii::CommandBuffer> rtCmdBuffer;
  std::shared_ptr<pompeii::Semaphore> rtSemaphore;
  std::shared_ptr<pompeii::/*Graphics*/Pipeline> rtPipeline;
  std::shared_ptr<pompeii::PipelineLayout> pipelineLayoutShadow;

  std::shared_ptr<pompeii::UniformBuffer> uniformBuffer0, uniformBuffer1;
  std::shared_ptr<pompeii::Sampler> nearestSampler;
  std::shared_ptr<pompeii::DescriptorSetLayout> descriptorSetLayout;
  std::shared_ptr<pompeii::DescriptorSet> descriptorSet;
  std::shared_ptr<pompeii::DescriptorPool> descriptorPool;
  std::shared_ptr<pompeii::PipelineLayout> pipelineLayout;
  std::shared_ptr<pompeii::/*Graphics*/Pipeline> graphicsPipeline;

  struct
  {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
    glm::vec3 lightPos;
    glm::vec3 viewPos;
  } ubo0;

  struct
  {
    glm::mat4 lightSpaceMatrix;
  } ubo1;

  std::shared_ptr<pompeii::utility::Geometry> geometry;

  virtual void initResources( void )
  {
    std::cout << "Creating FBO and RenderPass ...\n\n";
    createFramebuffer( { fbSize.x, fbSize.y } );
    std::cout << "\n\n ... created" << std::endl;

    geometry = std::make_shared<pompeii::utility::Geometry>( _window->device( ),
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "mammoth.ply" ) );

    std::cout << "Creating UBO ...\n\n";
    createUniformBuffer( );
    std::cout << "\n\n ... created" << std::endl;
    std::cout << "Creating DescriptorSets ...\n\n";
    setupDescriptorSet( );
    std::cout << "\n\n ... created" << std::endl;
    std::cout << "Creating Pipelines ...\n\n";
    setupPipelines( );
    std::cout << "\n\n ... created" << std::endl;
    std::cout << "Creating RTCmd ...\n\n";
    recordRTCommandBuffer( );
    std::cout << "\n\n ... created" << std::endl;
  }

  void createFramebuffer( const vk::Extent2D& extent )
  {
    auto device = _window->device( );
    auto depthFormat = vk::Format::eD16Unorm;
    fb.depth = device->createImage( { }, vk::ImageType::e2D,
      depthFormat, vk::Extent3D( extent.width, extent.height, 1 ), 1, 1,
      vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
      // We will sample directly from the depth attachment for the shadow mapping
      vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
      vk::SharingMode::eExclusive, { }, vk::ImageLayout::eUndefined,
      vk::MemoryPropertyFlagBits::eDeviceLocal );
    fb.depthView = fb.depth->createImageView(
        vk::ImageViewType::e2D, depthFormat,
        vk::ComponentMapping(
          vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
          vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA ),
        vk::ImageSubresourceRange( vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1 ) );

        //createImageView( vk::ImageViewType::e2D, depthFormat );

    vk::AttachmentDescription attachmentDescription;
    attachmentDescription.format = depthFormat;
    attachmentDescription.samples = vk::SampleCountFlagBits::e1;
    attachmentDescription.loadOp = vk::AttachmentLoadOp::eClear;
    attachmentDescription.storeOp = vk::AttachmentStoreOp::eStore;
    attachmentDescription.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
    attachmentDescription.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    attachmentDescription.initialLayout = vk::ImageLayout::eUndefined;
    attachmentDescription.finalLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;

    vk::AttachmentReference depthRef;
    depthRef.attachment = 0;
    depthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depthRef;

    // Use subpass dependencies for layout transitions
    std::array<vk::SubpassDependency, 2> dependencies;

    dependencies[ 0 ].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 0 ].dstSubpass = 0;
    dependencies[ 0 ].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;;
    dependencies[ 0 ].dstStageMask = vk::PipelineStageFlagBits::eLateFragmentTests;
    //dependencies[ 0 ].srcAccessMask = 0;
    dependencies[ 0 ].dstAccessMask = 
      vk::AccessFlagBits::eDepthStencilAttachmentRead |
      vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    dependencies[ 0 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    dependencies[ 1 ].srcSubpass = 0;
    dependencies[ 1 ].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[ 1 ].srcStageMask = vk::PipelineStageFlagBits::eLateFragmentTests;
    dependencies[ 1 ].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[ 1 ].srcAccessMask =
      vk::AccessFlagBits::eDepthStencilAttachmentRead |
      vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    dependencies[ 1 ].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[ 1 ].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    fb.renderPass = _window->device( )->createRenderPass(
      attachmentDescription, subpass, dependencies );

    fb.framebuffer = device->createFramebuffer( fb.renderPass, { fb.depthView }, extent, 1 );
  }

  void createUniformBuffer( void )
  {
    uniformBuffer0 = _window->device( )->createUniformBuffer( sizeof( ubo0 ) );
    uniformBuffer1 = _window->device( )->createUniformBuffer( sizeof( ubo1 ) );
  }

  void setupDescriptorSet( void )
  {
    auto device = _window->device( );
    descriptorPool = device->createDescriptorPool( 1, {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 ),
    } );
    descriptorSetLayout = device->createDescriptorSetLayout( {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
      ),
      DescriptorSetLayoutBinding( 1,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 2,
        vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      )
    } );
    descriptorSet = device->allocateDescriptorSet(
      descriptorPool, descriptorSetLayout );

    fb.sampler = device->createSampler(
      vk::Filter::eNearest, vk::Filter::eNearest,
      vk::SamplerMipmapMode::eNearest,
      vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge,
      vk::SamplerAddressMode::eClampToEdge,
      0.0f, false, 0.0f, false,
      vk::CompareOp::eNever, 0.0f, 1.0f,
      vk::BorderColor::eFloatOpaqueWhite, false );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uniformBuffer0, 0, sizeof( ubo0 ) )
      ),
      WriteDescriptorSet(
        descriptorSet, 1, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( uniformBuffer1, 0, sizeof( ubo1 ) )
      ),
      WriteDescriptorSet(
        descriptorSet, 2, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        DescriptorImageInfo( vk::ImageLayout::eDepthStencilReadOnlyOptimal,
          fb.depthView, fb.sampler
        ), nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void setupPipelines( void )
  {
    auto device = _window->device( );
    {
      std::cout << "\t\tSHADOW MAPPING DEPTH PIPELINE ... " << std::endl;
      
      pipelineLayoutShadow = device->createPipelineLayout( descriptorSetLayout );
      
      auto vertexStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "shadow_mapping_depth_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "shadow_mapping_depth_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput(
        vk::VertexInputBindingDescription( 0, sizeof( pompeii::utility::Vertex ),
          vk::VertexInputRate::eVertex ),
          {
            vk::VertexInputAttributeDescription(
              0, 0, vk::Format::eR32G32B32Sfloat,
              offsetof( pompeii::utility::Vertex, position )
            )
          }
      );

      vk::PipelineInputAssemblyStateCreateInfo assembly( {},
        vk::PrimitiveTopology::eTriangleList, VK_FALSE );

      PipelineViewportStateCreateInfo viewport( 1, 1 );
      vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );

      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
        0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
        stencilOpState, 0.0f, 0.0f );
      PipelineColorBlendStateCreateInfo colorBlend( false,
        vk::LogicOp::eNoOp, { }, { 1.0f, 1.0f, 1.0f, 1.0f }
      );
      PipelineDynamicStateCreateInfo dynamic( {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
      } );

      PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
        false, 0.0f, nullptr, false, false );
      rtPipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayoutShadow, fb.renderPass
      );
    }

    {
      std::cout << "\t\tSHADOW MAPPING PIPELINE ... " << std::endl;
      
      pipelineLayout = device->createPipelineLayout( descriptorSetLayout );

      auto vertexStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "shadow_mapping_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "shadow_mapping_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      PipelineVertexInputStateCreateInfo vertexInput(
        vk::VertexInputBindingDescription( 0, sizeof( pompeii::utility::Vertex ),
          vk::VertexInputRate::eVertex ),
          {
            vk::VertexInputAttributeDescription(
              0, 0, vk::Format::eR32G32B32Sfloat,
              offsetof( pompeii::utility::Vertex, position )
            ),
            vk::VertexInputAttributeDescription(
              1, 0, vk::Format::eR32G32B32Sfloat,
              offsetof( pompeii::utility::Vertex, normal )
            )/*,
            vk::VertexInputAttributeDescription(
              2, 0, vk::Format::eR32G32Sfloat,
              offsetof( pompeii::utility::Vertex, texCoord )
            )*/
          }
      );

      vk::PipelineInputAssemblyStateCreateInfo assembly( {},
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

      graphicsPipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayout, _window->renderPass( ) );
    }
  }

  void recordRTCommandBuffer( void )
  {
    rtCmdBuffer = _window->gfxCommandPool( )->allocateCommandBuffer( );
    rtSemaphore = _window->device( )->createSemaphore( );

    rtCmdBuffer->begin( );
    {
      std::array<vk::ClearValue, 1 > clearValues;
      clearValues[ 0 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

      rtCmdBuffer->beginRenderPass( fb.renderPass, fb.framebuffer,
        vk::Rect2D( { 0, 0 }, { fbSize.x, fbSize.y } ), clearValues, vk::SubpassContents::eInline );
      {
        rtCmdBuffer->setViewportScissors( { fbSize.x, fbSize.y } );
        rtCmdBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
          pipelineLayoutShadow, 0, descriptorSet, { } );
        rtCmdBuffer->bindGraphicsPipeline( rtPipeline );

        geometry->render( rtCmdBuffer );
      }
      rtCmdBuffer->endRenderPass( );
    }
    rtCmdBuffer->end( );
  }

  virtual void nextFrame( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    glm::vec3 lightPos = directionalLightPos;
    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;
    //std::cout << time << std::endl;
    // change light position over time
    //lightPos.x = sin( time ) * 3.0f;
    lightPos.z = cos( time ) * 2.0f;
    //lightPos.y = cos( time ) * 1.0f;

    ubo0.projection = glm::perspective( glm::radians( fov ), width / ( float ) height, 0.1f, 1000.0f );
    ubo0.projection[ 1 ][ 1 ] *= -1;

    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    ubo0.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );
    
    ubo0.model = glm::scale( glm::mat4( 1.0f ), mammothScale );
    ubo0.lightPos = lightPos;
    ubo0.viewPos = cameraPos;

    uniformBuffer0->set( &ubo0 );

    glm::mat4 lightProjection, lightView;
    float near_plane = 1.0f, far_plane = 1000.0f;
    float lightFOV = 45.0f;
    //lightProjection = glm::ortho( -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane );
    lightProjection = glm::perspective(glm::radians(lightFOV), 1.0f, near_plane, far_plane);
    //lightProjection[ 1 ][ 1 ] *= -1;
    lightView = glm::lookAt(ubo0.lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    ubo1.lightSpaceMatrix = lightProjection * lightView;

    uniformBuffer1->set( &ubo1 );

    _window->gfxQueue( )->submit( SubmitInfo{
        _window->currentSemaphore( ),
        { vk::PipelineStageFlagBits::eColorAttachmentOutput },
        rtCmdBuffer,
        rtSemaphore
    } );

    auto cmd = _window->currentCommandBuffer( );

    /*static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;*/

    std::array<vk::ClearValue, 2 > clearValues;
    clearValues[ 0 ] = pompeii::utils::getClearValueColor( 1.0f, 1.0f, 1.0f );
    clearValues[ 1 ] = pompeii::utils::getClearValueDepth( );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->bindGraphicsPipeline( graphicsPipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, nullptr );
    cmd->setViewportScissors( extent );
    geometry->render( cmd );

    cmd->endRenderPass( );

    _window->frameReady( rtSemaphore );
  }
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