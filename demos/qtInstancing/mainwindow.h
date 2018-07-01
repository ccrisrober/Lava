#pragma once

#include <QtWidgets/QSlider.h>
#include <QtWidgets/QPushButton.h>

class VulkanWindow;

class MainWindow: public QWidget
{
public:
	MainWindow( VulkanWindow* vw );
private:
	QSlider* slider;
	QPushButton* quitButton;
};