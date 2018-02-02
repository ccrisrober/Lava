#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <lava/lava.h>
#include <qtLava/qtLava.h>
using namespace lava;


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <routes.h>

class QCustomRenderer : public QVulkanWindowRenderer
{
public:
  QCustomRenderer( lava::QVulkanWindow *w )
    : QVulkanWindowRenderer( )
    , _window( w )
  {
    _window->setTitle( "Cube textured" );
  }

  struct
  {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  } uboVS;

  struct Vertex
  {
    glm::vec3 pos;
    glm::vec2 texCoord;
  };

  const float side = 1.0f;
  const float side2 = side * 0.5f;

  const std::vector<Vertex> vertices =
  {
    { { -side2, -side2,  side2 },{ 0.0f, 0.0f } },
    { { side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { -side2,  side2,  side2 },{ 0.0f, 1.0f } },
    { { side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { side2, -side2, -side2 },{ 1.0f, 0.0f } },
    { { -side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { side2,  side2, -side2 },{ 1.0f, 1.0f } },

    { { side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { -side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { -side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { -side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2,  side2, -side2 },{ 0.0f, 0.0f } },
    { { -side2,  side2,  side2 },{ 1.0f, 0.0f } },
    { { side2,  side2, -side2 },{ 0.0f, 1.0f } },
    { { side2,  side2,  side2 },{ 1.0f, 1.0f } },

    { { -side2, -side2, -side2 },{ 0.0f, 0.0f } },
    { { -side2, -side2,  side2 },{ 1.0f, 0.0f } },
    { { side2, -side2, -side2 },{ 0.0f, 1.0f } },
    { { side2, -side2,  side2 },{ 1.0f, 1.0f } }
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
      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );

      vertexBuffer = device->createBuffer( vertexBufferSize,
        vk::BufferUsageFlagBits::eVertexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      vertexBuffer->update<Vertex>( cmd, 0, { uint32_t( vertices.size( ) ),
        vertices.data( ) } );
      cmd->end( );
      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // Index buffer
    {
      uint32_t indexBufferSize = indices.size( ) * sizeof( uint32_t );
      auto cmd = _window->gfxCommandPool( )->allocateCommandBuffer( );
      cmd->begin( );

      indexBuffer = device->createBuffer( indexBufferSize,
        vk::BufferUsageFlagBits::eIndexBuffer |
        vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal );
      indexBuffer->update<uint16_t>( cmd, 0, { uint32_t( indices.size( ) ),
        indices.data( ) } );
      cmd->end( );
      _window->gfxQueue( )->submitAndWait( cmd );
    }

    // MVP buffer
    {
      mvpBuffer = device->createUniformBuffer( sizeof( uboVS ) );
    }

    tex = device->createTexture2D( LAVA_EXAMPLES_IMAGES_ROUTE +
      std::string( "uv_checker.png" ), _window->gfxCommandPool( ),
      _window->gfxQueue( ), vk::Format::eR8G8B8A8Unorm );

    auto vertexStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_vert.spv" ),
      vk::ShaderStageFlagBits::eVertex
    );
    auto fragmentStage = device->createShaderPipelineShaderStage(
      LAVA_EXAMPLES_SPV_ROUTE + std::string( "cubeUV_frag.spv" ),
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

    descriptorSetLayout = device->createDescriptorSetLayout( dslbs );

    pipelineLayout = device->createPipelineLayout( descriptorSetLayout, nullptr );

    vk::VertexInputBindingDescription binding( 0, sizeof( Vertex ),
      vk::VertexInputRate::eVertex );

    PipelineVertexInputStateCreateInfo vertexInput( binding, {
      vk::VertexInputAttributeDescription(
        0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, pos )
      ),
      vk::VertexInputAttributeDescription(
        1, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, texCoord )
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

    pipeline = device->createGraphicsPipeline( nullptr, { },
    { vertexStage, fragmentStage }, vertexInput, assembly, nullptr,
      viewport, rasterization, multisample, depthStencil, colorBlend, dynamic,
      pipelineLayout, _window->defaultRenderPass( )
    );

    std::array<vk::DescriptorPoolSize, 2> poolSize =
    {
      vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, 1 ),
      vk::DescriptorPoolSize( vk::DescriptorType::eCombinedImageSampler, 1 )
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
        tex->descriptor, nullptr
      )
    };
    device->updateDescriptorSets( wdss, { } );
  }

  void updateMVP( void )
  {
    QSize size = _window->size( );

    uint32_t width = size.width( ), height = size.height( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >( currentTime - startTime ).count( ) / 1000.0f;

    uboVS.model = glm::rotate( glm::mat4( 1.0f ), time * glm::radians( 90.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    uboVS.model = glm::rotate( uboVS.model, time * glm::radians( 45.0f ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

    uboVS.view = glm::lookAt( glm::vec3( 2.0f, 2.0f, 2.0f ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
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
  }


  void nextFrame( void ) override
  {
    /*if ( Input::isKeyPressed( lava::Keyboard::Key::Esc ) )
    {
      _window->_window->close( );
    }*/

    updateMVP( );

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { 0.2f, 0.3f, 0.3f, 1.0f };
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

    cmd->bindGraphicsPipeline( pipeline );
    cmd->bindDescriptorSets( vk::PipelineBindPoint::eGraphics,
      pipelineLayout, 0, { descriptorSet }, { } );

    cmd->bindVertexBuffer( 0, vertexBuffer, 0 );
    cmd->bindIndexBuffer( indexBuffer, 0, vk::IndexType::eUint16 );

    auto size_ = _window->size( );
    vk::Extent2D ext( size_.width( ), size_.height( ) );
    cmd->setViewportScissors( ext );
    cmd->drawIndexed( indices.size( ), 1, 0, 0, 1 );

    cmd->endRenderPass( );

    _window->frameReady( );
    _window->requestUpdate( );
  }

private:
  QVulkanWindow *_window;
  std::shared_ptr< Texture2D > tex;
  std::shared_ptr< DescriptorSetLayout > descriptorSetLayout;
  std::shared_ptr< DescriptorSet > descriptorSet;
  std::shared_ptr< PipelineLayout > pipelineLayout;
  std::shared_ptr< Pipeline > pipeline;
  std::shared_ptr< Buffer > vertexBuffer;
  std::shared_ptr< Buffer > indexBuffer;
  std::shared_ptr< UniformBuffer > mvpBuffer;
};

class VulkanWindow: public lava::QVulkanWindow
{
public:
  lava::QVulkanWindowRenderer* createRenderer( void ) override
  {
    return new QCustomRenderer( this );
  }
public slots:
  void checkButton( void )
  {
    std::cout << "Hello Buttton" << std::endl;
  }
};

class MainWindow : public QMainWindow
{
  //Q_OBJECT
public:
  MainWindow( VulkanWindow* window, QWidget *parent = 0 )
    : QMainWindow( parent )
    , _window( window )
  {
    resize( 400, 300 );
    centralWidget = new QWidget( this );
    verticalLayout_2 = new QVBoxLayout( centralWidget );
    verticalLayout_2->setSpacing( 6 );
    verticalLayout_2->setContentsMargins( 11, 11, 11, 11 );
    verticalLayout = new QVBoxLayout( );
    verticalLayout->setSpacing( 6 );
    vulkanWidget = QWidget::createWindowContainer( window );
    vulkanWidget->setMinimumSize( QSize( 0, 500 ) );

    verticalLayout->addWidget( vulkanWidget );

    horizontalLayout = new QHBoxLayout( );
    horizontalLayout->setSpacing( 6 );

    pushButton = new QPushButton( centralWidget );

    horizontalLayout->addWidget( pushButton );

    _closeButton = new QPushButton( centralWidget );

    horizontalLayout->addWidget( _closeButton );


    verticalLayout->addLayout( horizontalLayout );


    verticalLayout_2->addLayout( verticalLayout );

    label_2 = new QLabel( centralWidget );
    label_2->setAlignment( Qt::AlignCenter );
    label_2->setMinimumHeight( 25 );
    label_2->setMaximumHeight( 25 );

    verticalLayout_2->addWidget( label_2 );

    setCentralWidget( centralWidget );
    menuBar = new QMenuBar( this );
    menuBar->setGeometry( QRect( 0, 0, 400, 21 ) );
    setMenuBar( menuBar );
    mainToolBar = new QToolBar( this );
    addToolBar( Qt::TopToolBarArea, mainToolBar );
    statusBar = new QStatusBar( this );
    setStatusBar( statusBar );


    setWindowTitle( QApplication::translate( "MainWindow", "MainWindow", nullptr ) );
    pushButton->setText( QApplication::translate( "MainWindow", "Foo Button", nullptr ) );
    _closeButton->setText( QApplication::translate( "MainWindow", "Close Button", nullptr ) );
    label_2->setText( QApplication::translate( "MainWindow", "FooLabel", nullptr ) );


    connect( _closeButton, &QPushButton::clicked, &QCoreApplication::quit );
  }
  public slots:
  void handleClick( )
  {
    dynamic_cast<VulkanWindow*>( _window)->checkButton( );
  }
private:
  VulkanWindow* _window;
  QWidget *centralWidget;
  QVBoxLayout *verticalLayout_2;
  QVBoxLayout *verticalLayout;
  QWidget *vulkanWidget;
  QHBoxLayout *horizontalLayout;
  QPushButton *pushButton;
  QPushButton *_closeButton;
  QLabel *label_2;
  QMenuBar *menuBar;
  QToolBar *mainToolBar;
  QStatusBar *statusBar;
};

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

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

  VulkanWindow vw;
  vw.setQVulkanInstance( instance );
  // QApplication app(argc, argv);
  MainWindow w( &vw );
  w.show();
  /*VulkanWindow w;
  w.setQVulkanInstance( instance );

  w.resize( 500, 500 );
  w.show( );*/

  return app.exec();
}
