//
// Created by csh on 4/8/2022.
//

#include <swc2vol.h>
#include "NeuronInfo.h"
#include <windows.h>
#include <vol2obj.h>

static bool dirExists(const std::string& dirName_in) {
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!

    return false;    // this is not a directory!
}

void NeuronInfo::EditRadius(int id, double newRadius) {
    std:: cout << "point " << id << " radius change to " <<  newRadius << std::endl;

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
    std::cout << "delete vertex " << id << std::endl;

    auto deleteConnection = [&](int id){
        std::stack<int> toDelete;
        for(int j = 0;j < connectionList.size();j+=2){
            if(connectionList[j].current_id == id || connectionList[j+1].current_id == id){
                toDelete.push(j);
                toDelete.push(j+1);

                std::cout << "delete connection between " << connectionList[j].current_id << " and " << connectionList[j+1].current_id << std::endl;
            }
        }

        while(!toDelete.empty()){
            int lastid = toDelete.top();
            toDelete.pop();

            connectionList.erase(connectionList.begin() + lastid);
        }
    };

    //删除已添加的点
    for(int i = 0;i < addList.size();i++){
        if(id == addList[i].current_id){
            deleteConnection(id);
            addList.erase(addList.begin() + i);
            return;
        }
    }

    //恢复已删除的点
    int i = 0;
    for(i = 0;i< deleteList.size();i++){
        if(deleteList[i] == id){
            deleteList.erase(deleteList.begin() + i);
            return;
        }
    }

    //删除点
    deleteList.push_back(id);
    deleteConnection(id);
}

void NeuronInfo::Connect(int start,int end){
    if(start == end) return;
    //如果之前有相同的连接，表示这是删除连接而不是创建连接
    std::stack<int> toDelete;
    for(int i = 0;i <connectionList.size();i+=2){
        if((start == connectionList[i].current_id && end == connectionList[i+1].current_id) ||
                (end == connectionList[i].current_id && start == connectionList[i+1].current_id)){
            toDelete.push(i);
            toDelete.push(i+1);
        }
    }

    if(!toDelete.empty()){
        while(!toDelete.empty()){
            int offset = toDelete.top();
            toDelete.pop();
            connectionList.erase(connectionList.begin() + offset);
        }
        return;
    }

    if(point_vector[vertex_hash[start]].previous_id == end || point_vector[vertex_hash[end]].previous_id == start) return;

    //创建连接
    std::cout << "connect between " << start << " " << "and " << end << std::endl;
    if(start < point_vector.size()){
        connectionList.push_back(point_vector[vertex_hash[start]]);
    }
    else{
        for(auto item:addList){
            if(item.current_id == start) {

                connectionList.push_back(item);
                break;
            }
        }
    }

    if(end < point_vector.size()){
        connectionList.push_back(point_vector[vertex_hash[end]]);
    }

    else{
        for(auto item:addList){
            if(item.current_id == end) {
                connectionList.push_back(item);
                break;
            }
        }
    }
}

int NeuronInfo::Add(float* mapping_ptr){
    Vertex newVertex;
    newVertex.current_id = ++vertexCount;
    newVertex.radius = mapping_ptr[7];
    newVertex.x = (mapping_ptr[0] + mapping_ptr[4]) / 2;
    newVertex.y = (mapping_ptr[1] + mapping_ptr[5]) / 2;
    newVertex.z = (mapping_ptr[2] + mapping_ptr[6]) / 2;
    addList.push_back(newVertex);

    std::cout << "new point :" << vertexCount << " " << newVertex.x << " " << newVertex.y << " " << newVertex.z << " "
              << newVertex.radius << std::endl;

    return newVertex.current_id;
}

double NeuronInfo::CheckR(int id) {
    for(auto item:radiusChangeList){
        if(item.current_id == id)
            return item.radius;
    }
    return -1;
}

double NeuronInfo::GetR(int id) {
    for(auto item:radiusChangeList){
        if(item.current_id == id)
            return item.radius;
    }
    return point_vector[vertex_hash[id]].radius;
}

std::string NeuronInfo::PartialReconstruction() {
    std::string j = swcname.substr(swcname.size() - 8,4) ;
    auto outputdir = "C:/Users/csh/Desktop/bishe/sys/output/" + j + '/';
    auto objdir = outputdir + "obj/";
    auto volumedir = outputdir + "vol/";
    auto objname = objdir + "simplifiedObj.obj";

    if(addList.empty() && deleteList.empty() && connectionList.empty() && radiusChangeList.empty()) return objname;

    //定位体素块
    double xmin, ymin, zmin, xmax, ymax, zmax;
    xmin = ymin = zmin = 999999;
    xmax = ymax = zmax = -999999;

    auto getBox = [&xmin, &ymin, &zmin, &xmax, &ymax, &zmax](Vertex item) {
        xmin = std::min(xmin, item.x - item.radius);
        ymin = std::min(ymin, item.y - item.radius);
        zmin = std::min(zmin, item.z - item.radius);
        xmax = std::max(xmax, item.x + item.radius);
        ymax = std::max(ymax, item.y + item.radius);
        zmax = std::max(zmax, item.z + item.radius);

//        std::cout << "space " << xmin << " " << ymin << " " << zmin << " " << xmax << " " << ymax << " "<< zmax << std::endl;
    };

    for (auto itemid: deleteList) {
        auto item = point_vector[vertex_hash[itemid]];
        getBox(item);

        for (auto const &item1 : point_vector) {
            if (item1.previous_id == item.current_id) {
                getBox(item1);
            }
        }

        if (item.previous_id != -1) {
            auto preitem = point_vector[vertex_hash[item.previous_id]];
            getBox(preitem);
        }
    }

    for (auto item: radiusChangeList) {
        auto id = item.current_id;
        auto ver = point_vector[vertex_hash[id]];
        getBox(ver);

        for (auto const &item1 : point_vector) {
            if (item1.previous_id == item.current_id) {
                getBox(item1);
            }
        }

        if (item.previous_id != -1) {
            auto preitem = point_vector[vertex_hash[item.previous_id]];
            getBox(preitem);
        }
    }

    //不单独处理新建的节点，因为如果没有创建连接，则不应属于当前神经元

    //连接
    if (!connectionList.empty()) {
        for (int i = 0; i < connectionList.size() - 1; i += 2) {
            auto start = connectionList[i];
            getBox(point_vector[vertex_hash[start.previous_id]]);

            auto end = connectionList[i + 1];
            for (auto item:point_vector) {
                if (item.previous_id == end.current_id) {
                    getBox(item);
                }
            }

            getBox(start);
            getBox(end);
        }
    }

    int halfpadding = 1;
    int step = blocksize - 2;
    Vec3D<double> start_point{}, end_point{};

    start_point.x = std::floor((xmin - x_start) / step) * step + x_start;
    start_point.y = std::floor((ymin - y_start) / step) * step + y_start;
    start_point.z = std::floor((zmin - z_start) / step) * step + z_start;
    end_point.x = std::floor((xmax - x_start) / step) * step + step + x_start;
    end_point.y = std::floor((ymax - y_start) / step) * step + step + y_start;
    end_point.z = std::floor((zmax - z_start) / step) * step + step + z_start;

    start_point.x = (start_point.x - step <= x_start) ? start_point.x : start_point.x - step;
    start_point.y = (start_point.y - step <= y_start) ? start_point.y : start_point.y - step;
    start_point.z = (start_point.z - step <= z_start) ? start_point.z : start_point.z - step;

//    end_point.x = (end_point.x + step >= x_start + x_dimension) ? end_point.x : end_point.x + step;
//    end_point.y = (end_point.y + step >= y_start + y_dimension) ? end_point.y : end_point.y + step;
//    end_point.z = (end_point.z + step >= z_start + z_dimension) ? end_point.z : end_point.z + step;

    std::cout << "start_point: " << start_point.x << " " << start_point.y << " " << start_point.z << std::endl;
    std::cout << "dimension: " << end_point.x - start_point.x << " " << end_point.y - start_point.y << " "
              << end_point.z - start_point.z << std::endl;


    //重构体素场并写入体数据文件
    auto newPointVector = GetNewSWC();
    std::map<int,int> newVertexHash;

    auto newSWC2VOL = new SWC2VOL;
    auto newPaths = newSWC2VOL->searchPath(newPointVector,newVertexHash);
    newSWC2VOL->GetVolume(start_point,end_point,newPaths,blocksize,volumedir);

    //重新生成网格
    if(dirExists(objdir)){
        RemoveDirectory(objdir.c_str()) ;
    }

    if(!dirExists(objdir)){
        std::cout << "create dir for obj" << std::endl;
        CreateDirectory(objdir.c_str(),NULL) ;
    }

    VOL2OBJ* mVOL2OBJ = new VOL2OBJ();
    mVOL2OBJ->vol2obj(volumedir,objdir);

    //
    {
        std::cout << "vertex number: "<< point_vector.size() << " to " << newPointVector.size() <<std::endl;
        point_vector = newPointVector;
        vertex_hash = newVertexHash;
        paths = newPaths;
        mainPaths = newSWC2VOL->mainPathsIndex;
        radiusChangeList.clear();
        connectionList.clear();
        deleteList.clear();
        addList.clear();
        vertexCount = point_vector.size();
    }

    ExportSWC("aaa");
    return objname;
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

void NeuronInfo::Interpolate(int end, int start) {
//    int tmp = start;
//    start = end;
//    end = tmp;
    double length = 0;
    int i;
    for(i = start;i != end;i = point_vector[vertex_hash[i]].previous_id){
        length += point_vector[vertex_hash[i]].distance;
    }
    if(i != end) return;

    auto startR = GetR(start);
    auto endR = GetR(end);
    std::cout << "start R " << startR  << std::endl;
    std::cout << "end R " << endR << std::endl;

    double pos = 0;
    pos += point_vector[vertex_hash[start]].distance;
    for(i = point_vector[vertex_hash[start]].previous_id; i != end;i = point_vector[vertex_hash[i]].previous_id){
        Vertex newVertex;
        newVertex.current_id = i;
        double newR = startR + (endR - startR) * (pos / length);
        EditRadius(i,newR);

        pos += point_vector[vertex_hash[i]].distance;
    }
}

Vertex* NeuronInfo::GetVertex(int id){
    if(id < point_vector.size()){
        return &point_vector[vertex_hash[id]];
    }
    else{
        for(auto& item:addList){
            if(item.current_id == id){
                return &item;
            }
        }
    }
//    std::cerr << "GET VERTEX ERROR : no vertex " << id << std::endl;
    return nullptr;
}

std::vector<Vertex> NeuronInfo::GetNewSWC() {
    std::vector<Vertex> pointVectorCopy = point_vector;
    std::vector<Vertex> addListCopy = addList;
    std::vector<Vertex> newPointVector;

    //修改半径
    for(auto item:radiusChangeList){
        for(int i = 0;i < point_vector.size();i++){
            const auto& item1 = point_vector[i];
            if(item.current_id == item1.current_id){
                pointVectorCopy[i].radius = item.radius;
            }
        }
    }

    //连接
    if(!connectionList.empty()){
        for(int i = 0;i < connectionList.size() -1 ;i += 2){
            auto& item = connectionList[i];
            auto& nextItem = connectionList[i+1];
            if(item.current_id < point_vector.size() + 1){
                pointVectorCopy[vertex_hash[item.current_id]].previous_id = nextItem.current_id;
            }
            else{
                for(int j = 0;j < addList.size();j++){
//                    auto newItem = addList[i];
                    if(item.current_id == addList[i].current_id){
                        addListCopy[j].previous_id = addList[i].current_id;
                    }
                }
            }
        }
    }

    //分配id
    int count = 0;
    for(int j = 0;j < point_vector.size();j++){
        const auto& item = point_vector[j];
        if(std::find(deleteList.begin(),deleteList.end(),item.current_id) == deleteList.end()){
            int newCurrentId = ++count;//从1开始？
            pointVectorCopy[j].current_id = newCurrentId;

            for(int i = 0;i < point_vector.size();i++){
                const auto& item1 = point_vector[i];
                if(item1.previous_id == item.current_id){
                    pointVectorCopy[i].previous_id = newCurrentId;
                }
            }
            for(int i= 0;i < addList.size();i++){
                const auto& item1 = addList[i];
                if(item1.previous_id == item.current_id){
                    addListCopy[i].previous_id = newCurrentId;
                }
            }
        }
    }

    for(int j = 0;j < addList.size();j++){
        const auto& item = addList[j];
        int newCurrentId = ++count;
        addListCopy[j].current_id = newCurrentId;

        for(int i = 0;i < point_vector.size();i++){
            const auto& item1 = point_vector[i];
            if(item1.previous_id == item.current_id){
                pointVectorCopy[i].previous_id = newCurrentId;
            }
        }
        for(int i= 0;i < addList.size();i++){
            const auto& item1 = addList[i];
            if(item1.previous_id == item.current_id){
                addListCopy[i].previous_id = newCurrentId;
            }
        }
    }

    newPointVector.reserve(pointVectorCopy.size() - deleteList.size() + addListCopy.size());

    for(int i = 0;i < point_vector.size();i++){
        pointVectorCopy[i].is_visited = false;
        pointVectorCopy[i].degree = 0;
        pointVectorCopy[i].distance = 0;

        if(std::find(deleteList.begin(),deleteList.end(),point_vector[i].current_id) == deleteList.end()){
            newPointVector.push_back(pointVectorCopy[i]);
        }
    }

    for(auto item:addListCopy){
        item.is_visited = false;
        newPointVector.push_back(item);
    }

    return std::move(newPointVector);
}

void NeuronInfo::ExportSWC(std::string filename){
    filename = "C:/Users/csh/Desktop/bishe/" + swcname.substr(swcname.size() - 8);
    std::ofstream swcfile(filename,std::ios::out | std::ios::trunc);
    if(!swcfile.is_open()){
        std::cerr << "save failed" << std::endl;
        return;
    }

    for(auto item:point_vector){
        swcfile << item.current_id << " "<<
        item.point_type << " "<< setiosflags(ios::fixed) << setprecision(4) <<
        item.x << " " << setiosflags(ios::fixed) << setprecision(4) <<
        item.y << " " << setiosflags(ios::fixed) << setprecision(4) <<
        item.z << " " << setiosflags(ios::fixed) << setprecision(4) <<
        item.radius << " " <<
        item.previous_id << std::endl;
    }

    swcfile.close();
}