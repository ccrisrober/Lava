#ifndef __LAVA_QTRENDER_MAINWINDOW_H__
#define __LAVA_QTRENDER_MAINWINDOW_H__


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

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>

#include <qtLava/qtLava.h>

class MainWindowRenderer : public lava::QtVulkanWindowRenderer
{
private:
  lava::QtVulkanWindow* _window;
public:
  MainWindowRenderer( lava::QtVulkanWindow* window )
    : _window( window )
  {
  }

  virtual void nextFrame( void )
  {
    auto cmd = _window->currentCommandBuffer( );

    static auto startTime = std::chrono::high_resolution_clock::now( );

    auto currentTime = std::chrono::high_resolution_clock::now( );
    float time = std::chrono::duration_cast< std::chrono::milliseconds >(
      currentTime - startTime ).count( ) / 1000.0f;

    float _red = sin( time ) * 0.5f + 0.5f;
    float _blue = cos( time ) * 0.5f + 0.5f;

    std::array<vk::ClearValue, 2 > clearValues;
    std::array<float, 4> ccv = { _red, 0.0f, _blue, 1.0f };

    clearValues[ 0 ].color = vk::ClearColorValue( ccv );
    clearValues[ 1 ].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    vk::Extent2D extent = _window->swapchainImageSize( );

    cmd->beginRenderPass( _window->renderPass( ),
      _window->framebuffer( ),
      vk::Rect2D( { 0, 0 }, extent ), clearValues,
      vk::SubpassContents::eInline );

    cmd->endRenderPass( );

    _window->frameReady( );
  }
};

class VulkanWindow : public lava::QtVulkanWindow
{
  Q_OBJECT
private:
public:
  VulkanWindow( QWindow* parent = nullptr )
    : lava::QtVulkanWindow( parent )
  {
  }
  virtual lava::QtVulkanWindowRenderer* createRenderer( void )
  {
    return new MainWindowRenderer( this );
  }
public slots:
  void checkButton( void )
  {
    std::cout << "Hello Buttton" << std::endl;
  }
};


class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  MainWindow( VulkanWindow* window, QWidget* parent = 0 );
private slots:
  void handleClick( void);
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

#endif /* __LAVA_QTRENDER_MAINWINDOW_H__ */