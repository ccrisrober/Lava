#include <lava/lava.h>
using namespace lava;

#include <routes.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <routes.h>

class MyApp : public VulkanApp
{
public:
  std::shared_ptr<PipelineLayout> _pipelineLayout;
  std::shared_ptr<TextureCubemap> tex;

  struct
  {
    std::shared_ptr<Buffer> skybox;
    std::shared_ptr<Buffer> object;
  } uniformBuffers;

  struct UBOVS
  {
    glm::mat4 projection;
    glm::mat4 model;
  } uboVS;

  struct
  {
    std::shared_ptr<Pipeline> skybox;
    std::shared_ptr<Pipeline> object;
  } pipelines;

  struct
  {
    std::shared_ptr<DescriptorSet> skybox;
    std::shared_ptr<DescriptorSet> object;
  } descriptorSets;

  //uint32_t numIndices;

  std::shared_ptr<lava::extras::Geometry> geometry;
  std::shared_ptr<lava::extras::Geometry> skybox;

  MyApp( char const* title, uint32_t width, uint32_t height )
    : VulkanApp( title, width, height )
  {
    geometry = std::make_shared<lava::extras::Geometry>( _device,
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/monkey.obj_" ) );
    skybox = std::make_shared<lava::extras::Geometry>( _device,
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/skybox.obj_" ) );


    std::array< std::string, 6 > cubeImages =
    {
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string("/cubemap/right.jpg"),
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string("/cubemap/left.jpg"),
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string("/cubemap/top.jpg"),
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string("/cubemap/bottom.jpg"),
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string("/cubemap/back.jpg"),
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string("/cubemap/front.jpg"),
    };

    std::shared_ptr<CommandPool> commandPool = _device->createCommandPool(
      vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    tex = std::make_shared<TextureCubemap>( _device, cubeImages, commandPool, 
      _graphicsQueue );

    // Prepare and initialize uniform buffer containing shader uniforms
    {
      // Object vertex shader uniform buffer
      uint32_t mpBufferSize = sizeof(uboVS);
      uniformBuffers.object = _device->createBuffer( mpBufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer, 
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible | 
          vk::MemoryPropertyFlagBits::eHostCoherent );
      // Skybox vertex shader uniform buffer
      uniformBuffers.skybox = _device->createBuffer( mpBufferSize,
        vk::BufferUsageFlagBits::eUniformBuffer, 
        vk::SharingMode::eExclusive, nullptr,
        vk::MemoryPropertyFlagBits::eHostVisible | 
          vk::MemoryPropertyFlagBits::eHostCoherent );

      updateUniformBuffers( );
    }

    // SetupDescriptorSetLayout
    std::vector<DescriptorSetLayoutBinding> dslbs;
    // Binding 0 : Vertex shader uniform buffer
    dslbs.push_back( DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 
      vk::ShaderStageFlagBits::eVertex ) );
    // Binding 1 : Fragment shader image sampler
    dslbs.push_back( DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, 
      vk::ShaderStageFlagBits::eFragment ) );
    std::shared_ptr<DescriptorSetLayout> descriptorSetLayout = _device->createDescriptorSetLayout( dslbs );
    _pipelineLayout = _device->createPipelineLayout( descriptorSetLayout, nullptr );


    std::cout << "Preparing pipelines ..." << std::endl;
    // Prepare pipelines
    std::shared_ptr<PipelineCache> pipelineCache = _device->createPipelineCache( 0, nullptr );

    std::cout << "\tPreparing Skybox pipeline" << std::endl;
    // Skybox pipeline (background cube)
    // init shaders
    std::shared_ptr<ShaderModule> vertexShaderModule = _device->createShaderModule(
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/skybox_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    std::shared_ptr<ShaderModule> fragmentShaderModule = _device->createShaderModule(
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/skybox_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    // init pipeline
    PipelineShaderStageCreateInfo vertexStage( vk::ShaderStageFlagBits::eVertex, vertexShaderModule, "main" );
    PipelineShaderStageCreateInfo fragmentStage( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule, "main" );

    // Setup vertex description
    PipelineVertexInputStateCreateInfo vertexInput(
      vk::VertexInputBindingDescription( 0, sizeof( lava::extras::Vertex ),
        vk::VertexInputRate::eVertex ),
        {
          vk::VertexInputAttributeDescription( 0, 0, 
            vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, position ) ),
          vk::VertexInputAttributeDescription( 1, 0, 
            vk::Format::eR32G32B32Sfloat, offsetof( lava::extras::Vertex, normal ) ),
          vk::VertexInputAttributeDescription( 2, 0, 
            vk::Format::eR32G32Sfloat, offsetof( lava::extras::Vertex, texCoord ) )
        }
    );
    
    vk::PipelineInputAssemblyStateCreateInfo assembly( {}, vk::PrimitiveTopology::eTriangleList, VK_FALSE );
    vk::PipelineRasterizationStateCreateInfo rasterizationState( {}, true,
      false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack,
      vk::FrontFace::eCounterClockwise, false, 0.0f, 0.0f, 0.0f, 1.0f );

    PipelineViewportStateCreateInfo viewport( { {} }, { {} } );   // Dynamic viewport and scissor
    
    PipelineMultisampleStateCreateInfo multisample( vk::SampleCountFlagBits::e1, false, 0.0f, nullptr, false, false );
    vk::StencilOpState stencilOpState( vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways, 0, 0, 0 );
    vk::PipelineDepthStencilStateCreateInfo depthStencil( {}, false, false, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState, 0.0f, 0.0f );

    vk::PipelineColorBlendAttachmentState colorBlendAttachment( false, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eZero, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
      vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA );
    PipelineColorBlendStateCreateInfo colorBlend( false, vk::LogicOp::eNoOp, colorBlendAttachment, { 1.0f, 1.0f, 1.0f, 1.0f } );
    PipelineDynamicStateCreateInfo dynamic( { vk::DynamicState::eViewport, vk::DynamicState::eScissor } );


    std::cout << "\tCreating Skybox pipeline" << std::endl;
    pipelines.skybox = _device->createGraphicsPipeline( pipelineCache, {},
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterizationState, multisample, depthStencil, colorBlend, dynamic,
      _pipelineLayout, _renderPass );

    std::cout << "\tPreparing Model pipeline" << std::endl;
    // Model pipeline
    std::shared_ptr<ShaderModule> vertexShaderModule2 = _device->createShaderModule(
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/reflect_vert.spv" ), vk::ShaderStageFlagBits::eVertex );
    std::shared_ptr<ShaderModule> fragmentShaderModule2 = _device->createShaderModule(
      LAVA_EXAMPLES_RESOURCES_ROUTE + std::string( "/reflect_frag.spv" ), vk::ShaderStageFlagBits::eFragment );

    // init pipeline
    PipelineShaderStageCreateInfo vertexStage2( vk::ShaderStageFlagBits::eVertex, vertexShaderModule2, "main" );
    PipelineShaderStageCreateInfo fragmentStage2( vk::ShaderStageFlagBits::eFragment, fragmentShaderModule2, "main" );
    // Enable depth test and write
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthTestEnable = VK_TRUE;
    // Flip cull mode
    rasterizationState.cullMode = vk::CullModeFlagBits::eFront;


    std::cout << "\tCreating Model pipeline" << std::endl;
    pipelines.object = _device->createGraphicsPipeline( pipelineCache, {},
    { vertexStage2, fragmentStage2 }, vertexInput, assembly, nullptr,
      viewport, rasterizationState, multisample, depthStencil, colorBlend, dynamic,
      _pipelineLayout, _renderPass );


    // Setup descriptor pool
    std::array<vk::DescriptorPoolSize, 2> poolSize;
    poolSize[ 0 ] = vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 2 );
    poolSize[ 1 ] = vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 2 );
    std::shared_ptr<DescriptorPool> descriptorPool = _device->createDescriptorPool(
      {}, 2, poolSize );

    // Setup descriptor sets
    // Image descriptor for the cube map textur
    DescriptorImageInfo textureDescriptor( 
      vk::ImageLayout::eGeneral, 
      std::make_shared<vk::ImageView>( tex->view ), 
      std::make_shared<vk::Sampler>( tex->sampler )
    );
    // 3D object descriptor set
    descriptorSets.object = _device->allocateDescriptorSet( descriptorPool, descriptorSetLayout );
    
    std::vector<WriteDescriptorSet> wdss;
    // Binding 0 : Vertex shader uniform buffer
    wdss.push_back(
      WriteDescriptorSet( descriptorSets.object, 0, 0, vk::DescriptorType::eUniformBuffer, 1, nullptr,  
      DescriptorBufferInfo( uniformBuffers.object, 0, sizeof( uboVS ) ) )
    );
    // Binding 1 : Fragment shader cubemap sampler
    wdss.push_back(
      WriteDescriptorSet( descriptorSets.object, 0, 0, vk::DescriptorType::eCombinedImageSampler, 1, 
      DescriptorImageInfo( 
        vk::ImageLayout::eGeneral, 
        std::make_shared<vk::ImageView>( tex->view ), 
        std::make_shared<vk::Sampler>( tex->sampler )
      ), nullptr
    ));
    _device->updateDescriptorSets( wdss, {} );

    // Skybox descriptor set

    wdss.clear( );
    // Binding 0 : Vertex shader uniform buffer
    wdss.push_back(
      WriteDescriptorSet( descriptorSets.skybox, 0, 0, vk::DescriptorType::eUniformBuffer, 1, nullptr,  
      DescriptorBufferInfo( uniformBuffers.skybox, 0, sizeof( uboVS ) ) )
    );
    // Binding 1 : Fragment shader cubemap sampler
    wdss.push_back(
      WriteDescriptorSet( descriptorSets.skybox, 0, 0, vk::DescriptorType::eCombinedImageSampler, 1, 
      DescriptorImageInfo( 
        vk::ImageLayout::eGeneral, 
        std::make_shared<vk::ImageView>( tex->view ), 
        std::make_shared<vk::Sampler>( tex->sampler )
      ), nullptr
    ));
    _device->updateDescriptorSets( wdss, {} );
  }
  void updateUniformBuffers( void )
  {
    uint32_t width = _window->getWidth( );
    uint32_t height = _window->getHeight( );

    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

    vk::Device device = static_cast<vk::Device>(*_device);

    // 3D object
    uboVS.projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.001f, 256.0f);
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    uboVS.model = viewMatrix * glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    uint32_t mpBufferSize = sizeof(uboVS);
    void* data = uniformBuffers.object->map( 0, mpBufferSize );
    memcpy( data, &uboVS, sizeof(uboVS) );
    uniformBuffers.object->unmap( );

    // Skybox
    viewMatrix = glm::mat4();
    uboVS.projection = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.001f, 256.0f);
    uboVS.model = viewMatrix * glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    mpBufferSize = sizeof(uboVS);
    data = uniformBuffers.skybox->map( 0, mpBufferSize );
    memcpy( data, &uboVS, sizeof(uboVS) );
    uniformBuffers.skybox->unmap( );
  }

  void doPaint( void ) override
  {
    std::cout << "Updating buffer" << std::endl;
    updateUniformBuffers( );

    // create a command pool for command buffer allocation
    std::shared_ptr<CommandPool> commandPool = 
      _device->createCommandPool( 
          vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _queueFamilyIndex );
    std::shared_ptr<CommandBuffer> commandBuffer = commandPool->allocateCommandBuffer( );

    commandBuffer->begin( );

    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
    commandBuffer->beginRenderPass( _renderPass, 
      _defaultFramebuffer->getFramebuffer( ), 
      vk::Rect2D( { 0, 0 }, 
      _defaultFramebuffer->getExtent( ) ),
      { vk::ClearValue( ccv ), vk::ClearValue( 
        vk::ClearDepthStencilValue( 1.0f, 0 ) )
      }, vk::SubpassContents::eInline );
    commandBuffer->setViewport( 0, vk::Viewport( 0.0f, 0.0f,
      ( float ) _defaultFramebuffer->getExtent( ).width,
      ( float ) _defaultFramebuffer->getExtent( ).height, 0.0f, 1.0f ) );
    commandBuffer->setScissor( 0, vk::Rect2D( { 0, 0 }, _defaultFramebuffer->getExtent( ) ) );


    // Skybox
    {
      commandBuffer->bindGraphicsPipeline( pipelines.skybox );
      commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        _pipelineLayout, 0, { descriptorSets.skybox }, nullptr );
      skybox->render( commandBuffer );
    }

    // Model render
    {
      commandBuffer->bindGraphicsPipeline( pipelines.object );
      commandBuffer->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
        _pipelineLayout, 0, { descriptorSets.object }, nullptr );
      geometry->render( commandBuffer );
    }
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
        glfwSetWindowShouldClose( getWindow( )->getWindow( ), GLFW_TRUE );
        break;
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

int main( void )
{
  try
  {
    //if (glfwInit())
    //{
    VulkanApp* app = new MyApp( "Cube Mesh", 800, 600 );

    app->getWindow( )->setErrorCallback( glfwErrorCallback );

    while ( app->isRunning( ) )
    {
      app->waitEvents( );
      app->paint( );
    }

    delete app;
    //}
  }
  catch ( std::system_error err )
  {
    std::cout << "System Error: " << err.what( ) << std::endl;
  }
  system( "PAUSE" );
  return 0;
}