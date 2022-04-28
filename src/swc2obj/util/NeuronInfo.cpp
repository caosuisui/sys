//
// Created by csh on 4/8/2022.
//

#include "NeuronInfo.h"

void NeuronInfo::Show() {
    std::cout << blocksize << std::endl;
}

void NeuronInfo::ModifyPaths(Vertex *previousVertex, Vertex *modifiedVertex, int *startPoint, int *dimension) {

}

void NeuronInfo::EditRadius(int id, double newRadius) {
    for(auto& item:radiusChangeList ){
        if(item.current_id == id){
            item.radius = newRadius;
            return;
        }
    }

    Vertex newVertex;
    newVertex.current_id = id;
    newVertex.radius = newRadius;
    radiusChangeList.push_back(newVertex);
}

void NeuronInfo::DeleteVertex(int id) {
    int i = 0;
    for(i = 0;i< deleteList.size();i++){
        if(deleteList[i] == id){
            deleteList.erase(deleteList.begin() + i);
            return;
        }
    }
    deleteList.push_back(id);
}

void NeuronInfo::PartialReconstruction() {
    //定位体素块

    //构建局部体素块

    //写入体数据文件
}

void NeuronInfo::GetVertexPathInfo(int currentid, int &pathid, int &vertexid) {
    for(auto path:paths){
        for(int i = 0;i < path.path.size();i++){
            if(currentid == path.path[i].current_id){
                pathid = path.path_type;
                vertexid = i;
                return;
            }
        }
    }
}

void NeuronInfo::Interpolate(int start, int end) {
    double length = 0;
    int i;
    for(i = start;i != end;i = point_vector[vertex_hash[i]].previous_id){
        length += point_vector[vertex_hash[i]].distance;
    }
    if(i != end) return;


    auto startR = point_vector[vertex_hash[start]].radius;
    auto endR = point_vector[vertex_hash[end]].radius;

    double pos = 0;
    pos += point_vector[vertex_hash[start]].distance;
    for(int i = point_vector[vertex_hash[start]].previous_id; i != end;i = point_vector[vertex_hash[i]].previous_id){
        Vertex newVertex;
        newVertex.current_id = i;
        newVertex.radius = startR + (endR - startR) * (pos / length);
        radiusChangeList.push_back(newVertex);

        pos += point_vector[vertex_hash[i]].distance;
    }
}