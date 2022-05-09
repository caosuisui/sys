//
// Created by csh on 4/19/2022.
//


#include "PointItem.h"

PointItem::PointItem(Vertex *v, Path *p, int id) {
    vertex = v;
    path = p;
    index = id;

    QString tooltip = "vertex " + QString::number(vertex->current_id) +
            "<br>radius " + QString::number(vertex->radius) +
            "<br>in path " + QString::number(path->path_type) +
            "<br>index in path " + QString::number(id);
    setToolTip(tooltip);
    isSelected = false;
}

void PointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(QPen(color,3));
    painter->setBrush(color);

    painter->drawLine(0,- r,0,r);
    //painter->drawEllipse(-r,-r,2*r,2*r);
}

QRectF PointItem::boundingRect(void) const {
    double f = r + 2;
    return QRectF(-f,-f,2*f,2*f);
}

void PointItem::SetSelected(int d, PointType mtype) {
    if(isSelected == 1 && d == 2) return;
    isSelected = d;
    if(isSelected == 1){
        this->setZValue(2);
        if(mtype == PointType::Current)
            color = currentColor;
        else if(mtype == PointType::Last)
            color = lastColor;
        else if(mtype == PointType::Next)
            color = nextColor;
    }
    else if(isSelected == 0){
        this->setZValue(0);
        color = unselectedColor;
    }
    else if(isSelected == 2){
        this->setZValue(1);
        color = hoveredColor;
    }
    update();
};

