//
// Created by csh on 4/6/2022.
//

#ifndef SYS_SUBWAYMAPWIDGET_H
#define SYS_SUBWAYMAPWIDGET_H

#include <QWidget>
#include <based.h>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsEllipseItem>
#include <QGraphicsWidget>
#include <QVector>

#include <util/NeuronInfo.h>

#include "PointItem.h"

#define SUBWAYMAP_WIDTH 1200
#define SUBWAYMAP_HEIGHT 400

class SubwayMapWidget: public QWidget{
    Q_OBJECT
public:
    enum SelectionState{
        Normal,
        LastPoint,
        NextPoint,
        Delete,
        Add
    };

public:
    explicit SubwayMapWidget(QWidget* parent = nullptr);

    void SelectPath(int index);

    void SetPath(std::vector<Path> paths,int size, std::vector<int> mainPaths, NeuronInfo* info) ;

    void Update();

public slots:
    void SelectVertexSlot(int id);
    void SelectLastVertexSlot(int id);
    void SelectNextVertexSlot(int id);
    void ChangeSelectionState(SelectionState state);
signals:
    void SelectVertexSignal(int id);
    void SelectLastVertexSignal(int id);
    void SelectNextVertexSignal(int id);

private:
    enum ZoomDirection{
        Horizontal,
        Vertical,
        Both,
    };

protected:
    //void wheelEvent(QWheelEvent* e)Q_DECL_OVERRIDE;
//    void mousePressEvent(QMouseEvent* e)Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* e)Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event)Q_DECL_OVERRIDE;
private slots:
    void zoomIn();
    void zoomOut();

private://内部方法
    ///
    /// \param path
    /// \param x 路径起点，如果不是主要路径则是第二个点的位置
    /// \param y
    /// \param direction
    /// \param halfRange 路径及其子路径可生长的y范围
    /// \param lastPos 如果是父路径则空，如果是子路径，则代表已经被绘制的第一个点
    void GenMap(Path* path, double x, double y, double halfRange, QPointF lastPos,bool isFirstLevel = false);
    void clearScene();
    static QGraphicsPolygonItem* GetPolyGon(QPointF pos1 ,double r1, QPointF pos2, double r2, bool isHorizontal = true);

private://字段
    std::vector<Path> paths;

    int currentPath;

    double oriPathDis;

    QGraphicsScene* scene;
    QGraphicsView* view;
    std::map<int,std::vector<PointItem*> > points;

    PointItem* lastPoint;
    PointItem* preLastPoint;
    PointItem* preNextPoint;
    PointItem* lastHoveredPoint;

    double pathRatio;
    double radiusRatio;

    std::vector<int> mainPaths;

    int mainindex;

    NeuronInfo* neuronInfo;

    ZoomDirection zoomDirection;

    SelectionState selectionState;

};

#endif //SYS_SUBWAYMAPWIDGET_H
