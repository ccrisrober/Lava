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
    device->updateDescriptorSets( wdss, { } );
  }



  void initResources( void ) override
  {
    auto device = _window->device( );

    geometry = std::make_shared<pompeii::utility::Geometry>( device,
      POMPEII_EXAMPLES_MESHES_ROUTE + std::string( "wolf.obj_" ) );

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
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/MatCap_Toon3.png" ),
      pipelineLayouts.matcapLeft,
      descriptorSetLayout,
      descriptorSets.matcapLeft, textures.matcapLeft
    );

    createMaterial(
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/green_matcap.jpg" ),
      pipelineLayouts.matcapCenter,
      descriptorSetLayout,
      descriptorSets.matcapCenter, textures.matcapCenter
    );

    createMaterial(
      POMPEII_EXAMPLES_IMAGES_ROUTE + std::string( "/rubymatcap.jpg" ),
      pipelineLayouts.matcapRight,
      descriptorSetLayout,
      descriptorSets.matcapRight, textures.matcapRight
    );

    auto vertexStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "matcap_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      POMPEII_EXAMPLES_SPV_ROUTE + std::string( "matcap_frag.spv" ),
      vk::ShaderStageFlagBits::eFragment
    );

    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( pompeii::utility::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, vk::Format::eR32G32B32Sfloat,
          offsetof( pompeii::utility::Vertex, position ) ),
          vk::VertexInputAttributeDescription( 1, 0, vk::Format::eR32G32B32Sfloat,
          offsetof( pompeii::utility::Vertex, normal ) )
        }
    );
    vk::PipelineInputAssemblyStateCreateInfo assembly( { }, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    PipelineViewportStateCreateInfo viewport( { { } }, { { } } );   // one dummy viewport and scissor, as dynamic state sets them
    vk::PipelineRasterizationStateCreateInfo rasterization( { }, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( { }, true, true, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );
    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );


    pipelines.matcapLeft = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport,
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.matcapLeft, _window->renderPass( )
    );

    pipelines.matcapCenter = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport,
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.matcapCenter, _window->renderPass( )
    );

    pipelines.matcapRight = device->createGraphicsPipeline( _window->pipelineCache( ), { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr, viewport,
      rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayouts.matcapRight, _window->renderPass( )
    );
  }

  void updateUniformBuffers( void )
  {
    auto size = _window->swapchainImageSize( );

    uint32_t width = size.width, height = size.height;

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast<std::chrono::milliseconds>( currentTime - startTime ).count( ) / 1000.0f;

    ubo.model = glm::translate( glm::mat4( 1.0f ), 
      glm::vec3( 2.5f * std::sin( time ), 0.0f, 1.0f ) );
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
    updateUniformBuffers( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.0f, 0.0f, 0.0f, 1.0f };
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

    uint32_t width = size.width, 
             height = size.height;

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

    _window->frameReady( );
  }
private:
  std::shared_ptr<Buffer> uniformBufferMVP;

  std::shared_ptr<pompeii::utility::Geometry> geometry;

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
  VulkanWindow app( 1024, 768, "Multidescriptor", true );
  app.show( );
  return EXIT_SUCCESS;
}