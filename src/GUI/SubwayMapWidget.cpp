//
// Created by csh on 4/14/2022.
//

#include "SubwayMapWidget.h"

#include <utility>

SubwayMapWidget::SubwayMapWidget(QWidget *parent) :QWidget(parent){
    this->setFixedSize(SUBWAYMAP_WIDTH,SUBWAYMAP_HEIGHT);
    lastPoint = nullptr;
    preLastPoint = nullptr;
    preNextPoint = nullptr;
    oriPathDis = 200;
    pathRatio = 8;
    radiusRatio = 5;
    zoomDirection = ZoomDirection::Both;
    selectionState = SelectionState::Normal;

    mainindex = 0;

    currentPath = 0;
    scene = new QGraphicsScene();
    scene->setBackgroundBrush(Qt::white);
    view = new QGraphicsView(scene,this);
    //view->setOptimizationFlag(QGraphicsView::DontSavePainterState,true);
    //view->setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing,true);
    view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);

    view->resize(SUBWAYMAP_WIDTH ,SUBWAYMAP_HEIGHT);
    view->show();
}

//void SubwayMapWidget::ChangePath(int index) {
//    SelectPath(index);
//}

void SubwayMapWidget::zoomIn() {
    if(zoomDirection == ZoomDirection::Vertical){
        oriPathDis *= 1.5;
    }
    else if(zoomDirection == ZoomDirection::Both){
        view->scale(1.2,1.2);
    }
    else if(zoomDirection == ZoomDirection::Horizontal){
        pathRatio *= 1.2;
    }

    SelectPath(currentPath);

}

void SubwayMapWidget::zoomOut() {
    if(zoomDirection == ZoomDirection::Vertical){
        oriPathDis = (oriPathDis / 1.5 == 0) ? oriPathDis : oriPathDis / 1.5;
    }
    else if(zoomDirection == ZoomDirection::Both){
        view->scale(1/1.2,1/1.2);
    }
    else if(zoomDirection == ZoomDirection::Horizontal){
        pathRatio = (pathRatio / 1.2 < 1) ? pathRatio : pathRatio / 1.2;
    }

    SelectPath(currentPath);
}

//void SubwayMapWidget::wheelEvent(QWheelEvent *e) {
//    e->angleDelta().y() > 0 ? zoomIn() : zoomOut();
//}

void SubwayMapWidget::keyPressEvent(QKeyEvent *e) {
    e->accept();

    if(e->key() == Qt::Key_Z)
        zoomIn();
    else if(e->key() == Qt::Key_X)
        zoomOut();
//    else if(e->key() == Qt::Key_1){
//        mainindex++;
//        if(!(mainindex >=0 && mainindex < mainPaths.size()))
//            mainindex = 0;
//        currentPath = mainPaths[mainindex];
//        SelectPath(currentPath);
//    }
    else if(e->key() == Qt::Key_F1){
        zoomDirection = ZoomDirection::Both;
    }
    else if(e->key() == Qt::Key_F2){
        zoomDirection = ZoomDirection::Horizontal;
    }
    else if(e->key() == Qt::Key_F3){
        zoomDirection = ZoomDirection::Vertical;
    }

}

void SubwayMapWidget::mousePressEvent(QMouseEvent *e) {
    auto item = view->itemAt(e->pos());
    auto p = dynamic_cast<PointItem*>(item);
    if(p!= nullptr){
        if(selectionState == SelectionState::Normal){
            if(lastPoint != nullptr){
                lastPoint->SetSelected(false,PointItem::PointType::Unselected);
            }

            p->SetSelected(true, PointItem::PointType::Current);
            lastPoint = p;

            int pathid;
            int vertexid;
            p->GetInfo(pathid,vertexid);

            emit SelectVertexSignal(pathid,vertexid);
        }
        else if(selectionState == SelectionState::LastPoint){
            int pathid1;
            int vertexid1;
            p->GetInfo(pathid1,vertexid1);
            bool valid = false;

            if(lastPoint){
                int pathid2;
                int vertexid2;
                lastPoint->GetInfo(pathid2,vertexid2);

                if(pathid1 == pathid2 && vertexid1 > vertexid2){
                    valid = true;
                }
            }
            else if(preNextPoint){
                int pathid2;
                int vertexid2;
                preNextPoint->GetInfo(pathid2,vertexid2);

                if(pathid1 == pathid2 && vertexid1 > vertexid2){
                    valid = true;
                }
            }
            else
                valid = true;

            if(valid){
                if(preLastPoint != nullptr){
                    preLastPoint->SetSelected(false,PointItem::PointType::Unselected);
                }

                p->SetSelected(true,PointItem::PointType::Last);
                preLastPoint = p;
                emit SelectLastVertexSignal(pathid1,vertexid1);
            }
        }
        else if(selectionState == SelectionState::NextPoint){
            int pathid1;
            int vertexid1;
            p->GetInfo(pathid1,vertexid1);
            int valid = false;

            if(lastPoint){
                int pathid2;
                int vertexid2;
                lastPoint->GetInfo(pathid2,vertexid2);

                if(pathid1 == pathid2 && vertexid1 < vertexid2){
                    valid = true;
                }
            }
            else if(preLastPoint){
                int pathid2;
                int vertexid2;
                preLastPoint->GetInfo(pathid2,vertexid2);

                if(pathid1 == pathid2 && vertexid1 < vertexid2){
                    valid = true;
                }
            }

            if(valid){
                if(preNextPoint != nullptr){
                    preNextPoint->SetSelected(false,PointItem::PointType::Unselected);
                }

                p->SetSelected(true,PointItem::PointType::Next);
                preNextPoint = p;
                emit SelectNextVertexSignal(pathid1,vertexid1);
            }
        }
    }
    e->accept();
}

void SubwayMapWidget::GenMap(Path* path, double x, double y, double halfRange, QPointF lastPos,bool isFirstLevel) {
    std::vector<PointItem*> thisPath;
    thisPath.reserve(path->path.size());

    if(isFirstLevel){//主路径，生成第一个点，不是主路径则第一个点已经绘制
        auto& vertex = path->path.back();

        double r;
        if(vertex.radius == 0)
            r = 0.05 * radiusRatio;
        else
            r = vertex.radius * radiusRatio;

        auto point = new PointItem(&vertex,path,path->path.size()-1);
        point->SetR(r);
        point->moveBy(x,y);


        scene->addItem(point);
        thisPath.push_back(point);

        vertex.sy = y;
        vertex.sx = x;
        neuronInfo->point_vector[neuronInfo->vertex_hash[vertex.current_id]].sy = y;
        neuronInfo->point_vector[neuronInfo->vertex_hash[vertex.current_id]].sx = x;
        x += path->path[path->path.size()-2].distance * pathRatio;
    }

    //获取该路径一个点的信息
    const auto& lastver = neuronInfo->point_vector[neuronInfo->vertex_hash[path->path.back().current_id]];
    auto firstx = lastver.sx;
    auto firsty = lastver.sy;
    auto lastp = QPointF(firstx,firsty);
    double lastverr;
    if(lastver.radius == 0)
        lastverr = 0.1 * radiusRatio;
    else
        lastverr = lastver.radius * radiusRatio;

    //绘制当前路径
    for(int i = path->path.size()-2 ; i >= 0 ; i--) {
        auto& vertex = path->path[i];
        if(i != path->path.size()-2)
            x += vertex.distance * pathRatio;

        double r = vertex.radius * radiusRatio;

        auto point = new PointItem(&vertex,path,i);
        point->SetR(r);
        point->moveBy(x,y);

        if(i == path->path.size()-2 && !isFirstLevel){
            auto polygon = GetPolyGon(lastp,lastverr,point->pos(),r,false);
            scene->addItem(polygon);
        }
        else{
            auto polygon = GetPolyGon(thisPath.back()->pos(),thisPath.back()->GetR(),point->pos(),r);
            scene->addItem(polygon);
        }

        scene->addItem(point);
        thisPath.push_back(point);

        vertex.sy = y;
        vertex.sx = x;
        neuronInfo->point_vector[neuronInfo->vertex_hash[vertex.current_id]].sy = y;
        neuronInfo->point_vector[neuronInfo->vertex_hash[vertex.current_id]].sx = x;
    }

    points[path->path_type] = thisPath;

    //绘制分支
    int subPathNum = path->sub_paths_index.size();
    int a = subPathNum / 2;
    double nextHalfRange = halfRange / (a * 2);

    for(int i = 0; i < path->sub_paths_index.size();i++){//叶到根
        double nextx,nexty;

        int branchIndex = paths[path->sub_paths_index[i]].branch_point_index;
        nextx = path->path[branchIndex].sx;

        if(i % 2 == 0){
            nexty = y + nextHalfRange * ((i / 2 ) * 2 + 1);
        }
        else{
            nexty = y - nextHalfRange * ((i / 2 ) * 2 + 1);
        }

        GenMap(&paths[path->sub_paths_index[i]],nextx,nexty, nextHalfRange, QPointF(path->path[branchIndex].sx,path->path[branchIndex].sy));
    }

    view->show();
}

QGraphicsPolygonItem* SubwayMapWidget::GetPolyGon(QPointF pos1 ,double r1, QPointF pos2, double r2, bool isHorizontal)
{
    QPointF *lefttop, *leftbottom, *righttop, *rightbottom;
    if(isHorizontal){
        lefttop = new QPointF(pos1.x(),pos1.y() - r1);
        leftbottom = new QPointF(pos1.x(),pos1.y() + r1);
        righttop = new QPointF(pos2.x(),pos2.y() - r2);
        rightbottom = new QPointF(pos2.x(),pos2.y() + r2);
    }
    else{
        lefttop = new QPointF(pos2.x() - r2,pos2.y());
        leftbottom = new QPointF(pos1.x() - r1,pos1.y());
        rightbottom = new QPointF(pos1.x() + r1,pos1.y());
        righttop = new QPointF(pos2.x() + r2,pos2.y());
    }

    QVector<QPointF> vec;
    vec.push_back(*lefttop);
    vec.push_back(*leftbottom);
    vec.push_back(*rightbottom);
    vec.push_back(*righttop);
    vec.push_back(*lefttop);
    auto polygon = new QPolygonF(vec);
    auto polygonItem = new QGraphicsPolygonItem(*polygon);
    polygonItem->setZValue(-1);
    QPen pen1;
    pen1.setColor(QColor(0, 160, 230));
    pen1.setWidth(3);
    polygonItem->setPen(pen1);

    QBrush brush;
    brush.setColor(QColor(0, 160, 230));
    brush.setStyle(Qt::SolidPattern);
    polygonItem->setBrush(brush);
    brush.setStyle(Qt::NoBrush);
    return polygonItem;
}

void SubwayMapWidget::SelectPath(int index){
    if(index >= 0 && index < paths.size())
        currentPath = index;
    else
        currentPath = 0;

    std::cout << "select path : " << currentPath << std::endl;
    clearScene();

    double ydimension = oriPathDis * paths[currentPath].sub_paths_index.size() * 4;
    double starty = ydimension / 4;
    //view->translate(0,starty);


    GenMap(&paths[currentPath],0,starty,oriPathDis, QPointF(0,0),true);

}

void SubwayMapWidget::SetPath(std::vector<Path> paths,int size, std::vector<int> mainPaths, NeuronInfo* info) {
    this->paths = std::move(paths);
    points.clear();
    this->mainPaths = std::move(mainPaths);
    neuronInfo = info;
    SelectPath(0);
}

void SubwayMapWidget::clearScene() {
    scene->clear();

    points.clear();
    lastPoint = nullptr;
    preLastPoint = nullptr;
    preNextPoint = nullptr;

}

void SubwayMapWidget::ChangeSelectionState(SelectionState state) {
    if(state == selectionState)
        return;

    selectionState = state;
}

void SubwayMapWidget::SelectVertexSlot(int pathid, int vertexid) {
    if(pathid != currentPath)
        SelectPath(pathid);
    auto items = view->items();
    for(auto item : items){
        auto point = dynamic_cast<PointItem*>(item);
        if(point != nullptr){
            int id1,id2;
            point->GetInfo(id1,id2);
            if(id1 == pathid && id2 == vertexid){
                point->SetSelected(true,PointItem::PointType::Current);
                if(lastPoint != nullptr)
                    lastPoint->SetSelected(false,PointItem::PointType::Unselected);
                lastPoint = point;
                break;
            }
        }
    }
    emit SelectVertexSignal(pathid,vertexid);
}

void SubwayMapWidget::SelectLastVertexSlot(int pathid, int vertexid) {
    if(pathid != currentPath)
        SelectPath(pathid);
    auto items = view->items();
    for(auto item : items){
        auto point = dynamic_cast<PointItem*>(item);
        if(point != nullptr){
            int id1,id2;
            point->GetInfo(id1,id2);
            if(id1 == pathid && id2 == vertexid){
                point->SetSelected(true,PointItem::PointType::Last);
                if(preLastPoint != nullptr)
                    preLastPoint->SetSelected(false,PointItem::PointType::Unselected);
                preLastPoint = point;
                break;
            }
        }
    }
    emit SelectLastVertexSignal(pathid,vertexid);
}

void SubwayMapWidget::SelectNextVertexSlot(int pathid, int vertexid) {
    if(pathid != currentPath)
        SelectPath(pathid);
    auto items = view->items();
    for(auto item : items){
        auto point = dynamic_cast<PointItem*>(item);
        if(point != nullptr){
            int id1,id2;
            point->GetInfo(id1,id2);
            if(id1 == pathid && id2 == vertexid){
                point->SetSelected(true,PointItem::PointType::Next);
                if(preNextPoint != nullptr)
                    preNextPoint->SetSelected(false,PointItem::PointType::Unselected);
                preNextPoint = point;
                break;
            }
        }
    }
    emit SelectNextVertexSignal(pathid,vertexid);
}