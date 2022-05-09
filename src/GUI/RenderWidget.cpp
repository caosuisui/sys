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
    lastSelectedPath = -1;

    currentPathId = currentInPathId = -1;
    lastPathId = lastInPathId = -1;
    nextPathId = nextInPathId = -1;
    connectStart = -1;

    ifRenderVolume = ifRenderObj = ifRenderLine = true;

    this->setFixedSize(RENDER_WIDTH,RENDER_HEIGHT);
    camera = std::make_unique<control::TrackBallCamera>(
            50,
            this->width(),this->height(),
            glm::vec3{100,100,100}
    );
    volume = std::make_unique<VolumeProvider>("G:/csh/volume/ffmpegyuv420_9p2_max_lod0_28452_21866_4834.h264");

    proxy_cube_indices = {
            0, 1, 2, 0, 2, 3, 0, 4, 1, 4, 5, 1, 1, 5, 6, 6, 2, 1,
            6, 7, 2, 7, 3, 2, 7, 4, 3, 3, 4, 0, 4, 7, 6, 4, 6, 5
    };
    screen_quad_vertices = {
        1.f,-1.f, 1.f,1.f, -1.f,-1.f,
        -1.f,-1.f, 1.f,1.f, -1.f,1.f
    };
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
    program->addShaderFromSourceFile(QOpenGLShader::Vertex,"../../../src/GUI/src/obj_shader_v.glsl");
    program->addShaderFromSourceFile(QOpenGLShader::Fragment,"../../../src/GUI/src/obj_shader_f.glsl");
    if(!program->link()){
        std:: cout << "program link error" << std::endl;
    }
    program->bind();
    program->release();

    pathProgram = new QOpenGLShaderProgram;
    pathProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,"../../../src/GUI/src/line_shader_v.glsl");
    pathProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,"../../../src/GUI/src/line_shader_f.glsl");
    if(!pathProgram->link()){
        std:: cout << "program link error" << std::endl;
    }

    vertexProgram = new QOpenGLShaderProgram;
    vertexProgram->addShaderFromSourceFile(QOpenGLShader::Vertex,"../../../src/GUI/src/swcpoint_shader_v.glsl");
    vertexProgram->addShaderFromSourceFile(QOpenGLShader::Fragment,"../../../src/GUI/src/swcpoint_shader_f.glsl");
    if(!vertexProgram->link()){
        std:: cout << "program link error" << std::endl;
    }
//----------------------------------------------------------------------------
    volume_tex = new QOpenGLTexture(QOpenGLTexture::Target3D);
    volume_tex->create();
    assert(volume_tex->isCreated());
    volume_tex->bind();
    glTexImage3D(GL_TEXTURE_3D,0,GL_RED,VolumeTexSizeX,VolumeTexSizeY,VolumeTexSizeZ,0,GL_RED,GL_UNSIGNED_BYTE,nullptr);
    volume_tex->setMinMagFilters(QOpenGLTexture::Linear,QOpenGLTexture::Linear);
    volume_tex->setBorderColor(0,0,0,0);
    volume_tex->setWrapMode(QOpenGLTexture::ClampToBorder);

    proxy_cube_vbo = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    proxy_cube_ebo = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    proxy_cube_vao.create();
    proxy_cube_vbo.create();
    proxy_cube_ebo.create();
    proxy_cube_vao.bind();
    proxy_cube_vbo.bind();
    proxy_cube_vbo.allocate(proxy_cube_vertices.data(),sizeof(proxy_cube_vertices));
    proxy_cube_ebo.bind();
    proxy_cube_ebo.allocate(proxy_cube_indices.data(),sizeof(proxy_cube_indices));
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(GLfloat),nullptr);
    proxy_cube_vao.release();

    screen_quad_vao.create();
    screen_quad_vbo.create();
    screen_quad_vao.bind();
    screen_quad_vbo.bind();
    screen_quad_vbo.allocate(screen_quad_vertices.data(),sizeof(screen_quad_vertices));
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(GLfloat),nullptr);
    screen_quad_vao.release();

    ray_pos_shader = new QOpenGLShaderProgram;
    ray_pos_shader->addShaderFromSourceFile(QOpenGLShader::Vertex,"../../../src/GUI/src/volume_raycast_pos.vert");
    ray_pos_shader->addShaderFromSourceFile(QOpenGLShader::Fragment,"../../../src/GUI/src/volume_raycast_pos.frag");
    if(!ray_pos_shader->link()){
        std::cerr<<"ray pos shader link error"<<std::endl;
    }

    raycast_shader = new QOpenGLShaderProgram;
    raycast_shader->addShaderFromSourceFile(QOpenGLShader::Vertex,"../../../src/GUI/src/volume_raycast_render.vert");
    raycast_shader->addShaderFromSourceFile(QOpenGLShader::Fragment,"../../../src/GUI/src/volume_raycast_render.frag");
    if(!raycast_shader->link()){
        std::cerr<<"raycast shader link error"<<std::endl;
    }
    int w = frameSize().width(), h = frameSize().height();
    fbo = new QOpenGLFramebufferObject(w,h,QOpenGLFramebufferObject::Attachment::Depth);
    fbo->bind();
    ray_entry = new QOpenGLTexture(QOpenGLTexture::TargetRectangle);
    ray_entry->create();
    std::cerr<<"ray_entry: "<<ray_entry->textureId()<<std::endl;
    glBindTexture(GL_TEXTURE_RECTANGLE,ray_entry->textureId());
    glTexImage2D(GL_TEXTURE_RECTANGLE,0,GL_RGBA32F,w,h,0,GL_RGBA,GL_FLOAT,nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_RECTANGLE,ray_entry->textureId(),0);

    ray_exit = new QOpenGLTexture(QOpenGLTexture::TargetRectangle);;
    ray_exit->create();
    std::cerr<<"ray_exit: "<<ray_exit->textureId()<<std::endl;
    glBindTexture(GL_TEXTURE_RECTANGLE,ray_exit->textureId());
    glTexImage2D(GL_TEXTURE_RECTANGLE,0,GL_RGBA32F,w,h,0,GL_RGBA,GL_FLOAT,nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_RECTANGLE,ray_exit->textureId(),0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr<<"framebuffer error"<<std::endl;
    }
    fbo->release();

    ssbo.create();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,ssbo.bufferId());
    glBufferStorage(GL_SHADER_STORAGE_BUFFER,sizeof(float)*8,nullptr, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    mapping_ptr = static_cast<float*>(glMapBuffer(GL_SHADER_STORAGE_BUFFER,GL_WRITE_ONLY));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,0,ssbo.bufferId());
    std::cerr<<glGetError()<<" "<<"mapping_ptr: "<<mapping_ptr<<std::endl;

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
    proj.perspective(camera->getZoom(),float(this->width())/float(this->height()),1.f,25000.f);
    mvp = proj * view * model;

    glClearColor(0.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    if(ifRenderVolume) {
        //混合渲染
        auto volumearea = GetVolumeAreaData();
        //compute intersect blocks
        static std::vector<uint8_t> volume_data(volume_block_size);
        static glm::ivec3 last_start_block_index = {-1, -1, -1};
        if (volumearea.size() == 6) {
            float voxel_start_pos_x = volumearea[0] / volume_space_x;
            float voxel_x_len = virtual_block_length * volume_space_x;

            float voxel_start_pos_y = volumearea[1] / volume_space_y;
            float voxel_y_len = virtual_block_length * volume_space_y;

            float voxel_start_pos_z = volumearea[2] / volume_space_z;
            float voxel_z_len = virtual_block_length * volume_space_z;

            glm::ivec3 start_block_index = glm::ivec3(voxel_start_pos_x / virtual_block_length,
                                                      voxel_start_pos_y / virtual_block_length,
                                                      voxel_start_pos_z / virtual_block_length);
            voxel_start_pos_x = start_block_index.x * virtual_block_length * volume_space_x;
            voxel_start_pos_y = start_block_index.y * virtual_block_length * volume_space_y;
            voxel_start_pos_z = start_block_index.z * virtual_block_length * volume_space_z;

            if (start_block_index != last_start_block_index) {
                volume->readBlock({start_block_index.x, start_block_index.y, start_block_index.z}, volume_data.data(),
                                  volume_data.size());
                volume_tex->setData(0, 0, 0, VolumeTexSizeX, VolumeTexSizeY, VolumeTexSizeZ,
                                    QOpenGLTexture::PixelFormat::Red, QOpenGLTexture::PixelType::UInt8,
                                    volume_data.data());
                last_start_block_index = start_block_index;
            }
            proxy_cube_vertices[0] = {voxel_start_pos_x, voxel_start_pos_y, voxel_start_pos_z};
            proxy_cube_vertices[1] = {voxel_start_pos_x + voxel_x_len, voxel_start_pos_y, voxel_start_pos_z};
            proxy_cube_vertices[2] = {voxel_start_pos_x + voxel_x_len, voxel_start_pos_y + voxel_y_len,
                                      voxel_start_pos_z};
            proxy_cube_vertices[3] = {voxel_start_pos_x, voxel_start_pos_y + voxel_y_len, voxel_start_pos_z};
            proxy_cube_vertices[4] = {voxel_start_pos_x, voxel_start_pos_y, voxel_start_pos_z + voxel_z_len};
            proxy_cube_vertices[5] = {voxel_start_pos_x + voxel_x_len, voxel_start_pos_y,
                                      voxel_start_pos_z + voxel_z_len};
            proxy_cube_vertices[6] = {voxel_start_pos_x + voxel_x_len, voxel_start_pos_y + voxel_y_len,
                                      voxel_start_pos_z + voxel_z_len};
            proxy_cube_vertices[7] = {voxel_start_pos_x, voxel_start_pos_y + voxel_y_len,
                                      voxel_start_pos_z + voxel_z_len};
            proxy_cube_vbo.bind();
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(proxy_cube_vertices), proxy_cube_vertices.data());
            proxy_cube_vbo.release();

            ray_pos_shader->bind();
            ray_pos_shader->setUniformValue("MVPMatrix", mvp);

            QOpenGLVertexArrayObject::Binder binder1(&proxy_cube_vao);
            bool b = fbo->bind();
            assert(b);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);

            glEnable(GL_CULL_FACE);
            glFrontFace(GL_CCW);
            glDrawBuffer(GL_COLOR_ATTACHMENT1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            glDisable(GL_CULL_FACE);

            fbo->release();
            glGetError();
//        std::cerr<<glGetError()<<std::endl;

            ray_pos_shader->release();

            raycast_shader->bind();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_RECTANGLE, ray_entry->textureId());
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_RECTANGLE, ray_exit->textureId());
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_3D, volume_tex->textureId());

            raycast_shader->setUniformValue("RayStartPos", 0);
            raycast_shader->setUniformValue("RayEndPos", 1);
            raycast_shader->setUniformValue("VolumeData", 2);
            raycast_shader->setUniformValue("voxel", 1.f);
            raycast_shader->setUniformValue("step", 0.6f);
            raycast_shader->setUniformValue("volume_start_pos", voxel_start_pos_x, voxel_start_pos_y,
                                            voxel_start_pos_z);
            raycast_shader->setUniformValue("volume_extend", block_length * volume_space_x,
                                            block_length * volume_space_y, block_length * volume_space_z);
            raycast_shader->setUniformValue("padding", padding * volume_space_x, padding * volume_space_y,
                                            padding * volume_space_z);
            raycast_shader->setUniformValue("bg_color", QVector4D(0.0, 0.0, 0.0, 1.0));
            raycast_shader->setUniformValue("MVPMatrix", mvp);
            bool inside = false;
            if (pos.x() >= voxel_start_pos_x && pos.x() < voxel_start_pos_x + voxel_x_len
                && pos.y() >= voxel_start_pos_y && pos.y() < voxel_start_pos_y + voxel_y_len
                && pos.z() >= voxel_start_pos_z && pos.z() < voxel_start_pos_z + voxel_z_len) {
                inside = true;
            }
//        if(inside){
//            std::cerr<<"inside"<<std::endl;
//        }
            raycast_shader->setUniformValue("camera_pos", QVector4D(pos, inside ? 1.0 : 0.0));
//        std::cerr<<"mouse press pos: "<<mousePressPos.x()<<" "<<mousePressPos.y()<<std::endl;
            raycast_shader->setUniformValue("click_coord", mousePressPos.x(), height() - mousePressPos.y());

            //memset mapping_ptr before draw
            memset(mapping_ptr, 0, sizeof(float) * 8);

            QOpenGLVertexArrayObject::Binder binder3(&screen_quad_vao);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glFinish();

            raycast_shader->release();

//        std::cerr<<"click point info: ";
//        for(int i = 0;i<8;i++){
//            std::cerr<<mapping_ptr[i]<<" ";
//        }
//        std::cerr<<std::endl;
        }

        //debug for volume proxy cube wireframe
        if (true) {
            pathProgram->bind();
            pathProgram->setUniformValue("model", model);
            pathProgram->setUniformValue("view", view);
            pathProgram->setUniformValue("proj", proj);

            pathProgram->setUniformValue("color", QVector4D(0.5, 0.5, 0.5, 1));
            QOpenGLVertexArrayObject::Binder binder1(&proxy_cube_vao);
            ::glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
            ::glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        glGetError();

        //绘制线框

    }

//    std::cerr<<glGetError()<<std::endl;


    //obj
    if(objVAO.isCreated() && ifRenderObj){
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

    //线
    if(neuronInfo && ifRenderLine){
        pathProgram->bind();
        pathProgram->setUniformValue("model",model);
        pathProgram->setUniformValue("view",view);
        pathProgram->setUniformValue("proj",proj);

        pathProgram->setUniformValue("color",QVector4D(1,0,0,1));
        QOpenGLVertexArrayObject::Binder binder2(&pathVAO);
        glDrawArrays(GL_LINES,0,lines.size()/3);

//        if(!isWholeView){
            pathProgram->setUniformValue("color",QVector4D(1,0,0,1));
            glPointSize(8);
            QOpenGLVertexArrayObject::Binder binder3(&vertexVAO);
            glDrawArrays(GL_POINTS,0,swcPoints.size()/3);
//        }

        glClear(GL_DEPTH_BUFFER_BIT);

        pathProgram->setUniformValue("color",QVector4D(0,1,0,1));
        QOpenGLVertexArrayObject::Binder binder8(&deletedLinesVAO);
        glDrawArrays(GL_LINES,0,deletedLines.size()/3);

        pathProgram->setUniformValue("color",QVector4D(0,0,1,1));
        QOpenGLVertexArrayObject::Binder binder7(&newLinesVAO);
        glDrawArrays(GL_LINES,0,newLines.size()/3);

       if(currentPointId != -1){
           //std::cout << "Current vertex : " << currentPointId << std::endl;
           pathProgram->setUniformValue("color",QVector4D(0.5,0.5,1,1));
           glPointSize(10);
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
           pathProgram->setUniformValue("color",QVector4D(0,1,0,1));
           glPointSize(8);
           QOpenGLVertexArrayObject::Binder binder9(&deleteVertexVAO);
           glDrawArrays(GL_POINTS,0,deletePoints.size()/3);
       }

        if(!newPoints.empty()){
            pathProgram->setUniformValue("color",QVector4D(0,0,1,1));
            glPointSize(8);
            QOpenGLVertexArrayObject::Binder binder10(&newPointsVAO);
            glDrawArrays(GL_POINTS,0,newPoints.size()/3);
        }

        if(!upmostVertex.empty()){
            pathProgram->setUniformValue("color",QVector4D(1,0,0,1));
            glPointSize(8);
            QOpenGLVertexArrayObject::Binder binder3(&upmostVertexVAO);
            glDrawArrays(GL_POINTS,0,upmostVertex.size()/3);

            pathProgram->setUniformValue("color",QVector4D(1,0,0,1));
            QOpenGLVertexArrayObject::Binder binder20(&selectedLineVAO);
            glDrawArrays(GL_LINES,0,selectedLine.size()/3);
        }

       pathProgram->release();
   }

//    std::cout <<"paint"<<std::endl;

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

    dimension = neuronInfo->blocksize + 4;
//    data.push_back(startx);
//    data.push_back(starty);
//    data.push_back(startz);
    data.push_back(x);
    data.push_back(y);
    data.push_back(z);
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

    newLines.clear();
    for(auto& item:neuronInfo->connectionList){
        newLines.push_back(item.x);
        newLines.push_back(item.y);
        newLines.push_back(item.z);
    }

    selectedLine.clear();
    if(lastSelectedPath != -1){
        auto path = neuronInfo->paths[lastSelectedPath].path;
        int id = path[0].current_id;
        while(true){
            auto vertex = neuronInfo->point_vector[neuronInfo->vertex_hash[id]];
            if(vertex.previous_id == -1)
                break;

            std:: cout << id << std::endl;

            auto preVertex = neuronInfo->point_vector[neuronInfo->vertex_hash[vertex.previous_id]];

            if(std::find(deletedPoints.begin(),deletedPoints.end(),vertex.current_id) == deletedPoints.end() &&
               std::find(deletedPoints.begin(),deletedPoints.end(),preVertex.current_id) == deletedPoints.end()){
                std::cout << "1" << std::endl;
                selectedLine.push_back(vertex.x);
                selectedLine.push_back(vertex.y);
                selectedLine.push_back(vertex.z);
                selectedLine.push_back(preVertex.x);
                selectedLine.push_back(preVertex.y);
                selectedLine.push_back(preVertex.z);
            }

            id = vertex.previous_id;
        }
    }

    makeCurrent();

    GenObject(pathVBO,pathVAO,lines.data(),lines.size() * sizeof(float));
    GenObject(deletedLinesVBO,deletedLinesVAO,deletedLines.data(),deletedLines.size() * sizeof(float));
    GenObject(newLinesVBO,newLinesVAO,newLines.data(),newLines.size() * sizeof(float));
    GenObject(selectedLineVBO,selectedLineVAO,selectedLine.data(),selectedLine.size() * sizeof(float));

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

    newPoints.clear();
    for(auto point:neuronInfo->addList){
        newPoints.push_back(point.x);
        newPoints.push_back(point.y);
        newPoints.push_back(point.z);
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

    upmostVertex.clear();
    if(lastSelectedPath != -1){
        auto path = neuronInfo->paths[lastSelectedPath];
        int id = path.path[0].current_id;
        while(true){
            auto vertex = neuronInfo->point_vector[neuronInfo->vertex_hash[id]];
            int cid = vertex.current_id;
            bool flag = true;
            if(cid == currentPointId || cid == lastPointId || cid == nextPointId){
                flag = false;
            }
            if(std::find(neuronInfo->deleteList.begin(),neuronInfo->deleteList.end(),cid) != neuronInfo->deleteList.end()){
                flag = false;
            }
            if(flag){
                upmostVertex.push_back(vertex.x);
                upmostVertex.push_back(vertex.y);
                upmostVertex.push_back(vertex.z);
            }


            id = vertex.previous_id;
            if(id == -1)
                break;
        }
    }

    GenObject(vertexVBO,vertexVAO,swcPoints.data(),swcPoints.size() * sizeof(float));
    GenObject(currentVertexVBO,currentVertexVAO,currentPoint.data(),currentPoint.size() * sizeof(float));
    GenObject(lastVertexVBO,lastVertexVAO,lastPoint.data(),lastPoint.size() * sizeof(float));
    GenObject(nextVertexVBO,nextVertexVAO,nextPoint.data(),nextPoint.size() * sizeof(float));
    GenObject(deleteVertexVBO,deleteVertexVAO,deletePoints.data(),deletePoints.size() * sizeof(float));
    GenObject(newPointsVBO,newPointsVAO,newPoints.data(),newPoints.size()*sizeof(float));
    GenObject(upmostVertexVBO,upmostVertexVAO,upmostVertex.data(),upmostVertex.size()*sizeof(float));

    doneCurrent();

    repaint();
}

void RenderWidget::GenObject(QOpenGLBuffer &vbo, QOpenGLVertexArrayObject &vao,float* data,int count) {
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
    pathProgram->setAttributeBuffer(0,GL_FLOAT,0,3,3 * sizeof(float));
    pathProgram->enableAttributeArray(0);
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
//    std::cout << event->pos().x() << "   " << event->pos().y() << std::endl;

    repaint();
}

void RenderWidget::pickPoint(QPointF mousePos){
    if(!ifRenderLine){
        return;
    }
    mousePos.setY(RENDER_HEIGHT - mousePos.y());
    float limit = 25;
    float mindis = limit + 1;
    int id = -1;
    if(!neuronInfo) return;

    auto pick = [&](std::vector<Vertex>& pointList){
        for(auto point:pointList){
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
    };

    pick(neuronInfo->point_vector);

    if(id != -1){
        int id0,id1;
        neuronInfo->GetVertexPathInfo(id,id0,id1);
        lastSelectedPath = id0;
    }

    pick(neuronInfo->addList);

    if(selectionState == SubwayMapWidget::SelectionState::Add){
        if(std::find(neuronInfo->deleteList.begin(),neuronInfo->deleteList.end(),id) != neuronInfo->deleteList.end()) return;
        if(connectStart == -1){
            if(id != -1){
                connectStart = id;
            }
            else{
                if(!neuronInfo) return;
                if(mapping_ptr) {
                    int newid = neuronInfo->Add(mapping_ptr);
                    loadSWCPoint();

                    connectStart = newid;
                }
            }
        }
        else{
            if(id != -1){
                neuronInfo->Connect(connectStart,id);
                loadLines();
                connectStart = -1;
            }
            else{
                if(!neuronInfo) return;
                int newid = neuronInfo->Add(mapping_ptr);
                neuronInfo->Connect(connectStart,newid);
                loadSWCPoint();
                loadLines();
                connectStart = newid;
            }
        }

//        if(id == -1){
//            if(!neuronInfo) return;
//            if(mapping_ptr) {
//                neuronInfo->Add(mapping_ptr);
//                loadSWCPoint();
//
//                if(connectStart == -1)
//                    connectStart = id;
//                else{
//
//                }
//            }
//        }
//        else{
//
//
//        }
//            for(auto item:neuronInfo->addList){
//                if(id == item.current_id){
//                    neuronInfo->DeleteVertex(id);
//                    loadSWCPoint();
//                    loadLines();
//                }
//            }

    }

    if(id == -1) return;

//    int id1,id2;
//    neuronInfo->GetVertexPathInfo(id,id1,id2);
    if(selectionState == SubwayMapWidget::SelectionState::Normal) {
        currentPointId = id;
//        currentPathId = id1;
//        currentInPathId = id2;

        loadSWCPoint();

        emit SelectPointSignal(id);
        return;
    }
    else if(selectionState == SubwayMapWidget::SelectionState::LastPoint){
//        bool valid = false;
//
//        if(currentPointId != -1){
//            if(id1 == currentPathId && id2 > currentInPathId)
//                valid =true;
//        }
//        else if(nextPointId != -1){
//            if(id1 == nextPathId && id2 > nextInPathId)
//                valid =true;
//        }
//        else
//            valid = true;
//
//        if(valid){
            lastPointId = id;
//            lastPathId = id1;
//            lastInPathId = id2;
            loadSWCPoint();
            emit SelectLastPointSignal(id);
//        }
    }
    else if(selectionState == SubwayMapWidget::SelectionState::NextPoint){
//        bool valid = false;
//        if(currentPointId != -1){
//            if(id1 == currentPathId && id2 < currentInPathId)
//                valid =true;
//        }
//        else if(lastPointId != -1){
//            if(id1 == nextPathId && id2 < lastInPathId)
//                valid =true;
//        }
//        else
//            valid = true;
//
//        if(valid){
            nextPointId = id;
//            nextPathId = id1;
//            nextInPathId = id2;
            loadSWCPoint();
            emit SelectNextPointSignal(id);
//        }
    }
    else if(selectionState == SubwayMapWidget::SelectionState::Delete){
        neuronInfo->DeleteVertex(id);
        loadSWCPoint();
        loadLines();
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
        inputWidget->ChangeNextPoint(-1);
        inputWidget->ChangeCurrentPoint(-1);
        inputWidget->ChangeLastPoint(-1);
    }
}

void RenderWidget::SelectPointSlot(int id) {
    int pathid,vertexid;
    neuronInfo->GetVertexPathInfo(id,pathid,vertexid);
    if(pathid != currentPathId)
        init();

    lastSelectedPath = pathid;
    isWholeView = false;
    currentPointId = id;

    const auto& vertex = neuronInfo->point_vector[neuronInfo->vertex_hash[currentPointId]];
    camera = std::make_unique<control::TrackBallCamera>(
            10,
            this->width(),this->height(),
            glm::vec3{vertex.x,
                      vertex.y,
                      vertex.z}
    );
    loadSWCPoint();
    loadLines();
    repaint();
}

void RenderWidget::SelectLastPointSlot(int id) {
    isWholeView = false;
    lastPointId = id;
    int id0,id1;
    neuronInfo->GetVertexPathInfo(id,id0,id1);
    lastSelectedPath = id0;
    loadSWCPoint();
    loadLines();
    repaint();
}

void RenderWidget::SelectNextPointSlot(int id) {
    isWholeView = false;
    nextPointId = id;
    int id0,id1;
    neuronInfo->GetVertexPathInfo(id,id0,id1);
    lastSelectedPath = id0;
    loadSWCPoint();
    loadLines();
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
    connect(inputWidget,&InputWidget::ChangeRenderOptionSignal,this,&RenderWidget::ChangeRenderOption);

    connect(inputWidget->tf_editor_widget,&TF1DEditor::TF1DChanged,[this](){
        float tf[256];
        inputWidget->tf_editor_widget->getTransferFunction(tf,256,1.0);
        resetTransferFunc1D(tf,256);
        repaint();
    });
}

void RenderWidget::resetTransferFunc1D(float *data, int dim) {

}

