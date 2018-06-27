/**
 * Copyright (c) 2017 - 2018, Lava
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
#include <lavaUtils/lavaUtils.h>
#include <lavaRenderer/lavaRenderer.h>
using namespace lava;

#include <routes.h>
#include <glm/gtc/matrix_transform.hpp>

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "(Perfect) Toon Shading with Stencil Outline" );
  }

  void initResources( void ) override
  {
    auto device = _window->device( );

    geometry = std::make_shared<lava::utility::Geometry>( device, 
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "ear_study3.obj_" ) );

    uniformMVP = device->createUniformBuffer( sizeof( uboVS ) );
    uniformBufferOutline = device->createUniformBuffer( sizeof( uboVS2 ) );

    // Init descriptor and pipeline layouts
    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };
    descriptorSetLayouts.solid = device->createDescriptorSetLayout( dslbs );

    pipelineLayouts.solid = device->createPipelineLayout( descriptorSetLayouts.solid, nullptr );

    dslbs =
    {
      DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      ),
      DescriptorSetLayoutBinding( 1, vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex
      )
    };
    descriptorSetLayouts.outline = device->createDescriptorSetLayout( dslbs );

    pipelineLayouts.outline = device->createPipelineLayout( descriptorSetLayouts.outline, nullptr );

    // init pipeline
    auto vertexStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "perfectToon_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "perfectToon_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::utility::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof( lava::utility::Vertex, position )
          ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof( lava::utility::Vertex, normal )
          )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, 
      vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( 1, 1 );
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone,
      vk::FrontFace::eClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, 
      false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, 
      vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencilState( { }, true, true, 
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

    depthStencilState.stencilTestEnable = VK_TRUE;

    // glStencilFunc( GL_ALWAYS, 1, 0xFF );
    depthStencilState.back.compareOp = vk::CompareOp::eAlways;
    depthStencilState.back.compareMask = 0xff;
    depthStencilState.back.reference = 1;
    // glStencilMask(0xFF);
    depthStencilState.back.writeMask = 0xff;
    // glEnable( GL_DEPTH );
    depthStencilState.depthTestEnable = VK_TRUE;

    depthStencilState.back.failOp = vk::StencilOp::eKeep;
    depthStencilState.back.depthFailOp = vk::StencilOp::eKeep;
    depthStencilState.back.passOp = vk::StencilOp::eReplace;

    depthStencilState.front = depthStencilState.back;

    pipelines.solid = device->createGraphicsPipeline( _window->pipelineCache, { }, 
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, 
      viewport, rasterization, multisample, depthStencilState, colorBlend, 
      dynamic, pipelineLayouts.solid, _window->defaultRenderPass( )
    );

    // Outline pass
    // glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    depthStencilState.back.compareOp = vk::CompareOp::eNotEqual;
    // depthStencilState.back.compareMask = 0xff;
    // depthStencilState.back.reference = 1;
    // glDisable( GL_DEPTH );
    depthStencilState.depthTestEnable = VK_FALSE;

    depthStencilState.front = depthStencilState.back;

    vertexStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "outline_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    fragmentStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "outline_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    pipelines.outline = device->createGraphicsPipeline( _window->pipelineCache, { },
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencilState, colorBlend,
      dynamic, pipelineLayouts.outline, _window->defaultRenderPass( )
    );

    std::array<vk::DescriptorPoolSize, 1> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 3 )
    };
    auto descriptorPool = device->createDescriptorPool( 2, poolSize );

    // Init descriptor set
    descriptorSets.solid = device->allocateDescriptorSet( 
      descriptorPool, descriptorSetLayouts.solid
    );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSets.solid, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uniformMVP, 0, sizeof( uboVS )
        )
      )
    };
    device->updateDescriptorSets( wdss, { } );

    descriptorSets.outline = device->allocateDescriptorSet( 
      descriptorPool, descriptorSetLayouts.outline
    );
    wdss =
    {
      WriteDescriptorSet( descriptorSets.outline, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uniformMVP, 0, sizeof( uboVS )
        )
      ),
      WriteDescriptorSet( descriptorSets.outline, 1, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uniformBufferOutline, 0, sizeof( uboVS2 )
        )
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void updateUniforms( void )
  {
    auto size = _window->getExtent( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    uboVS.model = glm::mat4( 1.0f );
    uboVS.model = glm::rotate(
      uboVS.model, time * glm::radians( 90.0f ),
      glm::vec3( 0.0f, -1.0f, 0.0f )
    );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 0.0f, 5.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    uboVS.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );

    uboVS.proj = glm::perspective( glm::radians( 45.0f ),
      width / ( float ) height, 0.1f, 10.0f );
    uboVS.proj[ 1 ][ 1 ] *= -1;

    uniformMVP->set( &uboVS );

    uniformBufferOutline->set( &uboVS2 );
  }

  void updateOutlineWidth( float inc )
  {
    if ( uboVS2.outlineWidth + inc > 0.01f )
    {
      uboVS2.outlineWidth += inc;
    }
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }
    
    if ( Input::isKeyPressed( lava::Keyboard::Key::Z ) )
    {
      updateOutlineWidth( -0.005f );
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::X ) )
    {
      updateOutlineWidth( +0.005f );
    }
    
    updateUniforms( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil  = vk::ClearDepthStencilValue(  1.0f, 0 );

    const auto size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ), _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

      cmd->setViewportScissors( _window->getExtent( ) );

      // First pass renders object (toon shaded) and fills stencil buffer
      cmd->bindGraphicsPipeline( pipelines.solid );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        pipelineLayouts.solid, 0, { descriptorSets.solid }, nullptr );

      geometry->render( cmd );

      // Second pass renders scaled object only where stencil was not set by first pass
      cmd->bindGraphicsPipeline( pipelines.outline );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        pipelineLayouts.outline, 0, { descriptorSets.outline }, nullptr );

      geometry->render( cmd );

    cmd->endRenderPass( );

    _window->requestUpdate( );
  }
private:
  VulkanWindow *_window;

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  struct
  {
    float outlineWidth = 0.05f;
  } uboVS2;

  std::shared_ptr<lava::UniformBuffer> uniformMVP;
  std::shared_ptr<lava::UniformBuffer> uniformBufferOutline;

  struct
  {
    std::shared_ptr<Pipeline> solid;
    std::shared_ptr<Pipeline> outline;
  } pipelines;

  struct
  {
    std::shared_ptr<PipelineLayout> solid;
    std::shared_ptr<PipelineLayout> outline;
  } pipelineLayouts;

  struct
  {
    std::shared_ptr<DescriptorSet> solid;
    std::shared_ptr<DescriptorSet> outline;
  } descriptorSets;

  struct
  {
    std::shared_ptr<DescriptorSetLayout> solid;
    std::shared_ptr<DescriptorSetLayout> outline;
  } descriptorSetLayouts;

  std::shared_ptr<lava::utility::Geometry> geometry;

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