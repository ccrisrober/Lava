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

  quitButton = new QPushButton( tr( "&Quit" ) );
  quitButton->setFocusPolicy( Qt::NoFocus );

  comboModels = new QComboBox( );

  QStringList list;

  for ( int i = 0; i < 12; ++i )
  {
    list << QString( "mask" ) + QString::number( i );
  }

  comboModels->addItems( list );

  connect( comboModels, 
    static_cast<void ( QComboBox::* )( int )>(&QComboBox::currentIndexChanged),
    vw, &VulkanWindow::changeImage );
  connect( quitButton, &QPushButton::clicked, qApp, &QCoreApplication::quit );

  comboModels->setCurrentIndex( 0 );

  slider = new QSlider( Qt::Orientation::Vertical );
  slider->setMinimum( 0 );
  slider->setMaximum( 100 );

  slider->setValue( 50 );

  connect( slider,
    static_cast<void ( QSlider::* )( int )>( &QAbstractSlider::valueChanged ),
    vw, &VulkanWindow::changeMaskLevel );

  QGridLayout* layout = new QGridLayout( );

  layout->addWidget( comboModels, 0, 2 );
  layout->addWidget( slider, 1, 2 );
  layout->addWidget( quitButton, 2, 2 );
  layout->addWidget( wrapper, 0, 0, 7, 2 );
  setLayout( layout );
}
