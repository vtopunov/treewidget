#include "mainwidget.h"

const QString& treeFileName() {
	static const QString fileName("tree.xml");
	return fileName;
}

qint64 writeAll(const QString& fileName, const QByteArray& data) {
	QFile file(fileName);

	if (!file.open(QFile::WriteOnly)) {
		qWarning() << "can not open file " << fileName;
		return 0;
	}

	return file.write(data);
}

QByteArray readAll(const QString& fileName) {
	QFile file(fileName);

	if (!file.open(QFile::ReadOnly)) {
		qWarning() << "can not open file " << fileName;
		return {};
	}

	return file.readAll();
}

MainWidget::MainWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.tree->deserialize(readAll(treeFileName()));
}

void MainWidget::closeEvent(QCloseEvent*) {
	writeAll(treeFileName(), ui.tree->serialize());
}