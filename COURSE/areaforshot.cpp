#include "areaforshot.h"

AreaForShot::AreaForShot(QObject *parent) : QObject(parent)
{
	QPixmap texture;
	texture.load(":/img/cursortarget.png");

	texture = texture.scaled(43, 43);

	QCursor MyCursor = QCursor(texture);
	this->setCursor(MyCursor);
}

void AreaForShot::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
	/*
	 * посылаем сигнал о том, что был произведён выстрел,
	 * и отправляем координаты выстрела в слот SendFire(), описанный в файле mainwindow.cpp,
	 * для дальнейшей отправки на сервер.
	*/
	emit fire(event->pos().x(), event->pos().y());
}

QRectF AreaForShot::boundingRect() const
{
	//Формируем квадрат поля противника для вычисления координат.
	return QRectF(0, 0, WidthOfFrame, HeightOfFrame);
}

void AreaForShot::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(painter);
	Q_UNUSED(option);
	Q_UNUSED(widget);
}
