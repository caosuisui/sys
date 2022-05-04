//
// Created by csh on 4/6/2022.
//

#ifndef SYS_RENDERWIDGET_H
#define SYS_RENDERWIDGET_H
#include <iostream>
#include <QWidget>
#include <utility>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_3_3_Core>
#include <QMouseEvent>
#include <QOpenGLTexture>
#include <QOpenGLFramebufferObject>
#include "objmode.h"
#include "camera.hpp"
#include "NeuronInfo.h"
#include "SubwayMapWidget.h"
#include "InputWidget.h"
#include "VolumeProvider.hpp"
#define RENDER_WIDTH 1200
#define RENDER_HEIGHT 600

class RenderWidget: public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core{
    Q_OBJECT
public :
    explicit RenderWidget(QWidget* parent = nullptr);
    void SetObjFilePath(std::string path);
    void SetVolumeRenderStartPoint(double x,double y, double z);

    void SWCLoaded(NeuronInfo* neuronInfo);

    void SetOtherWidget(SubwayMapWidget* swidget ,InputWidget* iwidget);

signals:
    void SelectPointSignal(int pathid, int vertexid);
    void SelectLastPointSignal(int pathid, int vertexid);
    void SelectNextPointSignal(int pathid, int vertexid);

public slots:
    void SelectPointSlot(int pathid, int vertexid);
    void SelectLastPointSlot(int pathid, int vertexid);
    void SelectNextPointSlot(int pathid, int vertexid);
    void ChangeSelectionState(SubwayMapWidget::SelectionState in_state){
        selectionState = in_state;
    }


protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width,int height) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event)override;

private:
    void loadObj();

    void loadLines();

    void loadSWCPoint();

    void pickPoint(QPointF mousePos);

    void init();

    void GenObject(QOpenGLBuffer& vbo, QOpenGLVertexArrayObject& vao,float* data,int count);

    void AddPoint(QPoint screenPos);

    //
    /// 获取当前选中点所在体素块的起点和dimension
    /// \return 大小为6的数组，顺序为startx,starty,startz,xdimension,ydimension,zdimension.如果没有当前点则返回空vector
    std::vector<float> GetVolumeAreaData();
private:
    SubwayMapWidget* subwayMapWidget;
    InputWidget* inputWidget;

    QOpenGLShaderProgram* program;

    std::string objFilePath;
    double volumeRenderStart[3];/// start xyz
    int blocksize;

    ObjMode* objLoader;

    std::unique_ptr<control::TrackBallCamera> camera;

    NeuronInfo* neuronInfo;

    bool isWholeView;

    QMatrix4x4 mvp;
    QMatrix4x4 viewportMatrix;

    SubwayMapWidget::SelectionState selectionState = SubwayMapWidget::SelectionState::Normal;

    QPointF mousePressPos = QPointF(0,0);

    std::vector<float> box;//objbox

        QOpenGLBuffer objVBO;
        QOpenGLBuffer objEBO;
        QOpenGLVertexArrayObject objVAO;
        QVector<float> points;
        QVector<unsigned int> indexes;

        QOpenGLShaderProgram *pathProgram;
        QOpenGLBuffer pathVBO;
        QOpenGLVertexArrayObject pathVAO;

        QOpenGLShaderProgram *vertexProgram;
        QOpenGLBuffer vertexVBO;
        QOpenGLVertexArrayObject vertexVAO;
        std::vector<float> swcPoints;

        QOpenGLBuffer currentVertexVBO;
        QOpenGLVertexArrayObject currentVertexVAO;
        int currentPointId;
        std::vector<float> currentPoint;
        int currentPathId, currentInPathId;

        QOpenGLBuffer lastVertexVBO;
        QOpenGLVertexArrayObject lastVertexVAO;
        int lastPointId;
        std::vector<float> lastPoint;
        int lastPathId, lastInPathId;

        QOpenGLBuffer nextVertexVBO;
        QOpenGLVertexArrayObject nextVertexVAO;
        int nextPointId;
        std::vector<float> nextPoint;
        int nextPathId, nextInPathId;

        QOpenGLBuffer deleteVertexVBO;
        QOpenGLVertexArrayObject deleteVertexVAO;
        std::vector<float> deletePoints;

        QOpenGLBuffer deletedLinesVBO;
        QOpenGLVertexArrayObject deletedLinesVAO;
        std::vector<float> lines;
        std::vector<float> deletedLines;
};

#endif //SYS_RENDERWIDGET_H
