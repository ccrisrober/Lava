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

#include <pompeii/pompeii.h>
#include <pompeiiRenderer/pompeiiRenderer.h>
#include <pompeiiUtils/pompeiiUtils.h>
using namespace pompeii;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( glfw::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Shadow Mapping Screen" );
  }

  std::shared_ptr < Buffer > vertexBuffer;

  struct
  {
    glm::mat4 projection;
    glm::mat4 view;
  } ubo0;
  struct
  {
    glm::mat4 lightSpaceMatrix;
  } ubo1;
  struct
  {
    glm::vec3 lightPos;
    glm::vec3 viewPos;
  } ubo2;

  std::shared_ptr< UniformBuffer > ubo0Buffer, ubo1Buffer, ubo2Buffer;
  std::shared_ptr< Texture > cubeTexture;

  struct
  {
    std::shared_ptr<DescriptorSetLayout> shadow;
    std::shared_ptr<DescriptorSetLayout> solid;
  } descriptorSetLayouts;

  struct
  {
    std::shared_ptr<DescriptorSet> shadow;
    std::shared_ptr<DescriptorSet> solid;
  } descriptorSets;

  struct
  {
    std::shared_ptr<PipelineLayout> shadow;
    std::shared_ptr<PipelineLayout> solid;
  } pipelineLayouts;

  struct
  {
    std::shared_ptr<Pipeline> shadow;
    std::shared_ptr<Pipeline> solid;
  } pipelines;

  std::shared_ptr<::utility::CustomFramebuffer> postProcessFBO;

  void recreatePostProcessFBO( void )
  {
    uint32_t w, h;

    auto device = _window->device( );

    auto sc = _window->getExtent( );
    w = sc.width;// *2;
    h = sc.height;// *2;

    postProcessFBO = std::make_shared<pompeii::utility::CustomFramebuffer>( device, w, h );
    // Depth attachment
    /*vk::Format attDepthFormat;
    VkBool32 validDepthFormat = pompeii::utils::getSupportedDepthFormat(
      _window->physicalDevice( ), attDepthFormat );
    assert( validDepthFormat );*/
    vk::Format attDepthFormat = vk::Format::eD16Unorm;

    postProcessFBO->addAttachment( w, h, attDepthFormat,
      vk::ImageUsageFlagBits::eDepthStencilAttachment );
    postProcessFBO->createSampler( vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerAddressMode::eClampToEdge );
    postProcessFBO->createRenderPass( );
  }

  virtual void initSwapChainResources( void ) override
  {
    recreatePostProcessFBO( );
  }
  virtual void releaseSwapChainResources( void ) override
  {
    postProcessFBO.reset( );
  }

  void initResources( void ) override
  {
    std::vector<float> planeVertices = {
      // positions            // normals         // texcoords
       25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
      -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
      -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

       25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
      -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
       25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
    };

    auto device = _window->device( );

    // UBO buffers
    {
      ubo0Buffer = device->createUniformBuffer( sizeof( ubo0 ) );
      ubo1Buffer = device->createUniformBuffer( sizeof( ubo1 ) );
      ubo2Buffer = device->createUniformBuffer( sizeof( ubo2 ) );
    }

    // Vertex buffer
    {
      uint32_t vertexBufferSize = planeVertices.size( ) *
        sizeof( planeVertices.at( 0 ) );
      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );

      vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      vertexBuffer->update<float>( cmd, 0, { uint32_t( planeVertices.size( ) ),
        planeVertices.data( ) } );
      cmd->end( );
      _window->gfxQueue( )->submitAndWait( cmd );
    }

    cubeTexture = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "uv_checker.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 4 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 )
    };
    auto dspPool = device->createDescriptorPool( 2, poolSize );

    {
      auto vertexStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "shadow_mapping_depth_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        POMPEII_EXAMPLES_SPV_ROUTE + std::string( "shadow_mapping_depth_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        DescriptorSetLayoutBinding( 1, vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex
        )
      };

      descriptorSetLayouts.shadow = device->createDescriptorSetLayout( dslbs );

      vk::PushConstantRange pushConstantRange(
        vk::ShaderStageFlagBits::eVertex, 0, sizeof( glm::mat4 )
      );
      pipelineLayouts.shadow = device->createPipelineLayout(
        descriptorSetLayouts.shadow, pushConstantRange );

      vk::VertexInputBindingDescription binding( 0, sizeof( float ) * 8,
        vk::VertexInputRate::eVertex );
      PipelineVertexInputStateCreateInfo vertexInput( binding, {
        vk::VertexInputAttributeDescription(
          0, 0, vk::Format::eR32G32B32Sfloat, 0
        ),
        vk::VertexInputAttributeDescription(
          1, 0, vk::Format::eR32G32B32Sfloat, 3 * sizeof( float )
        ),
        vk::VertexInputAttributeDescription(
          2, 0, vk::Format::eR32G32Sfloat, 5 * sizeof( float )
        )
      } );
      vk::PipelineInputAssemblyStateCreateInfo assembly( { },
        vk::PrimitiveTopology::eTriangleList, VK_FALSE );
      PipelineViewportStateCreateInfo viewport( 1, 1 );
      vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
        false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
        vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
      PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1,
        false, 0.0f, nullptr, false, false );
      vk::StencilOpState stencilOpState( vk::StencilOp::eKeep,
        vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways,
        0, 0, 0 );
      vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true,
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
        stencilOpState, 0.0f, 0.0f );
      ;
      PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
        vk::PipelineColorBlendAttachmentState(
          false, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
          vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero,
          vk::BlendOp::eAdd, vk::ColorComponentFlagBits::eR |
          vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
          vk::ColorComponentFlagBits::eA
        ), { 1.0f, 1.0f, 1.0f, 1.0f }
      );
      PipelineDynamicStateCreateInfo dynamic( {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
      } );

      pipelines.shadow = device->createGraphicsPipeline( _window->pipelineCache, { },
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        pipelineLayouts.shadow, _window->defaultRenderPass( )
      );

      // Init descriptor set
      descriptorSets.shadow = device->allocateDescriptorSet( dspPool, descriptorSetLayouts.shadow );

      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet(
          descriptorSets.shadow, 1, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr, DescriptorBufferInfo( ubo1Buffer, 0, sizeof( ubo1 ) )
        )
      };
      device->updateDescriptorSets( wdss, { } );
    }

    recreatePostProcessFBO( );
  }

  glm::vec3 lightPos = glm::vec3( -2.0f, 4.0f, -1.0f );

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( pompeii::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 7.5f;
    //lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
    lightProjection = glm::ortho( -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane );
    lightView = glm::lookAt( lightPos, glm::vec3( 0.0f ), glm::vec3( 0.0, 1.0, 0.0 ) );
    lightSpaceMatrix = lightProjection * lightView;

    // Vulkan clip space has inverted Y and half Z.
    glm::mat4 clip = glm::mat4(
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.5f, 0.0f,
      0.0f, 0.0f, 0.5f, 1.0f
    );
    ubo1.lightSpaceMatrix = clip * lightSpaceMatrix;

    ubo1Buffer->writeData( 0, sizeof( ubo1 ), &ubo1 );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 1.0f, 0.0f, 1.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const vk::Offset2D size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );
    cmd->bindGraphicsPipeline( pipelines.shadow );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.shadow, 0, { descriptorSets.shadow }, { } );
    cmd->setViewportScissors( _window->getExtent( ) );
    cmd->pushConstants<glm::mat4>( *pipelineLayouts.shadow,
      vk::ShaderStageFlagBits::eGeometry, 0, glm::mat4( 1.0f ) );
    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->draw( 6, 1, 0, 0 );

    cmd->endRenderPass( );

    _window->requestUpdate( );
  }
private:
  VulkanWindow *_window;
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
    /*#ifndef NDEBUG
    "VK_LAYER_LUNARG_standard_validation",
    #endif*/
  };
  std::vector<const char*> extensions =
  {
    VK_KHR_SURFACE_EXTENSION_NAME,  // Surface extension
    POMPEII_KHR_EXT, // OS specific surface extension
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
