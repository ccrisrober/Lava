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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MainWindowRenderer : public glfw::VulkanWindowRenderer
{
private:
  glfw::VulkanWindow* _window;
public:
  MainWindowRenderer( glfw::VulkanWindow* window )
    : _window( window )
  {
  }

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  struct
  {
    float scale = 2.5f;
    float intensity = 50.0f;
    glm::vec3 viewPos;
  } uboFS;

  void initResources( void ) override
  {
    auto device = _window->device( );

    geometry = std::make_shared<pompeii::utility::Geometry>( device,
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "ico.obj" ) );

    // MVP buffer
    {
      mvpBuffer = device->createUniformBuffer( sizeof( uboVS ) );

      fsBuffer = device->createUniformBuffer( sizeof( uboFS ) );
    }

    texDiffuse = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
      std::string( "gum.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    {
      texNoise = device->createTexture2D( POMPEII_EXAMPLES_IMAGES_ROUTE +
        std::string( "noiseRGB.png" ), _window->gfxCommandPool( ),
        _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

      texNoise->sampler = device->createSampler( vk::Filter::eLinear,
        vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
        vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
        vk::SamplerAddressMode::eRepeat, 0.0f, true, 1.0f, false,
        vk::CompareOp::eNever, 0.0f, 0.0f, vk::BorderColor::eFloatOpaqueWhite,
        false );

      texNoise->updateDescriptor( );
    }

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "sparkle_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "sparkle_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

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
      DescriptorSetLayoutBinding( 3, vk::DescriptorType::eCombinedImageSampler,
        vk::ShaderStageFlagBits::eFragment
      ),
    };

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    vk::VertexInputBindingDescription binding( 0, sizeof( pompeii::utility::Vertex ),
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription(
        0, 0, vk::Format::eR32G32B32Sfloat, offsetof( pompeii::utility::Vertex, position )
      ),
      vk::VertexInputAttributeDescription(
        1, 0, vk::Format::eR32G32B32Sfloat, offsetof( pompeii::utility::Vertex, normal )
      ),
      vk::VertexInputAttributeDescription(
        2, 0, vk::Format::eR32G32Sfloat, offsetof( pompeii::utility::Vertex, texCoord )
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

    pipeline = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->renderPass( )
    );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 )
    };
    auto dspPool = device->createDescriptorPool( 1, poolSize );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( dspPool, descriptorSetLayout );

    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet(
        descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( mvpBuffer, 0, sizeof( uboVS ) )
      ),
      WriteDescriptorSet(
        descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        texNoise->descriptor, nullptr
      ),
      WriteDescriptorSet(
        descriptorSet, 2, 0, vk::DescriptorType::eUniformBuffer,
        1, nullptr, DescriptorBufferInfo( fsBuffer, 0, sizeof( uboFS ) )
      ),
      WriteDescriptorSet(
        descriptorSet, 3, 0, vk::DescriptorType::eCombinedImageSampler, 1,
        texDiffuse->descriptor, nullptr
      ),
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void updateMVP( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, 
             height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ) * 0.5f, glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.model = glm::rotate( uboVS.model, time * glm::radians( 45.0f ) * 0.5f, glm::vec3( 0.0f, 1.0f, 0.0f ) );

    glm::vec3 camPos = glm::vec3( 2.0f, 2.0f, 2.0f );

    uboVS.view = glm::lookAt( camPos, glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );

    // Vulkan clip space has inverted Y and half Z.
    glm::mat4 clip = glm::mat4(
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, -1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 0.5f, 0.0f,
      0.0f, 0.0f, 0.5f, 1.0f
    );
    uboVS.proj = clip * uboVS.proj;
    //uboVS.proj[ 1 ][ 1 ] *= -1;

    mvpBuffer->writeData( 0, sizeof( uboVS ), &uboVS );

    uboFS.viewPos = camPos;
    fsBuffer->writeData( 0, sizeof( uboFS ), &uboFS );
  }

  void nextFrame( void ) override
  {
    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    const auto size = _window->swapchainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent = size;
    cmd->beginRenderPass(
      _window->renderPass( ),
      _window->framebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );
    cmd->setViewportScissors( size );

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, { } );

    /*cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );

    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );*/
    geometry->render( cmd );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
private:
  std::shared_ptr< Texture2D > texDiffuse;
  std::shared_ptr< Texture2D > texNoise;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< Buffer > vertexBuffer;
  std::shared_ptr< Buffer > indexBuffer;
  std::shared_ptr< UniformBuffer > mvpBuffer;
  std::shared_ptr< UniformBuffer > fsBuffer;

  std::shared_ptr<pompeii::utility::Geometry> geometry;
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
  VulkanWindow app( 500, 500, "Sparkle Mesh", true );
  app.show( );
  return EXIT_SUCCESS;
}