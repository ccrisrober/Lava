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

#include <iostream>

#include <qtPompeii/qtPompeii.h>

#include <QApplication>

#include "mainwindow.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  /**
  VulkanWindow vw( engine );
  vw.setVkInstance( engine->GetVkInstance( ) );
  vw.setWidth( 640 );
  vw.setHeight( 480 );
  vw.show( );
  /**/

  /**/
  VulkanWindow* vw = new VulkanWindow( );
  std::vector< int > supportedSamples = vw->supportedSampleCounts( );
  vw->setSampleCountFlagBits( supportedSamples.at( 0 ) );
  MainWindow w ( vw );
  w.show( );

  /**/
  return app.exec( );
}