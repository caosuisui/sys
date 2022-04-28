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
    void Show();

    /// 修改路径
    /// \param previousVertex
    /// \param modifiedVertex
    /// \param startPoint 影响的空间起点，在方法内分配内存并赋值
    /// \param dimension 影响的空间范围，在方法内分配内存并赋值
    void ModifyPaths(Vertex* previousVertex, Vertex* modifiedVertex,int* startPoint,int* dimension);

    /// 传入影响的空间范围，重构相交的体素块
    void PartialReconstruction();

    void Interpolate(int start, int end);

    void EditRadius(int id, double newRadius);

    void DeleteVertex(int id);

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

};

#endif //SYS_NEURONINFO_H
