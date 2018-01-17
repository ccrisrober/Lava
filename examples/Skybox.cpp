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

const unsigned int SCR_WIDTH = 500;
const unsigned int SCR_HEIGHT = 500;

class CustomRenderer : public VulkanWindowRenderer
{
public:
  CustomRenderer( lava::VulkanWindow *w )
    : VulkanWindowRenderer( )
    , _window( w )
  {
    _window->setWindowTitle( "Skybox with refract/reflect" );
    camera = Camera( glm::vec3( 0.0f, 2.0f, 8.0f ) );
  }

  // camera
  Camera camera;
  // timing
  float deltaTime = 0.0f; // time between current frame and last frame
  float lastFrame = 0.0f;

  float lastX = SCR_WIDTH * 0.5f;
  float lastY = SCR_HEIGHT * 0.5f;
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
  const float side2 = side * 0.5f;
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
      auto cmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );
      cmd->beginSimple( );

      skybox.vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      skybox.vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ),
        vertices.data( ) } );
      cmd->end( );
      _window->graphicQueue( )->submitAndWait( cmd );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );
      auto cmd = _window->graphicsCommandPool( )->allocateCommandBuffer( );
      cmd->beginSimple( );

      skybox.indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      skybox.indexBuffer->update<uint16_t>( cmd, 0, { uint32_t( indices.size( ) ),
        indices.data( ) } );
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
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/right.png" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/left.png" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/top.png" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/bottom.png" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/back.png" ),
      LAVA_EXAMPLES_IMAGES_ROUTE + std::string( "/spaceCubemap/front.png" ),
    };

    tex = device->createTextureCubemap( cubeImages, 
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
            vk::ImageLayout::eGeneral, tex->view, tex->sampler
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

    uniformMVP->set( &uboVS );

    uboFS.viewPos = camera.Position;
    uniformViewPos->set( &uboFS );
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
      lava::utils::saveScreenshot( _window->device( ), "file.ppm", 
        size.x, size.y, _window->colorFormat( ),
        // Source for the copy is the last rendered swapchain image
        _window->defaultFramebuffer( )->getLastImage( ),
        _window->graphicsCommandPool( ), _window->graphicQueue( )
      );
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
  w.resize( SCR_WIDTH, SCR_HEIGHT );

  w.show( );

  return 0;
}