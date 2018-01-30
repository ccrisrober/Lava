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
#include <lavaRenderer/lavaRenderer.h>
using namespace lava;

#include <routes.h>

#include "utils/Camera.h"

const unsigned int SCR_WIDTH = 500;
const unsigned int SCR_HEIGHT = 500;

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Earth with Clouds Atmosphere" );
    camera = Camera( glm::vec3( 0.0f, 0.0f, 3.5f ) );
  }

  // camera
  Camera camera;
  // timing
  float deltaTime = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;

  float lastX = SCR_WIDTH * 0.5f;
  float lastY = SCR_HEIGHT * 0.5f;
  bool firstMouse = true;

  void initResources( void ) override
  {
    auto device = _window->device( );

    geometry = std::make_shared<lava::extras::Geometry>( device,
      LAVA_EXAMPLES_MESHES_ROUTE + std::string( "sphere.obj_" ) );

    // MVP buffers
    {
      diffuse.mvpBuffer = device->createUniformBuffer( sizeof( diffuse.uboVS ) );
      atmosphere.mvpBuffer = device->createUniformBuffer( sizeof( atmosphere.uboVS ) );
    }

    tex = device->createTexture2D( LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "earth/earth_diffuse.jpg" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    tex2 = device->createTexture2D( LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "earth/earth_normal.jpg" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    tex3 = device->createTexture2D( LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "earth/earth_clouds.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 3 )
    };
    auto dspPool = device->createDescriptorPool( 2, poolSize );

    {
      auto vertexStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "mesh_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "mesh_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
        ),
        DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        ),
        DescriptorSetLayoutBinding( 2, vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        )
      };

      diffuse.descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

      vk::PushConstantRange pushConstantRange(
        vk::ShaderStageFlagBits::eVertex, 0, sizeof( diffuse.pc )
      );

      diffuse.pipelineLayout = device->createPipelineLayout( diffuse.descriptorSetLayout, pushConstantRange );

      vk::VertexInputBindingDescription binding( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding, {
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
      } );

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
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
        stencilOpState, 0.0f, 0.0f );
      ;
      PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
        vk::PipelineColorBlendAttachmentState( false, 
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
          vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, 
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | 
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
        ), { 1.0f, 1.0f, 1.0f, 1.0f }
      );
      PipelineDynamicStateCreateInfo dynamic( {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
      } );

      diffuse.pipeline = device->createGraphicsPipeline( _window->pipelineCache, {},
      { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        diffuse.pipelineLayout, _window->defaultRenderPass( )
      );

      // Init descriptor set
      diffuse.descriptorSet = device->allocateDescriptorSet( dspPool, 
        diffuse.descriptorSetLayout );

      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet(
          diffuse.descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr, DescriptorBufferInfo( diffuse.mvpBuffer, 0, sizeof( diffuse.uboVS ) )
        ),
        WriteDescriptorSet(
          diffuse.descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
          tex->descriptor, nullptr
        ),
        WriteDescriptorSet(
          diffuse.descriptorSet, 2, 0, vk::DescriptorType::eCombinedImageSampler, 1,
          tex2->descriptor, nullptr
        )
      };
      device->updateDescriptorSets( wdss, {} );
    }

    {
      auto vertexStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "atmosphere_vert.spv" ),
        vk::ShaderStageFlagBits::eVertex
      );
      auto fragmentStage = device->createShaderPipelineShaderStage(
        LAVA_EXAMPLES_SPV_ROUTE + std::string( "atmosphere_frag.spv" ),
        vk::ShaderStageFlagBits::eFragment
      );

      std::vector<DescriptorSetLayoutBinding> dslbs =
      {
        DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer,
          vk::ShaderStageFlagBits::eVertex
        ),
        DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler,
          vk::ShaderStageFlagBits::eFragment
        )
      };

      atmosphere.descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

      atmosphere.pipelineLayout = device->createPipelineLayout( atmosphere.descriptorSetLayout, nullptr );

      vk::VertexInputBindingDescription binding( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex );

      PipelineVertexInputStateCreateInfo vertexInput( binding, {
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
      } );

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
        vk::CompareOp::eLessOrEqual, false, false, stencilOpState,
        stencilOpState, 0.0f, 0.0f );
      ;
      vk::PipelineColorBlendAttachmentState colorBlendAttachment( true,
        vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
        vk::BlendFactor::eOne, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
      );
      PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp,
        colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f }
      );
      PipelineDynamicStateCreateInfo dynamic( {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor
      } );

      atmosphere.pipeline = device->createGraphicsPipeline( _window->pipelineCache, {},
        { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
        viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
        atmosphere.pipelineLayout, _window->defaultRenderPass( )
      );

      // Init descriptor set
      atmosphere.descriptorSet = device->allocateDescriptorSet( dspPool, atmosphere.descriptorSetLayout );

      std::vector<WriteDescriptorSet> wdss =
      {
        WriteDescriptorSet(
          atmosphere.descriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer,
          1, nullptr, DescriptorBufferInfo( atmosphere.mvpBuffer, 0, sizeof( atmosphere.uboVS ) )
        ),
        WriteDescriptorSet(
          atmosphere.descriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, 1,
          tex3->descriptor, nullptr
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
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    diffuse.uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 5.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ) );
    diffuse.uboVS.view = camera.GetViewMatrix( );
    diffuse.uboVS.proj = glm::perspective( glm::radians( camera.Zoom ),
      ( float ) width / ( float ) height, 0.1f, 100.0f );
    diffuse.uboVS.proj[ 1 ][ 1 ] *= -1;

    diffuse.uboVS.cameraPos = camera.Position;

    // diffuse.uboVS.lightPos = glm::vec3( 2.5f, 2.5f, 2.5f );

    // change light position over time
    diffuse.uboVS.lightPos.x = sin( time ) * 3.0f;
    diffuse.uboVS.lightPos.z = cos( time ) * 2.0f;
    diffuse.uboVS.lightPos.y = 5.0 + cos( time ) * 1.0f;


    diffuse.mvpBuffer->set( &diffuse.uboVS );


    atmosphere.uboVS.mvp = glm::rotate( glm::mat4( 1.0f ), 0.75f * time * glm::radians( 5.0f ),
      glm::vec3( 0.0f, 1.0f, 0.0f ) );
    atmosphere.uboVS.mvp = glm::scale( atmosphere.uboVS.mvp, glm::vec3( 1.015f ) );
    atmosphere.uboVS.mvp = diffuse.uboVS.proj * diffuse.uboVS.view * atmosphere.uboVS.mvp;

    atmosphere.mvpBuffer->set( &atmosphere.uboVS );
  }

  void nextFrame( void ) override
  {
    if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }

    if ( Input::isKeyPressed( lava::Keyboard::Key::A ) )
    {
      diffuse.pc.normalScale -= 0.1f;
    }
    else if ( Input::isKeyPressed( lava::Keyboard::Key::S ) )
    {
      diffuse.pc.normalScale += 0.1f;
    }

    //std::cout << "NORMAL SCALE: " << pc.normalScale << std::endl;

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
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

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

    cmd->setViewportScissors( _window->getExtent( ) );

    // Earth
    {
      cmd->bindGraphicsPipeline( diffuse.pipeline );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        diffuse.pipelineLayout, 0, { diffuse.descriptorSet }, {} );

      cmd->pushConstants<Diffuse::PushConstant>( *diffuse.pipelineLayout,
        vk::ShaderStageFlagBits::eVertex, 0, diffuse.pc );
      geometry->render( cmd, 1 );
    }

    // Atmosphere
    {
      cmd->bindGraphicsPipeline( atmosphere.pipeline );
      cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        atmosphere.pipelineLayout, 0, { atmosphere.descriptorSet }, {} );

      geometry->render( cmd, 1 );
    }

    cmd->endRenderPass( );

    _window->frameReady( );
  }

private:
  VulkanWindow *_window;
  std::shared_ptr< Texture2D > tex, tex2, tex3;

  struct Diffuse
  {
    std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
    std::shared_ptr< DescriptorSet > descriptorSet;
    std::shared_ptr< PipelineLayout > pipelineLayout;
    std::shared_ptr< Pipeline > pipeline;
    std::shared_ptr< UniformBuffer > mvpBuffer;

    struct
    {
      glm::mat4 model;
      glm::mat4 view;
      glm::mat4 proj;
      glm::vec3 lightPos;
      glm::vec3 lightColor = glm::vec3( 0.5f, 0.5f, 0.0f );
      glm::vec3 cameraPos;
    } uboVS;

    struct PushConstant
    {
      float normalScale = 2.0f;
    } pc;
  } diffuse;

  struct
  {
    std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
    std::shared_ptr< DescriptorSet > descriptorSet;
    std::shared_ptr< PipelineLayout > pipelineLayout;
    std::shared_ptr< Pipeline > pipeline;
    std::shared_ptr< UniformBuffer > mvpBuffer;

    struct
    {
      glm::mat4 mvp;
    } uboVS;
  } atmosphere;

  std::shared_ptr< lava::extras::Geometry > geometry;
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
  {},
    &appInfo,
    layers.size( ),
    layers.data( ),
    extensions.size( ),
    extensions.data( )
  ) );

  CustomVkWindow w;
  w.setVulkanInstance( instance );
  w.resize( SCR_WIDTH, SCR_HEIGHT );

  w.show( );

  return 0;
}