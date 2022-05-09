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
#include <QOpenGLFunctions_4_5_Core>
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

    void SetOtherWidget(SubwayMapWidget* swidget, InputWidget* iwidget);

    void resetTransferFunc1D();

signals:
    void SelectPointSignal(int id);
    void SelectLastPointSignal(int id);
    void SelectNextPointSignal(int id);

public slots:
    void SelectPointSlot(int id);
    void SelectLastPointSlot(int id);
    void SelectNextPointSlot(int id);
    void ChangeSelectionState(SubwayMapWidget::SelectionState in_state){
        selectionState = in_state;
        connectStart = -1;
    }
    void ChangeRenderOption(bool vol, bool obj, bool line){
        ifRenderVolume = vol;
        ifRenderObj = obj;
        ifRenderLine = line;
        repaint();
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

    /// 获取当前选中点所在体素块的起点和dimension
    /// \return 大小为6的数组，顺序为startx,starty,startz,xdimension,ydimension,zdimension.如果没有当前点则返回空vector
    std::vector<float> GetVolumeAreaData();
private:
    SubwayMapWidget* subwayMapWidget;
    InputWidget* inputWidget;

    bool ifRenderVolume,ifRenderObj,ifRenderLine;

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

        QOpenGLBuffer upmostVertexVBO;
        QOpenGLVertexArrayObject upmostVertexVAO;
        std::vector<float> upmostVertex;

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

        QOpenGLBuffer newLinesVBO;
        QOpenGLVertexArrayObject newLinesVAO;
        std::vector<float> newLines;

        QOpenGLBuffer selectedLineVBO;
        QOpenGLVertexArrayObject selectedLineVAO;
        std::vector<float> selectedLine;

        QOpenGLBuffer newPointsVBO;
        QOpenGLVertexArrayObject newPointsVAO;
        std::vector<float> newPoints;

    int connectStart;
    int lastSelectedPath;
    //-------------------------------------------
    static constexpr float volume_space_x = 0.32f;
    static constexpr float volume_space_y = 0.32f;
    static constexpr float volume_space_z = 2.f;
    static constexpr float base_space = (std::min)({volume_space_x,volume_space_y,volume_space_z});
    static constexpr float space_ratio_x = volume_space_x / base_space;
    static constexpr float space_ratio_y = volume_space_y / base_space;
    static constexpr float space_ratio_z = volume_space_z / base_space;
    static constexpr int virtual_block_length = 508;
    static constexpr int block_length = 512;
    static constexpr int padding = 2;
    static constexpr int volume_block_size = 512 * 512 * 512;

    QOpenGLTexture* volume_tex;
    QOpenGLShaderProgram* ray_pos_shader;
    QOpenGLShaderProgram* raycast_shader;
    static constexpr int VolumeTexSizeX = block_length;
    static constexpr int VolumeTexSizeY = block_length;
    static constexpr int VolumeTexSizeZ = block_length;
    QOpenGLBuffer proxy_cube_vbo;
    QOpenGLVertexArrayObject proxy_cube_vao;
    QOpenGLBuffer proxy_cube_ebo;
    std::array<glm::vec3,8> proxy_cube_vertices;
    std::array<uint32_t,36> proxy_cube_indices;
    QOpenGLBuffer screen_quad_vbo;
    QOpenGLVertexArrayObject screen_quad_vao;
    std::array<float,12> screen_quad_vertices;
    std::unique_ptr<VolumeProvider> volume;

    QOpenGLFramebufferObject* fbo;
    QOpenGLTexture* ray_entry;
    QOpenGLTexture* ray_exit;
    QOpenGLTexture* tf;
    std::vector<float> tfdata;

    QOpenGLBuffer ssbo;
    float* mapping_ptr = nullptr;
};

#endif //SYS_RENDERWIDGET_H
