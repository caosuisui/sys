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

    for(int i = 0;i < addList.size();i++){
        if(id == addList[i].current_id){
            addList.erase(addList.begin() + i);
            return;
        }
    }

    int i = 0;
    for(i = 0;i< deleteList.size();i++){
        if(deleteList[i] == id){
            deleteList.erase(deleteList.begin() + i);
            return;
        }
    }
    deleteList.push_back(id);
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

void NeuronInfo::PartialReconstruction() {
    //定位体素块
    double xmin,ymin,zmin,xmax,ymax,zmax;
    xmin = ymin = zmin = 999999;
    xmax = ymax = zmax = -999999;

    auto getBox = [&xmin,&ymin,&zmin,&xmax,&ymax,&zmax](Vertex item){
        xmin = std::min(xmin,item.x - item.radius);
        ymin = std::min(xmin,item.y - item.radius);
        zmin = std::min(xmin,item.z - item.radius);
        xmax = std::max(xmax,item.x + item.radius);
        ymax = std::max(xmax,item.y + item.radius);
        zmax = std::max(xmax,item.z + item.radius);
    };

    for(auto itemid: deleteList){
        auto item = point_vector[vertex_hash[itemid]];
        getBox(item);

        for(auto const& item1 : point_vector){
            if(item1.previous_id != item.current_id) continue;
            getBox(item1);
        }

        if(item.previous_id == -1) continue;
        auto preitem = point_vector[vertex_hash[item.previous_id]];
        getBox(preitem);
    }

    for(auto item: radiusChangeList){
        auto id = item.current_id;
        auto ver = point_vector[vertex_hash[id]];
        getBox(ver);

        for(auto const& item1 : point_vector){
            if(item1.previous_id == item.current_id){
                getBox(item1);
            }
        }

        if(item.previous_id != -1) {
            auto preitem = point_vector[vertex_hash[item.previous_id]];
            getBox(preitem);
        }
    }

    //不单独处理新建的节点，因为如果没有创建连接，则不应属于当前神经元

    //连接
    if(connectionList.size() != 0){
        for(int i = 0;i < connectionList.size() - 1;i += 2){
            auto start = connectionList[i];
            getBox(point_vector[vertex_hash[start.previous_id]]);

            auto end = connectionList[i+1];
            for(auto item:point_vector){
                if(item.previous_id == end.current_id){
                    getBox(item);
                }
            }

            getBox(start);
            getBox(end);
        }
    }

    int halfpadding = 1;
    int step = blocksize - 2;
    Vec3D<double> start_point{},end_point{};

    start_point.x = std::floor((xmin - x_start) / step) * step + x_start;
    start_point.y = std::floor((ymin - y_start) / step) * step + y_start;
    start_point.z = std::floor((zmin - z_start) / step) * step + z_start;
    end_point.x = std::floor((xmax - x_start) / step) * step + blocksize + x_start;
    end_point.y = std::floor((ymax - y_start) / step) * step + blocksize + y_start;
    end_point.z = std::floor((zmax - z_start)) / step) * step + blocksize + z_start;

    std::cout << "start_point: " << start_point.x << " " << start_point.y << " " << start_point.z << std::endl;
    std::cout << "dimension: " << end_point.x - start_point.x << " " << end_point.y - start_point.y << " " << end_point.z - start_point.z << std::endl;

//    std::string j = swcname.substr(swcname.size() - 8,4) ;
//    auto outputdir = "C:/Users/csh/Desktop/bishe/sys/output/" + j + '/';
//    auto objdir =outputdir + "obj";
//    auto volumedir = outputdir + "vol";
//
//    //重构体素场并写入体数据文件
//    auto newPointVector = GetNewSWC();
//    std::map<int,int> newVertexHash;
//
//    auto newSWC2VOL = new SWC2VOL;
//    auto newPaths = newSWC2VOL->searchPath(newPointVector,newVertexHash);
//    newSWC2VOL->GetVolume(start_point,end_point,newPaths,blocksize,volumedir);
//
//    //重新生成网格
//    if(dirExists(objdir))
//        RemoveDirectory(objdir.c_str()) ;
//    if(!dirExists(objdir)){
//        std::cout << "create dir for obj" << std::endl;
//        CreateDirectory(objdir.c_str(),NULL) ;
//    }
//
//    VOL2OBJ* mVOL2OBJ = new VOL2OBJ();
//    mVOL2OBJ->vol2obj(volumedir,objdir);
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
    for(int i = 0;i < connectionList.size() -1 ;i += 2){
         auto& item = connectionList[i];
         auto& nextItem = connectionList[i+1];
        if(item.current_id < point_vector.size() + 1){
            pointVectorCopy[vertex_hash[item.current_id]].previous_id = nextItem.current_id;
        }
        else{
            for(int j = 0;j < addList.size();j++){
                const auto& newItem = addList[i];
                if(item.current_id == newItem.current_id){
                    addListCopy[j].previous_id = nextItem.current_id;
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
        if(std::find(deleteList.begin(),deleteList.end(),point_vector[i].current_id) == deleteList.end())
            newPointVector.push_back(pointVectorCopy[i]);
    }

    for(auto item:addListCopy)
        newPointVector.push_back(item);

    return std::move(newPointVector);
}