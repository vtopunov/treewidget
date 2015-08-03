#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QtWidgets>

#include "ui_mainwidget.h"

class MainWidget : public QWidget
{
	Q_OBJECT

public:
	MainWidget(QWidget *parent = 0);

protected:
	virtual void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;

private:
	Ui::MainWidget ui;
};

#endif // MAINWIDGET_H
