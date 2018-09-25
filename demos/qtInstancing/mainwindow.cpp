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

#include "vulkanwindow.h"

#include <QApplication>
#include <QGridLayout>
#include <QtGlobal>

MainWindow::MainWindow( VulkanWindow * vw )
{
  QWidget* wrapper = QWidget::createWindowContainer( vw );
  wrapper->setFocusPolicy( Qt::StrongFocus );
  wrapper->setFocus( );

  slider = new QSlider( );
  slider->setFocusPolicy( Qt::NoFocus ); // do not interfere with vulkanWindow's keyboard input

  slider->setMinimum( 0 );
  slider->setMaximum( 50000 );

  slider->setValue( slider->maximum( ) );

  connect( slider,
    static_cast<void ( QSlider::* )( int )>( &QAbstractSlider::valueChanged ),
    vw, &VulkanWindow::changeNumInstancing );

  quitButton = new QPushButton( tr( "&Quit" ) );
  quitButton->setFocusPolicy( Qt::NoFocus );
  
  connect( quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit );

  QGridLayout* layout = new QGridLayout( );

  layout->addWidget( slider, 0, 2 );
  layout->addWidget( quitButton, 1, 2 );
  layout->addWidget( wrapper, 0, 0, 7, 2 );
  setLayout( layout );
}
