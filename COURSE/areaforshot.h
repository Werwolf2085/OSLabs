#ifndef AREAFORSHOT_H
#define AREAFORSHOT_H

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <stdint.h>

#define WidthOfFrame 580
#define HeightOfFrame 415

class AreaForShot : public QObject, public QGraphicsItem
{
	Q_OBJECT
public:
	explicit AreaForShot(QObject* parent = 0);

private:
	QRectF boundingRect() const;
	void mousePressEvent(QGraphicsSceneMouseEvent* event);
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

signals:
	void fire(int16_t x, int16_t y); //Сигнал отправляет координаты x и y в слот SendFire(), описанный в mainwindow.cpp, для отправки на сервер.

public slots:

private slots:
};

#endif // AREAFORSHOT_H
