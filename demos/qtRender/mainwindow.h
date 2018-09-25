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

#ifndef __POMPEII_QTRENDER_MAINWINDOW_H__
#define __POMPEII_QTRENDER_MAINWINDOW_H__


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

#include <qtPompeii/qtPompeii.h>

class MainWindowRenderer : public pompeii::qt::VulkanWindowRenderer
{
private:
  pompeii::qt::VulkanWindow* _window;
public:
  MainWindowRenderer( pompeii::qt::VulkanWindow* window )
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

class VulkanWindow : public pompeii::qt::VulkanWindow
{
  Q_OBJECT
private:
public:
  VulkanWindow( QWindow* parent = nullptr )
    : pompeii::qt::VulkanWindow( parent )
  {
  }
  virtual pompeii::qt::VulkanWindowRenderer* createRenderer( void )
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

#endif /* __POMPEII_QTRENDER_MAINWINDOW_H__ */