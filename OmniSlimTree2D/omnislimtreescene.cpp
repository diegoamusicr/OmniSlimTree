#include "omnislimtreescene.h"

void SlimTreeScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Drawing = true;
    QueryPoint = event->scenePos();
    QGraphicsScene::mousePressEvent(event);
}

void SlimTreeScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (Drawing)
    {
        if (!QueryRadio)
        {
            QueryRadio = new QGraphicsLineItem();
            this->addItem(QueryRadio);
            QueryRadio->setPen(QPen(Qt::red, 1.5, Qt::DotLine));
            QueryRadio->setPos(QueryPoint);
        }
        if (!QueryArea)
        {
            QueryArea = new QGraphicsEllipseItem();
            this->addItem(QueryArea);
            QueryArea->setPen(QPen(Qt::red, 1, Qt::SolidLine));
            QueryArea->setPos(QueryPoint);
            Radio = 0;
        }
        Radio = sqrt( pow(event->scenePos().rx() - QueryPoint.rx(),2) + pow(event->scenePos().ry() - QueryPoint.ry(),2) );
        QueryRadio->setLine(0, 0, event->scenePos().rx() - QueryPoint.rx(), event->scenePos().ry() - QueryPoint.ry());
        QueryArea->setRect(-Radio, -Radio, Radio*2, Radio*2);
    }
    else QGraphicsScene::mouseMoveEvent(event);
}

void SlimTreeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QueryRadio = nullptr;
    QueryArea = nullptr;
    Drawing = false;
    emit QueryDrew(QueryPoint, Radio);
    Radio = 0;
    QGraphicsScene::mouseReleaseEvent(event);
}

void SlimTreeScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit PointCreated(event->scenePos());
    QGraphicsScene::mouseDoubleClickEvent(event);
}

void SlimTreeScene::DrawTree(SlimTree<QPointF> *ST)
{
    pair< vector< pair<QPointF, stDist> >, vector<QPointF> > TreeData = ST->GetTreeData();
    for (int i = 0; i < TreeData.first.size(); i++)
    {
        stDist R = TreeData.first[i].second;
        QGraphicsEllipseItem * A = new QGraphicsEllipseItem();
        this->addItem(A);
        A->setPen(QPen(Qt::black, 1, Qt::SolidLine));
        A->setPos(TreeData.first[i].first);
        A->setRect(-R, -R, R*2, R*2);
        QGraphicsLineItem * B = new QGraphicsLineItem();
        this->addItem(B);
        B->setPen(QPen(Qt::black, 1.5, Qt::DotLine));
        B->setPos(TreeData.first[i].first);
        B->setLine(0, 0, R, 0);
    }
    for (int i = 0; i < TreeData.second.size(); i++)
    {
        this->addEllipse(TreeData.second[i].rx() - RadioPunto, TreeData.second[i].ry() - RadioPunto, RadioPunto*2, RadioPunto*2, QPen(Qt::black), QBrush(Qt::black));
    }
    for (int i = 0; i < ST->m_Foci.size(); i++)
    {
        this->addEllipse(ST->m_Foci[i].rx() - RadioFoci, ST->m_Foci[i].ry() - RadioFoci, RadioFoci*2, RadioFoci*2, QPen(Qt::blue), QBrush(Qt::blue));
    }
}

void SlimTreeScene::PintarPuntosQuery(vector<QPointF> &puntos)
{
    QPen Pen(Qt::red);
    QBrush Brush(Qt::red);
    for (auto it = puntos.begin(); it != puntos.end(); it++)
    {
        this->addEllipse(it->rx() - RadioPunto, it->ry() - RadioPunto, RadioPunto*2, RadioPunto*2, Pen, Brush);
    }
}

void SlimTreeScene::DrawFociRadios(SlimTree<QPointF> *ST, QPointF QueryP, qreal QueryR)
{
    for (int i = 0; i < FociLowBounds.size(); i++)
    {
        this->removeItem(FociLowBounds[i]);
        this->removeItem(FociUpBounds[i]);
    }
    FociLowBounds.clear();
    FociUpBounds.clear();
    for (int i = 0; i < ST->m_Foci.size(); i++)
    {
        qreal LowerBoundRadio = ST->m_fDistAux(ST->m_Foci[i], QueryP) - QueryR;
        qreal UpperBoundRadio = ST->m_fDistAux(ST->m_Foci[i], QueryP) + QueryR;
        QGraphicsEllipseItem * LB = new QGraphicsEllipseItem(ST->m_Foci[i].rx() - LowerBoundRadio, ST->m_Foci[i].ry() - LowerBoundRadio, LowerBoundRadio*2, LowerBoundRadio*2);
        QGraphicsEllipseItem * UB = new QGraphicsEllipseItem(ST->m_Foci[i].rx() - UpperBoundRadio, ST->m_Foci[i].ry() - UpperBoundRadio, UpperBoundRadio*2, UpperBoundRadio*2);
        LB->setPen(QPen(Qt::blue, 1.2, Qt::DotLine));
        UB->setPen(QPen(Qt::blue, 1.2, Qt::DotLine));
        FociLowBounds.push_back(LB);
        FociUpBounds.push_back(UB);
    }
    for (int i = 0; i < FociLowBounds.size(); i++)
    {
        this->addItem(FociLowBounds[i]);
        this->addItem(FociUpBounds[i]);
    }
}
