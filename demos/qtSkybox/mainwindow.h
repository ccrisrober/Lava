#pragma once

#include <QtWidgets/QCheckBox.h>
#include <QtWidgets/QPushButton.h>
#include <QtWidgets/QComboBox.h>

class VulkanWindow;

class MainWindow: public QWidget
{
public:
	MainWindow( VulkanWindow* vw );
private:
	QCheckBox* meshSwitch;
  QComboBox* comboModels;
  QPushButton* pauseButton;
	QPushButton* quitButton;
};