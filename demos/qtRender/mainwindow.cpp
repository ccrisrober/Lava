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

#include "mainwindow.h"

MainWindow::MainWindow( VulkanWindow* window, QWidget* parent )
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

  /**/
  vulkanWidget = QWidget::createWindowContainer( window );
  vulkanWidget->setMinimumSize( QSize( 0, 500 ) );

  verticalLayout->addWidget( vulkanWidget );
  /**/

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
  connect( pushButton, SIGNAL( clicked( ) ), SLOT( handleClick( ) ) );
}

void MainWindow::handleClick( void )
{
  std::cout << "CLICK" << std::endl;


  if ( !_window->supportGrab( ) )
  {
    QMessageBox::warning( this, tr( "Cannot grab" ), tr( "Swapchain doesn't support readbacks." ) );
    return;
  }
  QImage img;// = _window->grab( );
  QFileDialog fd( this );
  fd.setAcceptMode( QFileDialog::AcceptSave );
  fd.setDefaultSuffix( "png" );
  if ( fd.exec( ) == QDialog::Accepted )
  {
    img.save( fd.selectedFiles( ).first( ) );
  }

  //_window->checkButton( );
}