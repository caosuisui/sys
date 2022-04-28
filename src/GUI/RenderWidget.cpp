//
// Created by csh on 4/14/2022.
//
#include "RenderWidget.h"

RenderWidget::RenderWidget(QWidget *parent) :QOpenGLWidget(parent),
    objEBO(QOpenGLBuffer::Type::IndexBuffer),
    objVBO(QOpenGLBuffer::Type::VertexBuffer)
{
    objLoader = new ObjMode();
    blocksize = 256;
    isWholeView = false;
    mvp.setToIdentity();
    viewportMatrix.setToIdentity();
    currentPointId = -1;
    lastPointId = -1;
    nextPointId = -1;

    currentPathId = currentInPathId = -1;
    lastPathId = lastInPathId = -1;
    nextPathId = nextInPathId = -1;

    this->setFixedSize(RENDER_WIDTH,RENDER_HEIGHT);
    camera = std::make_unique<control::TrackBallCamera>(
            50,
            this->width(),this->height(),
            glm::vec3{100,100,100}
    );
}

void RenderWidget::SetVolumeRenderStartPoint(double x,double y,double z){
    volumeRenderStart[0] = x;
    volumeRenderStart[1] = y;
    volumeRenderStart[2] = z;
}

void RenderWidget::initializeGL() {
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, [&](){
    });

    initializeOpenGLFunctions();
    makeCurrent();
    glClearColor(0,0,0,0);

    program = new QOpenGLShaderProgram;
    program->addShaderFromSourceFile(QOpenGLShader::Vertex,"C:/Users/csh/Desktop/bishe/sys/src/GUI/src/obj_shader_v.glsl");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment,"C:/Users/csh/Desktop/bishe/sys/src/GUI/src/obj_shader_f.glsl");
    if(!program->link()){
        std:: cout << "program link error" << std::endl;
    }
    program->bind();
    program->release();

    pathProgram = new QOpenGLShaderProgram;
    pathProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,"C:/Users/csh/Desktop/bishe/sys/src/GUI/src/line_shader_v.glsl");
    pathProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,"C:/Users/csh/Desktop/bishe/sys/src/GUI/src/line_shader_f.glsl");
    if(!pathProgram->link()){
        std:: cout << "program link error" << std::endl;
    }

    vertexProgram = new QOpenGLShaderProgram;
    vertexProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,"C:/Users/csh/Desktop/bishe/sys/src/GUI/src/swcpoint_shader_v.glsl");
    vertexProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,"C:/Users/csh/Desktop/bishe/sys/src/GUI/src/swcpoint_shader_f.glsl");
    if(!vertexProgram->link()){
        std:: cout << "program link error" << std::endl;
    }

    doneCurrent();

}

void RenderWidget::resizeGL(int width, int height) {
    makeCurrent();
    glViewport(0, 0, width, height);
    camera->setScreenSize(width,height);
    viewportMatrix = QMatrix4x4(width / 2.f, 0,            0,     width / 2.f,
                                0,           height / 2.f, 0,     height/2.f,
                                0,           0,            1,     0,
                                0,           0,            0,     1);
    doneCurrent();
}

void RenderWidget::paintGL() {
    makeCurrent();

    QMatrix4x4 model;
    QMatrix4x4 view;
    QMatrix4x4 proj;
    model.setToIdentity();
    view.setToIdentity();
    proj.setToIdentity();

    QVector3D look_at={
            camera->getCameraLookAt().x,
            camera->getCameraLookAt().y,
            camera->getCameraLookAt().z,
    };
    QVector3D up={
            camera->getCameraUp().x,
            camera->getCameraUp().y,
            camera->getCameraUp().z,
    };
    QVector3D pos={
            camera->getCameraPos().x,
            camera->getCameraPos().y,
            camera->getCameraPos().z,
    };
    view.lookAt(pos,look_at,up);
    proj.perspective(camera->getZoom(),float(this->width())/float(this->height()),0.01f,5000.f);
    mvp = proj * view * model;

    glClearColor(1.0,1.0,1.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    //obj
    if(objVAO.isCreated()){
        program->bind();
        QMatrix4x4 objmodel;
        objmodel.setToIdentity();
        objmodel.translate(1,1,1);
        program->setUniformValue("model",objmodel);
        program->setUniformValue("view",view);
        program->setUniformValue("proj",proj);
        QOpenGLVertexArrayObject::Binder binder1(&objVAO);
        //glDrawElements(GL_TRIANGLES,indexes.size(),GL_UNSIGNED_INT,0);
        glDrawArrays(GL_TRIANGLES,0,points.size() / 3);

        program->release();
    }

    //混合渲染
    auto volumearea = GetVolumeAreaData();






    glClear(GL_DEPTH_BUFFER_BIT);

    //线
    if(neuronInfo){
       pathProgram->bind();
       pathProgram->setUniformValue("model",model);
       pathProgram->setUniformValue("view",view);
       pathProgram->setUniformValue("proj",proj);

       pathProgram->setUniformValue("color",QVector4D(1,0,0,1));
       QOpenGLVertexArrayObject::Binder binder2(&pathVAO);
       glDrawArrays(GL_LINES,0,lines.size()/3);

       pathProgram->setUniformValue("color",QVector4D(0,255 / 255.f,0,1));
       QOpenGLVertexArrayObject::Binder binder8(&deletedLinesVAO);
       glDrawArrays(GL_LINES,0,deletedLines.size()/3);

       if(!isWholeView){
           pathProgram->setUniformValue("color",QVector4D(1,0,0,1));
           glPointSize(8);
           QOpenGLVertexArrayObject::Binder binder3(&vertexVAO);
           glDrawArrays(GL_POINTS,0,swcPoints.size()/3);
       }

       if(currentPointId != -1){
           //std::cout << "Current vertex : " << currentPointId << std::endl;
           pathProgram->setUniformValue("color",QVector4D(0.5,1,1,1));
           glPointSize(8);
           QOpenGLVertexArrayObject::Binder binder4(&currentVertexVAO);
           glDrawArrays(GL_POINTS,0,currentPoint.size()/3);
       }

       if(lastPointId != -1){
           //std::cout << "Last vertex : " << currentPointId << std::endl;
           pathProgram->setUniformValue("color",QVector4D(80 / 255.f,50 / 255.f,0,1));
           glPointSize(8);
           QOpenGLVertexArrayObject::Binder binder5(&lastVertexVAO);
           glDrawArrays(GL_POINTS,0,lastPoint.size()/3);
       }

       if(nextPointId != -1){
           //std::cout << "Next vertex : " << currentPointId << std::endl;
           pathProgram->setUniformValue("color",QVector4D(0,160 / 255.f,0,1));
           glPointSize(8);
           QOpenGLVertexArrayObject::Binder binder6(&nextVertexVAO);
           glDrawArrays(GL_POINTS,0,nextPoint.size()/3);
       }

       if(!deletePoints.empty()){
           pathProgram->setUniformValue("color",QVector4D(0,255 / 255.f,0,1));
           glPointSize(8);
           QOpenGLVertexArrayObject::Binder binder7(&deleteVertexVAO);
           glDrawArrays(GL_POINTS,0,deletePoints.size()/3);
       }

       pathProgram->release();
   }

    std::cout <<"paint"<<std::endl;

    doneCurrent();
}

std::vector<float> RenderWidget::GetVolumeAreaData()
{
    std::vector<float> data;
    if(currentPointId == -1) return data;
    double x,y,z,startx,starty,startz,dimension;
    x = neuronInfo->point_vector[neuronInfo->vertex_hash[currentPointId]].x;
    y = neuronInfo->point_vector[neuronInfo->vertex_hash[currentPointId]].y;
    z = neuronInfo->point_vector[neuronInfo->vertex_hash[currentPointId]].z;

    int padding = 4;
    int a = (x - (neuronInfo->x_start - padding / 2)) / (neuronInfo->blocksize - 2);
    startx = a * (neuronInfo->blocksize - 2) + (neuronInfo->x_start - padding / 2);
    a = (y - (neuronInfo->y_start - padding / 2)) / (neuronInfo->blocksize - 2);
    starty = a * (neuronInfo->blocksize - 2) + (neuronInfo->y_start - padding / 2);
    a = (z - (neuronInfo->z_start - padding / 2)) / (neuronInfo->blocksize - 2);
    startz = a * (neuronInfo->blocksize - 2) + (neuronInfo->z_start - padding / 2);

    dimension = neuronInfo->blocksize + 2;
    data.push_back(startx);
    data.push_back(starty);
    data.push_back(startz);
    data.push_back(dimension);
    data.push_back(dimension);
    data.push_back(dimension);
    return data;
}
void RenderWidget::SetObjFilePath(std::string path) {
    objFilePath = std::move(path);
    if(neuronInfo!= nullptr)
        loadObj();
    repaint();
}

void RenderWidget::loadLines(){
    lines.clear();
    lines.reserve(neuronInfo->point_vector.size() * 2);
    deletedLines.clear();
    deletedLines.reserve(neuronInfo->deleteList.size() * 3);
    const auto& deletedPoints = neuronInfo->deleteList;
    for(auto vertex : neuronInfo->point_vector){
        if(vertex.previous_id == -1)
            continue;

        auto preVertex = neuronInfo->point_vector[neuronInfo->vertex_hash[vertex.previous_id]];

        if(std::find(deletedPoints.begin(),deletedPoints.end(),vertex.current_id) == deletedPoints.end() &&
                std::find(deletedPoints.begin(),deletedPoints.end(),preVertex.current_id) == deletedPoints.end()){
            lines.push_back(vertex.x);
            lines.push_back(vertex.y);
            lines.push_back(vertex.z);
            lines.push_back(preVertex.x);
            lines.push_back(preVertex.y);
            lines.push_back(preVertex.z);
        }
        else{
            deletedLines.push_back(vertex.x);
            deletedLines.push_back(vertex.y);
            deletedLines.push_back(vertex.z);
            deletedLines.push_back(preVertex.x);
            deletedLines.push_back(preVertex.y);
            deletedLines.push_back(preVertex.z);
        }
    }

    makeCurrent();

    if(pathVAO.isCreated() | pathVBO.isCreated()){
        pathVAO.destroy();
        pathVBO.destroy();
    }

    pathVAO.create();
    pathVBO.create();
    pathVAO.bind();
    pathVBO.bind();
    pathVBO.allocate(lines.data(),lines.size() * sizeof(float));
    pathProgram->setAttributeBuffer(0,GL_FLOAT,0,3,3 * sizeof(float));
    pathProgram->enableAttributeArray(0);
    pathVAO.release();

    if(deletedLinesVAO.isCreated() | deletedLinesVBO.isCreated()){
        deletedLinesVAO.destroy();
        deletedLinesVBO.destroy();
    }

    deletedLinesVAO.create();
    deletedLinesVBO.create();
    deletedLinesVAO.bind();
    deletedLinesVBO.bind();
    deletedLinesVBO.allocate(deletedLines.data(),deletedLines.size() * sizeof(float));
    pathProgram->setAttributeBuffer(0,GL_FLOAT,0,3,3 * sizeof(float));
    pathProgram->enableAttributeArray(0);
    deletedLinesVAO.release();

    doneCurrent();

    repaint();
}

void RenderWidget::loadSWCPoint() {
    makeCurrent();
    const auto& deleteList = neuronInfo->deleteList;

    currentPoint.clear();
    if(currentPointId != -1 && (std::find(deleteList.begin(),deleteList.end(),currentPointId) == deleteList.end())){
        neuronInfo->GetVertexPathInfo(currentPointId, currentPathId,currentInPathId);
        auto vertex = neuronInfo->point_vector[neuronInfo->vertex_hash[currentPointId]];
        currentPoint.push_back(vertex.x);
        currentPoint.push_back(vertex.y);
        currentPoint.push_back(vertex.z);
    }

    lastPoint.clear();
    if(lastPointId != -1 && (std::find(deleteList.begin(),deleteList.end(),lastPointId) == deleteList.end())){
        neuronInfo->GetVertexPathInfo(lastPointId, lastPathId,lastInPathId);
        auto vertex = neuronInfo->point_vector[neuronInfo->vertex_hash[lastPointId]];
        lastPoint.push_back(vertex.x);
        lastPoint.push_back(vertex.y);
        lastPoint.push_back(vertex.z);
        std::cout << lastPoint.data() << std::endl;
    }

    nextPoint.clear();
    if(nextPointId != -1 && (std::find(deleteList.begin(),deleteList.end(),nextPointId) == deleteList.end())){
        neuronInfo->GetVertexPathInfo(nextPointId, nextPathId,nextInPathId);
        auto vertex = neuronInfo->point_vector[neuronInfo->vertex_hash[nextPointId]];
        nextPoint.push_back(vertex.x);
        nextPoint.push_back(vertex.y);
        nextPoint.push_back(vertex.z);
    }

    deletePoints.clear();
    for(auto pointid : neuronInfo->deleteList){
        const auto& point = neuronInfo->point_vector[neuronInfo->vertex_hash[pointid]];
        deletePoints.push_back(point.x);
        deletePoints.push_back(point.y);
        deletePoints.push_back(point.z);
    }

    swcPoints.clear();
    swcPoints.reserve(neuronInfo->point_vector.size());
    for(auto vertex : neuronInfo->point_vector){
        if(vertex.previous_id == -1)
            continue;
        int cid = vertex.current_id;
        if(cid == currentPointId || cid == lastPointId || cid == nextPointId){
            continue;
        };
        if(std::find(neuronInfo->deleteList.begin(),neuronInfo->deleteList.end(),cid) != neuronInfo->deleteList.end())
            continue;
        swcPoints.push_back(vertex.x);
        swcPoints.push_back(vertex.y);
        swcPoints.push_back(vertex.z);
    }

    GenPointObject(vertexVBO,vertexVAO,swcPoints.data(),swcPoints.size() * sizeof(float));
    GenPointObject(currentVertexVBO,currentVertexVAO,currentPoint.data(),currentPoint.size() * sizeof(float));
    GenPointObject(lastVertexVBO,lastVertexVAO,lastPoint.data(),lastPoint.size() * sizeof(float));
    GenPointObject(nextVertexVBO,nextVertexVAO,nextPoint.data(),nextPoint.size() * sizeof(float));
    GenPointObject(deleteVertexVBO,deleteVertexVAO,deletePoints.data(),deletePoints.size() * sizeof(float));

    doneCurrent();

    repaint();
}

void RenderWidget::GenPointObject(QOpenGLBuffer &vbo, QOpenGLVertexArrayObject &vao,float* data,int count) {
    makeCurrent();
    if(vao.isCreated() | vbo.isCreated()){
        vao.destroy();
        vbo.destroy();
    }

    vao.create();
    vbo.create();
    vao.bind();
    vbo.bind();
    vbo.allocate(data,count);
    vertexProgram->setAttributeBuffer(0,GL_FLOAT,0,3,3 * sizeof(float));
    vertexProgram->enableAttributeArray(0);
    vao.release();
    doneCurrent();
}

void RenderWidget::SWCLoaded(NeuronInfo* neuronInfo) {
    this->neuronInfo = neuronInfo;
    currentPointId = -1;
    lastPointId = -1;
    nextPointId = -1;
    isWholeView = true;

    camera = std::make_unique<control::TrackBallCamera>(
            neuronInfo->z_dimension / 4.f,
            this->width(),this->height(),
            glm::vec3{neuronInfo->point_vector[0].x,
                      neuronInfo->point_vector[0].y,
                      neuronInfo->point_vector[0].z}
    );

    loadLines();
    loadSWCPoint();
}

void RenderWidget::loadObj(){

    objLoader->loadObjModel(QString::fromStdString(objFilePath),points,indexes,box);
    //return;
    std::cout << points.size() << "   " << indexes.size() << std::endl;

    camera = std::make_unique<control::TrackBallCamera>(
            (box[5] - box[2]) / 2.f,
            this->width(),this->height(),
            glm::vec3{(box[0] + box[3]) / 2.f,
                      (box[1] + box[4]) / 2.f,
                      (box[2] + box[5]) / 2.f}
    );

    makeCurrent();
//    objVBO = QOpenGLBuffer(QOpenGLBuffer::Type::VertexBuffer);
//    objEBO = QOpenGLBuffer(QOpenGLBuffer::Type::IndexBuffer);

    if(objVAO.isCreated() | objVBO.isCreated() | objEBO.isCreated()){
        objVAO.destroy();
        objVBO.destroy();
        objEBO.destroy();
    }

    objVAO.create();
    objVBO.create();
    //objEBO.create();

    objVAO.bind();

    objVBO.bind();
    objVBO.allocate(points.data(),points.size()*sizeof(float));

    program->setAttributeBuffer(0,GL_FLOAT,0,3,6 * sizeof(GLfloat));
    program->enableAttributeArray(0);
    program->setAttributeBuffer(1,GL_FLOAT,3 * sizeof(float),3,6 * sizeof(GLfloat));
    program->enableAttributeArray(1);

//    objEBO.bind();
//    objEBO.allocate(indexes.data(),indexes.size()*sizeof(uint32_t));

//    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
//    f->glEnableVertexAttribArray(0);
//    f->glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(GLfloat),nullptr);
//    f->glEnableVertexAttribArray(1);
//    f->glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(GLfloat),(const GLvoid*)(3 * sizeof(float)));


    objVAO.release();
    //objVBO.release();

    doneCurrent();
}

void RenderWidget::mousePressEvent(QMouseEvent *event)
{
    mousePressPos = event->pos();
    if(!camera) return;
    camera->processMouseButton(control::CameraDefinedMouseButton::Left,
                               true,
                               event->position().x(),
                               event->position().y());
    event->accept();
    repaint();
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event)
{
    camera->processMouseMove(event->pos().x(),event->pos().y());

    event->accept();
    repaint();
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if((mousePressPos - event->pos()).manhattanLength() < 20){
        pickPoint(event->position());
    }
    else{
        camera->processMouseButton(control::CameraDefinedMouseButton::Left,
                                   false,
                                   event->position().x(),
                                   event->position().y());
    }

    event->accept();
    std::cout << event->pos().x() << "   " << event->pos().y() << std::endl;

    repaint();
}

void RenderWidget::pickPoint(QPointF mousePos){
    mousePos.setY(RENDER_HEIGHT - mousePos.y());
    float limit = 25;
    float mindis = limit + 1;
    int id = -1;
    if(!neuronInfo) return;
    for(auto point : neuronInfo->point_vector){
        QVector4D screen = mvp.map(QVector4D(point.x,point.y,point.z,1.0));

        if(screen.w() !=0.0f){
            screen.setX(screen.x() / screen.w());
            screen.setY(screen.y() / screen.w());
            screen.setZ(screen.z() / screen.w());
            screen.setW(1);
        }
        if(screen.x() > -1 && screen.x() < 1 && screen.y() > -1 &&screen.y() < 1){
            screen = viewportMatrix.map(screen);

            float dis = (QPointF(screen.x(),screen.y()) - mousePos ).manhattanLength();
            if( dis > limit | dis > mindis) continue;
            mindis = dis;
            id = point.current_id;
        }
    }

    if(id == -1) return;

    int id1,id2;
    neuronInfo->GetVertexPathInfo(id,id1,id2);
    if(selectionState == SubwayMapWidget::SelectionState::Normal) {
//        if(currentPathId != id1){
//            lastPointId = -1;
//            nextPointId = -1;
//
//            lastPathId = lastInPathId = -1;
//            nextPathId = nextInPathId = -1;
//        }
        currentPointId = id;
        currentPathId = id1;
        currentInPathId = id2;

        std::cout << lastPointId << std::endl;

        loadSWCPoint();

        emit SelectPointSignal(currentPathId, currentInPathId);
        return;
    }
    else if(selectionState == SubwayMapWidget::SelectionState::LastPoint){
        bool valid = false;

        if(currentPointId != -1){
            if(id1 == currentPathId && id2 > currentInPathId)
                valid =true;
        }
        else if(nextPointId != -1){
            if(id1 == nextPathId && id2 > nextInPathId)
                valid =true;
        }
        else
            valid = true;

        if(valid){
            lastPointId = id;
            lastPathId = id1;
            lastInPathId = id2;
            loadSWCPoint();
            emit SelectLastPointSignal(id1, id2);
        }
    }
    else if(selectionState == SubwayMapWidget::SelectionState::NextPoint){
        bool valid = false;
        if(currentPointId != -1){
            if(id1 == currentPathId && id2 < currentInPathId)
                valid =true;
        }
        else if(lastPointId != -1){
            if(id1 == nextPathId && id2 < lastInPathId)
                valid =true;
        }
        else
            valid = true;

        if(valid){
            lastPointId = id;
            nextPathId = id1;
            nextInPathId = id2;
            loadSWCPoint();
            emit SelectNextPointSignal(id1, id2);
        }
    }
    else if(selectionState == SubwayMapWidget::SelectionState::Delete){
        neuronInfo->DeleteVertex(id);
        loadLines();
        loadSWCPoint();
    }
}

void RenderWidget::wheelEvent(QWheelEvent *event)
{
    camera->processMouseScroll(event->angleDelta().y());
    event->accept();
    repaint();
}

void RenderWidget::init(){
    currentPointId = -1;
    lastPointId = -1;
    nextPointId = -1;

    currentPathId = currentInPathId = -1;
    lastPathId = lastInPathId = -1;
    nextPathId = nextInPathId = -1;
    if(inputWidget){
        inputWidget->ChangeNextPoint(-1,-1);
        inputWidget->ChangeCurrentPoint(-1,-1);
        inputWidget->ChangeLastPoint(-1,-1);
    }
}

void RenderWidget::SelectPointSlot(int pathid, int vertexid) {
    if(pathid != currentPathId)
        init();

    isWholeView = false;
    currentPointId = neuronInfo->paths[pathid].path[vertexid].current_id;

    const auto& vertex = neuronInfo->point_vector[neuronInfo->vertex_hash[currentPointId]];
    camera = std::make_unique<control::TrackBallCamera>(
            10,
            this->width(),this->height(),
            glm::vec3{vertex.x,
                      vertex.y,
                      vertex.z}
    );
    loadSWCPoint();
    repaint();
}

void RenderWidget::SelectLastPointSlot(int pathid, int vertexid) {
    isWholeView = false;
    lastPointId = neuronInfo->paths[pathid].path[vertexid].current_id;
    loadSWCPoint();
    repaint();
}

void RenderWidget::SelectNextPointSlot(int pathid, int vertexid) {
    isWholeView = false;
    nextPointId = neuronInfo->paths[pathid].path[vertexid].current_id;
    loadSWCPoint();
    repaint();
}

void RenderWidget::SetOtherWidget(SubwayMapWidget *swidget ,InputWidget* iwidget) {
    subwayMapWidget = swidget;
    connect(subwayMapWidget,&SubwayMapWidget::SelectVertexSignal,this,&RenderWidget::SelectPointSlot);
    connect(subwayMapWidget,&SubwayMapWidget::SelectLastVertexSignal,this,&RenderWidget::SelectLastPointSlot);
    connect(subwayMapWidget,&SubwayMapWidget::SelectNextVertexSignal,this,&RenderWidget::SelectNextPointSlot);

    connect(this,&RenderWidget::SelectPointSignal,subwayMapWidget,&SubwayMapWidget::SelectVertexSlot);
    connect(this,&RenderWidget::SelectLastPointSignal,subwayMapWidget,&SubwayMapWidget::SelectLastVertexSlot);
    connect(this,&RenderWidget::SelectNextPointSignal,subwayMapWidget,&SubwayMapWidget::SelectNextVertexSlot);

    inputWidget = iwidget;
    connect(inputWidget,&InputWidget::ChangeSelectionStateSignal,this,&RenderWidget::ChangeSelectionState);
}

