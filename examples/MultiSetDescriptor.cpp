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
#include <lavaRenderer/lavaRenderer.h>
using namespace lava;

#include <routes.h>

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Multidescriptor" );
  }

  void createMaterial( const std::string& texPath,
    std::shared_ptr<PipelineLayout>& pipelineLayout,
    std::shared_ptr<DescriptorSetLayout>& descriptorSetLayoutBasic,
    std::shared_ptr<DescriptorSet>& descriptorSet,
    std::shared_ptr<Texture2D>& texture )
  {
    auto device = _window->device( );

    texture = device->createTexture2D( texPath, _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    std::vector<DescriptorSetLayoutBinding> dslbs =
    {
      DescriptorSetLayoutBinding( 0, 
        vk::DescriptorType::eCombinedImageSampler, 
        vk::ShaderStageFlagBits::eFragment
      )
    };

    auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( 
      { descriptorSetLayoutBasic, descriptorSetLayout }, nullptr );

    // Init descriptor set
    descriptorSet = device->allocateDescriptorSet( descriptorPool, 
      descriptorSetLayout );
    std::vector<WriteDescriptorSet> wdss =
    {
      WriteDescriptorSet( descriptorSets.basic, 0, 0,
        vk::DescriptorType::eUniformBuffer, 1, nullptr,
        DescriptorBufferInfo(
          uniformBufferMVP, 0, sizeof( ubo )
        )
      ),
      WriteDescriptorSet( descriptorSet, 0, 0,
        vk::DescriptorType::eCombinedImageSampler, 1,
        texture->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, {} );
  }

  void initResources( void ) override
  {
    auto device = _window->device( );

    geometry = std::make_shared<lava::extras::Geometry>( device, 
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "wolf.obj_" ) );

    // MVP buffer
    {
      uniformBufferMVP = device->createBuffer( sizeof( ubo ), 
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | 
        vk::MemoryPropertyFlagBits::eHostCoherent );
    }

    std::vector< DescriptorSetLayoutBinding > dslbs =
    {
      DescriptorSetLayoutBinding( 0,
        vk::DescriptorType::eUniformBuffer,
        vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment )
    };
    auto descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    descriptorPool = device->createDescriptorPool( 4, {
      { vk::DescriptorType::eUniformBuffer, 1 },
      { vk::DescriptorType::eCombinedImageSampler, 3 }
    } );

    descriptorSets.basic = device->allocateDescriptorSet( descriptorPool,
      descriptorSetLayout );

    createMaterial(
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/MatCap_Toon3.png" ),
      pipelineLayouts.matcapLeft,
      descriptorSetLayout,
      descriptorSets.matcapLeft, textures.matcapLeft
    );

    createMaterial(
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/green_matcap.jpg" ),
      pipelineLayouts.matcapCenter,
      descriptorSetLayout,
      descriptorSets.matcapCenter, textures.matcapCenter
    );

    createMaterial(
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/rubymatcap.jpg" ),
      pipelineLayouts.matcapRight,
      descriptorSetLayout,
      descriptorSets.matcapRight, textures.matcapRight
    );

    auto vertexStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "matcap_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "matcap_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof( lava::extras::Vertex, position ) ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat, 
            offsetof( lava::extras::Vertex, normal ) )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { {} }, { {} } );   // one dummy viewport and scissor, as dynamic state sets them
    vk::PipelineRasterizationStateCreateInfo rasterization( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );


    pipelines.matcapLeft = device->createGraphicsPipeline( _window->pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport,
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.matcapLeft, _window->defaultRenderPass( )
    );

    pipelines.matcapCenter = device->createGraphicsPipeline( _window->pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport,
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.matcapCenter, _window->defaultRenderPass( )
    );

    pipelines.matcapRight = device->createGraphicsPipeline( _window->pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport,
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.matcapRight, _window->defaultRenderPass( )
    );
  }

  void updateUniformBuffers( void )
  {
    auto size = _window->getExtent( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    ubo.model = glm::translate( glm::mat4( 1.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    ubo.model = glm::rotate( ubo.model,
      time * glm::radians( 90.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    ubo.model = glm::translate( ubo.model, glm::vec3( 0.5f, 0.0f, 1.0f ) );
    glm::vec3 cameraPos = glm::vec3( 0.0f, 1.0f, 10.5f );
    glm::vec3 cameraFront = glm::vec3( 0.0f, 0.0f, -1.0f );
    glm::vec3 cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );

    ubo.view = glm::lookAt( cameraPos, cameraPos + cameraFront, cameraUp );
    ubo.proj = glm::perspective( glm::radians( 45.0f ), width / ( float ) height, 0.1f, 10.0f );
    ubo.proj[ 1 ][ 1 ] *= -1;

    uniformBufferMVP->set( &ubo );
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }
    
    updateUniformBuffers( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil  = vk::ClearDepthStencilValue(  1.0f, 0 );

    const glm::ivec2 size = _window->swapChainImageSize( );
    auto cmd = _window->currentCommandBuffer( );
    vk::Rect2D rect;
    rect.extent.width = size.x;
    rect.extent.height = size.y;
    cmd->beginRenderPass(
      _window->defaultRenderPass( ),
      _window->currentFramebuffer( ),
      rect, clearValues, vk::SubpassContents::eInline
    );

    uint32_t width = size.x, height = size.y;

    vk::Rect2D scissor( { 0, 0 }, { width, height } );

    cmd->setViewport( 0, vk::Viewport( 0.0f, 0.0f, width, height ) );

    // Left pipeline
    cmd->bindGraphicsPipeline( pipelines.matcapLeft );
    scissor.offset.x = 0;
    scissor.extent.width = ( float ) width / 3.0;
    cmd->setScissor( 0, scissor );

    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.matcapLeft, 0,
      { descriptorSets.basic, descriptorSets.matcapLeft }, nullptr );

    geometry->render( cmd );

    // Center pipeline
    cmd->bindGraphicsPipeline( pipelines.matcapCenter );
    scissor.offset.x = ( float ) width / 3.0;
    cmd->setScissor( 0, scissor );

    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.matcapCenter, 0, 
      { descriptorSets.basic, descriptorSets.matcapCenter }, nullptr );

    geometry->render( cmd );

    // Right pipeline
    cmd->bindGraphicsPipeline( pipelines.matcapRight );
    scissor.offset.x = ( float ) width / 3.0 + ( float ) width / 3.0;
    cmd->setScissor( 0, scissor );

    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayouts.matcapRight, 0,
      { descriptorSets.basic, descriptorSets.matcapRight }, nullptr );

    geometry->render( cmd );

    cmd->endRenderPass( );

    _window->requestUpdate( );
  }
private:
  VulkanWindow *_window;

  std::shared_ptr<Buffer> uniformBufferMVP;

  std::shared_ptr<lava::extras::Geometry> geometry;

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } ubo;

  struct
  {
    std::shared_ptr<DescriptorSet> basic;
    std::shared_ptr<DescriptorSet> matcapLeft;
    std::shared_ptr<DescriptorSet> matcapCenter;
    std::shared_ptr<DescriptorSet> matcapRight;
  } descriptorSets;

  struct
  {
    std::shared_ptr<Texture2D> matcapLeft;
    std::shared_ptr<Texture2D> matcapCenter;
    std::shared_ptr<Texture2D> matcapRight;
  } textures;

  struct
  {
    std::shared_ptr<PipelineLayout> matcapLeft;
    std::shared_ptr<PipelineLayout> matcapCenter;
    std::shared_ptr<PipelineLayout> matcapRight;
  } pipelineLayouts;

  struct
  {
    std::shared_ptr<Pipeline> matcapLeft;
    std::shared_ptr<Pipeline> matcapCenter;
    std::shared_ptr<Pipeline> matcapRight;
  } pipelines;

  std::shared_ptr<DescriptorPool> descriptorPool;
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