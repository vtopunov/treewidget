#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtWidgets>

#include "ui_mainwidget.h"

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget(QWidget *parent = 0);

private:
	Ui::MainWidget ui;
};

#endif // MAINWIDGET_H
