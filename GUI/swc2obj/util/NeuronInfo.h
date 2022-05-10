//
// Created by csh on 4/8/2022.
//

#ifndef SYS_NEURONINFO_H
#define SYS_NEURONINFO_H


#include <iostream>
#include "../data/SWCReader.h"
#include "../data/cmdline.h"
#include "BVH.h"
#include "../data/based.h"

class NeuronInfo{

public:
    void GetVertexPathInfo(int currentid,int& pathid, int& vertexid);

    /// 传入影响的空间范围，重构相交的体素块
    std::string PartialReconstruction();

    void Interpolate(int start, int end);

    void EditRadius(int id, double newRadius);

    void DeleteVertex(int id);

    void Connect(int start,int end);

    int Add(float* mapping_ptr);

    std::vector<Vertex> GetNewSWC();

    double CheckR(int id);

    double GetR(int id);

    Vertex* GetVertex(int id);

public:
    int x_dimension;
    int y_dimension;
    int z_dimension;

    double x_space;
    double y_space;
    double z_space;

    double x_start;
    double y_start;
    double z_start;

    std::vector<Vertex> point_vector;

    std::map<int, int> vertex_hash;

    BVH* bvh;

    std::vector<Path> paths;
    std::vector<int> mainPaths;

    int blocksize;

    std::vector<Vertex> radiusChangeList;
    std::vector<Vertex> addList;
    std::vector<int> deleteList;

    std::vector<Vertex> connectionList;

    std::string swcname;

    int vertexCount;

};

#endif //SYS_NEURONINFO_H
