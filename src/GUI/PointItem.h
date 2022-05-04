//
// Created by csh on 4/19/2022.
//

#ifndef SYS_POINTITEM_H
#define SYS_POINTITEM_H

#include <QGraphicsEllipseItem>
#include <QPainter>

#include "utils.h"
#include "based.h"

class PointItem:public QObject,public QGraphicsItem{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    enum PointType{
        Unselected,
        Current,
        Last,
        Next
    };
public:
    PointItem(Vertex* v, Path* p, int id);

    void SetSelected(bool d,PointType mtype){
        isSelected = d;
        if(isSelected){
            this->setZValue(1);
            if(mtype == PointType::Current)
                color = currentColor;
            else if(mtype == PointType::Last)
                color = lastColor;
            else if(mtype == PointType::Next)
                color = nextColor;
        }else{
            this->setZValue(0);
            color = unselectedColor;
        }
        update();
    };

    void SetR(int in_r){
        r = in_r;
    }

    int GetR(){return r;}


    void GetInfo(int& pathid, int& vertexid){
        pathid = path->path_type;
        vertexid = index;
    }

    Vertex* GetVertex(){
        return vertex;
    }

    QRectF boundingRect(void) const Q_DECL_OVERRIDE;

private:
    void paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

private:
    Vertex* vertex;
    Path* path;
    int index;

    QString text;
    bool isSelected;
    int r = 5;

    QColor color = unselectedColor;

    static constexpr QColor currentColor = QColor(160,0,0);
    static constexpr QColor lastColor = QColor(80,50,0);
    static constexpr QColor nextColor = QColor(0,160,0);
    static constexpr QColor unselectedColor = QColor(0,160,230);
};

#endif //SYS_POINTITEM_H
