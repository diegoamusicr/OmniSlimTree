#ifndef OMNISLIMTREESCENE_H
#define OMNISLIMTREESCENE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QPointF>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include "omnislimtree.h"

class SlimTreeScene: public QGraphicsScene
{
    Q_OBJECT
private:
    bool Drawing;
    QPointF QueryPoint;
    QGraphicsLineItem * QueryRadio;
    QGraphicsEllipseItem * QueryArea;
    qreal Radio;
    qreal RadioPunto;
    qreal RadioFoci;
    QVector <QGraphicsEllipseItem * > FociLowBounds;
    QVector <QGraphicsEllipseItem * > FociUpBounds;
public:
    SlimTreeScene(QObject * parent): QGraphicsScene(parent)
    {
        Drawing = false;
        QueryRadio = nullptr;
        QueryArea = nullptr;
        Drawing = false;
        RadioPunto = 1.25;
        RadioFoci = 10;
        this->setItemIndexMethod(QGraphicsScene::NoIndex);
    }
    void mousePressEvent(QGraphicsSceneMouseEvent * event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void DrawTree(SlimTree<QPointF> * ST);
    void PintarPuntosQuery(vector<QPointF> &puntos);
    void DrawFociRadios(SlimTree<QPointF> * ST, QPointF QueryP, qreal QueryR);
signals:
    void QueryDrew(QPointF, double);
    void PointCreated(QPointF);
};

#endif // OMNISLIMTREESCENE_H
