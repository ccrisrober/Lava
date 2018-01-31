
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QMainWindow>
#include <QHBoxLayout>

#include <lava/lava.h>
#include <qtLava/qtLava.h>
using namespace lava;

class VulkanWindow: public lava::QVulkanWindow
{
public:
  //lava::QVulkanWindowRenderer* createRenderer( void ) override
  //{
  //  return QCustomRenderer( this );
  //}
public slots:
  void checkButton( void )
  {
    std::cout << "Hello Buttton" << std::endl;
  }
};

class MainWindow : public QMainWindow
{
public:
  MainWindow( VulkanWindow* window, QWidget *parent = 0 )
    : QMainWindow( parent )
  {
    resize(400, 471);

    centralWidget = new QWidget( this );
    centralWidget->setObjectName(QStringLiteral("centralWidget"));

    verticalLayoutWidget = new QWidget(centralWidget);
    verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
    verticalLayoutWidget->setGeometry(QRect(10, 10, 381, 401));
    verticalLayout = new QVBoxLayout(verticalLayoutWidget);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    vulkanWidget = /*new QLabel( tr("WIDGET" ) ); //*/ QWidget::createWindowContainer( window );
    vulkanWidget->setObjectName(QStringLiteral("vulkanWidget"));
    vulkanWidget->setMinimumSize(QSize(0, 350));

    verticalLayout->addWidget(vulkanWidget);

    _label = new QLabel(verticalLayoutWidget);
    _label->setObjectName(QStringLiteral("_label"));
    _label->setAlignment(Qt::AlignCenter);

    verticalLayout->addWidget(_label);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setSpacing(6);
    horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
    _button = new QPushButton(verticalLayoutWidget);
    _button->setObjectName(QStringLiteral("_button"));

    horizontalLayout->addWidget(_button);

    _closeButton = new QPushButton(verticalLayoutWidget);
    _closeButton->setObjectName(QStringLiteral("_closeButton"));

    horizontalLayout->addWidget(_closeButton);


    verticalLayout->addLayout(horizontalLayout);


    _label->setText(QApplication::translate("MainWindow", "Foo Label", Q_NULLPTR));
    _button->setText(QApplication::translate("MainWindow", "Foo Button", Q_NULLPTR));
    _closeButton->setText(QApplication::translate("MainWindow", "Close Button", Q_NULLPTR));

    //connect( _button, &QPushButton::clicked, &VulkanWindow::checkButton );
    connect( _closeButton, &QPushButton::clicked, &QCoreApplication::quit );

    setCentralWidget(centralWidget);
  }
private:
  QWidget *centralWidget;
  QWidget *verticalLayoutWidget;
  QVBoxLayout *verticalLayout;
  QWidget *vulkanWidget;
  QLabel *_label;
  QHBoxLayout *horizontalLayout;
  QPushButton *_button;
  QPushButton *_closeButton;
};

int main( int argc, char** argv )
{
  QGuiApplication app( argc, argv );

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

  /*VulkanWindow vw;
  vw.setVulkanInstance( instance );

  QApplication app(argc, argv);
  MainWindow w( &vw );
  w.show()*/;
  VulkanWindow w;
  w.setVulkanInstance( instance );

  w.resize( 500, 500 );
  w.show( );
  return app.exec();
}
