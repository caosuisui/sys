#pragma once

#include <iostream>
#include <fstream>
#include <glm/gtx/string_cast.hpp>
#include <omp.h>
#include <io.h>
#include <algorithm>

#include "NeuronInfo.h"
#include "SWCReader.h"
#include "cmdline.h"
#include "BVH.h"
#include "config.h"
#include "based.h"

#include <vtkImageReader.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyData.h>
#include <vtkAppendPolyData.h>
#include <vtkOBJWriter.h>


class SWC2VOL {
public:
    std::vector<Path> searchPath(std::vector<Vertex> &point_vector, std::map<int, int> &vertex_hash) {
        mainPathsIndex.clear();
        std::vector<int> leaf_nodes;
//	std::map<int, int> vertex_hash;
        for (auto i = 0; i < point_vector.size(); i++) {
            vertex_hash[point_vector[i].current_id] = i;
        }

        for (auto &i : point_vector) {
            if (i.previous_id != -1) {
                point_vector[vertex_hash[i.previous_id]].degree++;//子节点数量即degree
            }
        }


        for (auto &i : point_vector) {
            if (i.degree == 0 && i.previous_id != -1) {
                leaf_nodes.push_back(i.current_id);
            }
        }


        std::vector<Path> paths;
        auto path_index = 0;
        for (auto &left_node : leaf_nodes) {
            Path path;
            std::vector<Vertex> vertices;
            double length = 0;

            auto cur_idx = left_node;
            while (cur_idx != -1 && !point_vector[vertex_hash[cur_idx]].is_visited) {
                auto &buf_vertex = point_vector[vertex_hash[cur_idx]];

                auto previous_id = buf_vertex.previous_id;
                if (previous_id != -1) {
                    auto &pre_vertex = point_vector[vertex_hash[previous_id]];
                    buf_vertex.distance = calculateDistance(buf_vertex, pre_vertex);
                    length += buf_vertex.distance;
                } else {
                    buf_vertex.distance = 0;
                }

                buf_vertex.is_visited = true;
                vertices.push_back(buf_vertex);
                cur_idx = previous_id;
            }

            if (cur_idx != -1) {
                vertices.push_back(point_vector[vertex_hash[cur_idx]]);
//                if (point_vector[vertex_hash[cur_idx]].previous_id != -1) {
                    int vertexIndexInParentPath = -1;
                    int parentPath = -1;
                    findParentPath(paths, point_vector[vertex_hash[cur_idx]], path_index, vertexIndexInParentPath,
                                   parentPath);
                    path.branch_point_index = vertexIndexInParentPath;
                    path.parent_path = parentPath;
//                }
            }
            else{
                path.branch_point_index = -1;
                path.parent_path = -1;
            }

            path.path = vertices;
            path.path_type = path_index;
            path.length = length;
            path_index++;
            paths.push_back(path);
        }

//        mainPathsIndex.push_back(0);
        for (auto &path : paths) {
            if (path.path.back().previous_id == -1 && path.parent_path == -1) {
                mainPathsIndex.push_back(path.path_type);
//                path.branch_point_index = -1;
//                path.parent_path = -1;
            }

            //重新对子分支排序，越靠近叶节点的越前
            sort(path.sub_paths_index.begin(), path.sub_paths_index.end(),
                 [&paths](int id1, int id2) -> bool {
                     int n = paths[id1].branch_point_index - paths[id2].branch_point_index;//越小越靠近叶节点
                     if (n > 0)
                         return false;
                     else if (n < 0)
                         return true;
                     else {
                         //分支点的下一个点半径小的更靠前，算更靠近根节点
                         double r1 = paths[id1].path[paths[id1].path.size() - 2].radius;
                         double r2 = paths[id2].path[paths[id2].path.size() - 2].radius;
                         if (r1 >= r2)
                             return false;
                         else
                             return true;
                     }
                 });
        }

        return paths;
    }

    double calculateDistance(const Vertex &v1, const Vertex &v2) {
        return std::sqrt(std::pow(std::abs(v1.x - v2.x), 2) + std::pow(std::abs(v1.y - v2.y), 2) +
                         std::pow(std::abs(v1.z - v2.z), 2));

    }

    void findParentPath(std::vector<Path> &paths, const Vertex &vertex, int pathIndex, int &vertexIndexInParentPath,
                        int &parentPath) {
        int vertexCurrentID = vertex.current_id;
        for (auto &path:paths) {
            for (int i = 0; i < path.path.size(); i++) {
                if (path.path[i].current_id == vertexCurrentID) {
                    path.sub_paths_index.push_back(pathIndex);
                    vertexIndexInParentPath = i;
                    parentPath = path.path_type;
                    return;
                }
            }
        }
    }

//求整体的包围盒，考虑点的半径，更新起始点和维度
    void boundingBox(std::vector<Vertex> &point_vector,
                     double x_space, double y_space, double z_space,
                     Vec3D<double> &start_position,
                     Vec3D<int> &dimension) {

        double max_x = -999999, min_x = 999999;
        double max_y = -999999, min_y = 999999;
        double max_z = -999999, min_z = 999999;

        for (auto &vertex : point_vector) {

            //write_file << "v " << (vertex.x) / x_space << " " << (vertex.y) / y_space << " " << (vertex.z) / z_space << std::endl;
            //vertex.x /= x_space;
            //vertex.y /= y_space;
            //vertex.z /= z_space;
//            vertex.x /= 0.5;//为什么要*2
//            vertex.y /= 0.5;
//            vertex.z /= 0.5;
//		vertex.radius /= 0.5 ;

            max_x = std::max(max_x, vertex.x + RADIUS * vertex.radius);//球的半径单位长度为RADIOUS
            max_y = std::max(max_y, vertex.y + RADIUS * vertex.radius);
            max_z = std::max(max_z, vertex.z + RADIUS * vertex.radius);
            min_x = std::min(min_x, vertex.x - RADIUS * vertex.radius);
            min_y = std::min(min_y, vertex.y - RADIUS * vertex.radius);
            min_z = std::min(min_z, vertex.z - RADIUS * vertex.radius);
        }

        std::cout << "Bounding box : [" << min_x << ", " << min_y << ", " << min_z << "] --- [" << max_x << ", "
                  << max_y << ", " << max_z << "]" << std::endl;

        std::cout << "Dimension x : " << (max_x - min_x) << std::endl;
        std::cout << "Dimension y : " << (max_y - min_y) << std::endl;
        std::cout << "Dimension z : " << (max_z - min_z) << std::endl;

        start_position = {min_x - 1, min_y - 1, min_z - 1};
        dimension = {int(max_x - min_x + 2), int(max_y - min_y + 2), int(max_z - min_z + 2)};
    }

    void getSphereBoundingBox(Vec3D<double> &center_point, double radius, Vec3D<int> &low_bounding_int,
                              Vec3D<int> &high_bounding_int) {
        auto low_bounding = center_point - radius;
        low_bounding_int = {int(center_point.x - radius), int(center_point.y - radius), int(center_point.z - radius)};
        high_bounding_int = {int(ceil(center_point.x + radius) + 0.1), int(ceil(center_point.y + radius) + 0.1),
                             int(ceil(center_point.z + radius) + 0.1)};
    }

    Sphere *getSphere(Vec3D<double> &center_point, double radius) {

        Sphere *ss = new Sphere(radius, center_point.to_vec3());
        return ss;
    }

    void average1(std::vector<float> *mask_vector, glm::dvec3 dimension) {
        int offset[8][3] = {{0, 0, 0},
                            {0, 0, 1},
                            {0, 1, 0},
                            {0, 1, 1},
                            {1, 0, 0},
                            {1, 0, 1},
                            {1, 1, 0},
                            {1, 1, 1}};//取周围的8个点进行均值计算
//    std::cout<<glm::to_string(glm::dvec3(dimension))<<std::endl ;
//    std::cout<<mask_vector->size()<<std::endl ;
        for (auto k = 0; k < dimension.z - 1; k++) {
            for (auto j = 0; j < dimension.y - 1; j++) {
                for (auto i = 0; i < dimension.x - 1; i++) {
                    auto sum = 0.0;
                    for (auto &p : offset) {
                        sum += (*mask_vector)[(p[0] + k) * int(dimension.x) * int(dimension.y) +
                                              (p[1] + j) * int(dimension.x) + (p[2] + i)];
                    }
                    (*mask_vector)[k * int(dimension.x) * int(dimension.y) + j * int(dimension.x) + i] = sum / 8;
                }
            }
            std::cout << "Process for averaging mask vector : " << k << " in " << dimension.z - 1 << std::endl;
        }
    }

    void averageAll(BVHNode *root) {
        if (root->id >= 0 && root->isUsingNode()) average1(root->getData(), root->getDimension());
        else {
            if (root->children[0] != nullptr) averageAll(root->children[0]);
            if (root->children[1] != nullptr) averageAll(root->children[1]);
        }
    }

    void change(BVHNode *node) {
        int padding = 4;
        glm::dvec3 dim = node->getDimension();
        glm::dvec3 newdim = node->getDimension() - glm::dvec3(padding, padding, padding);
        std::vector<float> *data = node->getData();
        std::vector<float> *t = new std::vector<float>(int(newdim[0] * newdim[1] * newdim[2]));

        for (int i = 0; i < newdim[2]; i++) {
            for (int j = 0; j < newdim[1]; j++) {
                for (int z = 0; z < newdim[0]; z++) {
                    (*t)[i * int(newdim[1]) * int(newdim[0]) + j * int(newdim[0]) + z] =
                            (*data)[(i + 2) * int(dim[1]) * int(dim[0]) + (j + 2) * int(dim[0]) + (z + 2)];
                }//平移
            }
        }
        std::cout << "new dimension is" << glm::to_string(newdim) << std::endl;

        glm::dvec3 start = node->getStart() + glm::dvec3(padding / 2, padding / 2, padding / 2);
        VoxelBox *box = new VoxelBox(start, start + newdim);
        box->data = t;

        node->setNewBox(box);
    }

//原来padding为4，相邻体素块有2个单位的面重叠，各自减小2并前移2即可消除重叠
    void changeAll(BVHNode *root) {
        if (root->id >= 0 && root->isUsingNode()) change(root);
        else {
            if (root->children[0] != nullptr) changeAll(root->children[0]);
            if (root->children[1] != nullptr) changeAll(root->children[1]);
        }
    }

    void
    writeMHD(std::string file_name, int NDims, glm::dvec3 DimSize, glm::dvec3 ElementSize, glm::dvec3 ElementSpacing,
             glm::dvec3 Position, bool ElementByteOrderMSB, std::string ElementDataFile) {
        std::ofstream writer(file_name.c_str(), ios::trunc);
        writer << "NDims = " << NDims << std::endl;
        writer << "DimSize = " << DimSize.x << " " << DimSize.y << " " << DimSize.z << std::endl;
        writer << "ElementSize = " << ElementSize.x << " " << ElementSize.y << " " << ElementSize.z << std::endl;
        writer << "ElementSpacing = " << ElementSpacing.x << " " << ElementSpacing.y << " " << ElementSpacing.z
               << std::endl;
        writer << "ElementType = " << "MET_DOUBLE" << std::endl;
        writer << "Position = " << Position.x << " " << Position.y << " " << Position.z << std::endl;
        if (ElementByteOrderMSB)
            writer << "ElementByteOrderMSB = " << "True" << std::endl;
        else
            writer << "ElementByteOrderMSB = " << "False" << std::endl;
        writer << "ElementDataFile = " << ElementDataFile << std::endl;
        std::cout << "File " << ElementDataFile << " has been saved." << std::endl;
    }

    void write(std::vector<float> &mask_vector, std::string rawname, std::string mhdname, glm::dvec3 dimension,
               glm::dvec3 start_point) {
        std::ofstream outFile(rawname, std::ios::out | std::ios::binary | ios::trunc);

        double t = 0.0;
        for (int i = 0; i < mask_vector.size(); i++) {
            t = mask_vector[i];
            outFile.write(reinterpret_cast<char *>(&t), sizeof(double));
        }
        std::cout << "mask_vector" << mask_vector.size() << std::endl;
        outFile.close();
//    start_point[0] /= 0.32 ;
//    start_point[1] /= 0.32 ;
//    start_point[2] /= 2 ;

        std::cout << "File has been saved." << std::endl;
        //      writeMHD(mhdname, 3, dimension, { 1, 1, 1 }, { 0.5, 0.5, 0.5 }, start_point/double(2.0), false, rawname);
        writeMHD(mhdname, 3, dimension, {1, 1, 1}, {1.0, 1.0, 1.0}, start_point, false, rawname);
    }

//每个box都输出一个文件
    void writeAll(BVHNode *root, std::string &path) {
        if (root->id >= 0 && root->isUsingNode()) {
            std::string name = path +
//                                std::to_string(filecount++) + "_" +//输出编号
                               std::to_string(int(root->box->getStart()[0])) + "_" +
                               std::to_string(int(root->box->getStart()[1])) + "_" +
                               std::to_string(int(root->box->getStart()[2])) + ".";//经过的空间的起点
            std::string rawname = name + "raw";
            std::string mhdname = name + "mhd";



            write(*root->getData(), rawname, mhdname, root->getDimension(), root->box->getStart());
        } else {
            if (root->children[0] != nullptr) writeAll(root->children[0], path);
            if (root->children[1] != nullptr) writeAll(root->children[1], path);
        }
    }

    int getDetph(std::vector<Path> &paths, std::vector<Vertex> &point_vector, std::map<int, int> &vertex_hash) {
        int maxid = -1;
        double max_degree = -1;

        //遍历所有路径，找出最大degree
        for (auto &path : paths) {
            for (int i = 0; i < path.path.size(); i++) {
                if (path.path[i].degree > max_degree) {
                    max_degree = path.path[i].degree;
                    maxid = path.path[i].current_id;
                }
            }
        }

        std::cout << "get maxid is " << maxid << std::endl;
        int id = maxid;
        int countdis = 0;
        //更新degree最大的点所在路径上，该点及之后的点的深度信息，并调整半径
        while (id != -1) {
            point_vector[vertex_hash[id]].depth = countdis++;//从degree最大的点开始从0递增

//            double radius1 = point_vector[vertex_hash[id]].radius ;
//            int degree1 = point_vector[vertex_hash[id]].degree ;
            id = point_vector[vertex_hash[id]].previous_id;

            //如果前一个点不是原点且半径大于当前点的半径则将前一个点的半径改为当前点半径
            //保证从起始点到原点之间的点半径不递减
//            if(id!=-1 && point_vector[vertex_hash[id]].radius>radius1)
//            {
//                std::cout<<id<<" a change to "<<radius1<<std::endl ;
//                point_vector[vertex_hash[id]].radius = radius1 ;
//            }
        }

        //存储所有路径起始点的id
        std::stack<int> point_vector_index;
        for (int i = 0; i < paths.size(); i++) {
            point_vector_index.push(paths[i].path[0].current_id);
        }

        while (point_vector_index.size() != 0)//对每条路径
        {
            int id = point_vector_index.top();//第一个点
            int next_id = point_vector[vertex_hash[id]].previous_id;
            if (next_id == -1)//如果当前点是第一个点
            {
                point_vector[vertex_hash[id]].depth = -2;//修改深度为-2
                point_vector_index.pop();
                continue;
            }
            int depth_next = point_vector[vertex_hash[next_id]].depth;
            double radius_next = point_vector[vertex_hash[next_id]].radius;
            int degree_next = point_vector[vertex_hash[next_id]].degree;

            if (radius_next == 0.0) {
                std::cout << point_vector[vertex_hash[next_id]].current_id << " radius zero" << std::endl;
            }

            if (depth_next == -2) {
                point_vector[vertex_hash[id]].depth = -2;//与原点在同一个路径上的点的深度为-2？
                point_vector_index.pop();
                continue;
            } else if (depth_next >= 0) {
                point_vector[vertex_hash[id]].depth = depth_next + 1;//maxdegree点所在的路径上，点的深度是下一个点的深度+1
                if (point_vector[vertex_hash[id]].radius >= radius_next) {
                    std::cout << id << " radius " << point_vector[vertex_hash[id]].radius << " change to "
                              << radius_next << std::endl;
                    point_vector[vertex_hash[id]].radius = radius_next;//保证从起始点到之后点的半径不递减，原点半径最大
                }

                point_vector_index.pop();
                continue;
            } else if (depth_next == -1) {
                point_vector_index.push(next_id);//处理所有路径上的第一个符合前两个条件的点
                continue;
            }
        }

        for (int j = 0; j < paths.size(); j++) {
            for (int i = 0; i < paths[j].path.size(); i++) {
                paths[j].path[i].radius = point_vector[vertex_hash[paths[j].path[i].current_id]].radius;
            }//对path上的半径赋值
        }
        std::cout << " get depth end" << std::endl;
        return maxid;

    }

    int swc2vol(std::string swc_file, int blocksize, std::string volume_dir) {
        std::cout << "we has " << processor_N << " processor" << std::endl;

        int x_dimension = 28452;
        int y_dimension = 21866;
        int z_dimension = 4834;

        double x_space = 1;
        double y_space = 1;
        double z_space = 1;

        //int x_n = 889;
        //int y_n = 683;
        //int z_n = 151;

        SWCReader swc_reader;
        swc_reader.readSWC(swc_file);

        auto point_vector = swc_reader.getPointVector();
        std::map<int, int> vertex_hash;

        std::cout << "swc file " + swc_file + " has been loaded." << std::endl;
        std::cout << point_vector.size() << std::endl;

        Vec3D<double> start_point{};
        Vec3D<int> dimension{};
        boundingBox(point_vector, x_space, y_space, z_space, start_point, dimension);
        BVH *bvh = new BVH(start_point.to_i32vec3(),
                           start_point.to_i32vec3() + dimension.to_i32vec3(),
                           glm::dvec3(blocksize, blocksize, blocksize));

        auto paths = searchPath(point_vector, vertex_hash);
        std::cout << paths.size() << std::endl;

//        //调整每条路径上半径过小的点的半径
//        for (auto& path : paths)
//        {
//            for (int i = 0; i < path.path.size(); i++)
//            {
//                if(path.path[i].radius<0.01)//对每条路径上半径小于0.01的点，根据路径前后半径>0.01的第一个点的半径调整自身的半径
//                {
//                    float sum = 0 ;
//                    int begin = i ;
//                    int count = 0 ;
//
//                    while(count<1)//考虑往前第一个半径>0.01的点（包括自身），sum += 半径，如果没有则sum += 0.1
//                    {
//                        if(path.path[begin].radius>0.01 ){
//                            sum += path.path[begin].radius ;
//                            count++ ;
//                        }
//                        begin-- ;
//                        if(begin<0)
//                        {
//                            if(count<1)
//                            {
//                                sum += 0.1 ;
//                                count++ ;
//                            }
//                            break ;
//                        }
//                    }
//                    begin = i ;
//                    while(count<2)//此时count==1，考虑往后往前第一个半径>0.01的点（包括自身），sum += 半径，如果没有则sum += 0.1
//                    {
//                        if(path.path[begin].radius>0.01 ){
//                            sum += path.path[begin].radius ;
//                            count++ ;
//                        }
//
//                        begin-- ;
//                        if(begin>=path.path.size())
//                        {
//                            if(count<2)
//                            {
//                                sum += 0.1 ;
//                                count++ ;
//                            }
//                            break ;
//                        }
//                    }
//
//                    //新半径为两者平均
//                    path.path[i].radius = sum / count ;
//                    std::cout<<point_vector[vertex_hash[path.path[i].current_id]].radius<<" change to " <<path.path[i].radius<<std::endl ;
//                    point_vector[vertex_hash[path.path[i].current_id]].radius = path.path[i].radius ;
//
//                }
//            }
//        }

        //?
//        for(int i = 0 ; i < point_vector.size(); i++)
//        {
//            if(point_vector[i].radius<=0.0)
//            {
//                std::cout<<"shit happened"<<std::endl ;
//            }
//        }

//        int maxid = getDetph(paths,point_vector,vertex_hash) ;

//	start_point *= space;

        std::cout << "Start Points : " << start_point << std::endl;
        std::cout << "Dimension: " << dimension << std::endl;

        double step = 0.0002;
        int window_offset = 0;

#pragma omp parallel for num_threads(24)
        for (int j = 0; j < paths.size(); j++) {
            const auto path = paths[j];
            for (int i = 1; i < path.path.size(); i++) {
                Vec3D<double> start_point = {path.path[i - 1].x, path.path[i - 1].y, path.path[i - 1].z};
                Vec3D<double> end_point = {path.path[i].x, path.path[i].y, path.path[i].z};
                auto direction = (end_point - start_point).normalized();
                auto vec_length = (end_point - start_point).length();
                std::vector<Vec3D<double>> points;
                std::vector<double> radius;
                std::vector<Sphere *> spheres;
                double r = path.path[i].radius - path.path[i - 1].radius;

                //在每两个点中间插值加入一系列点，位置和半径都是线性插值
                Vec3D<double> startP = (start_point);
                spheres.push_back(getSphere(startP, path.path[i - 1].radius));
                for (int t = 1; t < vec_length / step; t++) {
                    auto cur_point = direction * (t * step) + start_point;
                    Vec3D<double> startP = (cur_point);
                    spheres.push_back(getSphere(startP, r * (t * step) / vec_length + path.path[i - 1].radius));
                }
                startP = end_point;
                spheres.push_back(getSphere(startP, path.path[i].radius));

                std::vector<BVHNode *> nodes;
                for (auto p = 0; p < spheres.size(); p++) {
                    nodes.clear();
                    bvh->getInteract(nodes, *spheres[p]);

                    for (auto begin = nodes.begin(); begin != nodes.end(); begin++) {
                        BVHNode *node = *begin;
#pragma omp critical
                        {
                            node->usingNode();
                            node->setData(spheres[p]);//将所有在球内的点标为1
                        };

                    }

                }

                std::cout << "Process : " << j << " in " << paths.size() << ", " << i << " in " << paths[j].path.size()
                          << std::endl;
            }
        }
        averageAll(bvh->getRoot());
        averageAll(bvh->getRoot());//平滑（获取体数据）
        changeAll(bvh->getRoot());//除去padding
        bvh->check();//保证所有相邻体素块相邻面体素值相同，取两者较大值
//    std::string path = std::string(CONDIF_DIR)+"Result/" ;
        writeAll(bvh->getRoot(), volume_dir);

        delete bvh;
        return 0;
    }

    void GetNeuronInfo(std::string swc_file, int blocksize, NeuronInfo *neuronInfo) {
        double x_space = 1;
        double y_space = 1;
        double z_space = 1;

        SWCReader swc_reader;
        swc_reader.readSWC(swc_file);

        auto point_vector = swc_reader.getPointVector();
        std::map<int, int> vertex_hash;

        std::cout << "swc file " + swc_file + " has been loaded." << std::endl;
        std::cout << point_vector.size() << std::endl;

        Vec3D<double> start_point{};
        Vec3D<int> dimension{};
        boundingBox(point_vector, x_space, y_space, z_space, start_point, dimension);
        BVH *bvh = new BVH(start_point.to_i32vec3(),
                           start_point.to_i32vec3() + dimension.to_i32vec3(),
                           glm::dvec3(blocksize, blocksize, blocksize));

        auto paths = searchPath(point_vector, vertex_hash);
        std::cout << paths.size() << std::endl;

        std::cout << "Start Points : " << start_point << std::endl;
        std::cout << "Dimension: " << dimension << std::endl;

        neuronInfo->mainPaths = mainPathsIndex;
        neuronInfo->paths = paths;
        neuronInfo->vertex_hash = vertex_hash;
        neuronInfo->point_vector = point_vector;
        neuronInfo->bvh = bvh;
        neuronInfo->blocksize = blocksize;
        neuronInfo->x_dimension = dimension.x;
        neuronInfo->y_dimension = dimension.y;
        neuronInfo->z_dimension = dimension.z;
        neuronInfo->x_space = x_space;
        neuronInfo->y_space = y_space;
        neuronInfo->z_space = z_space;
        neuronInfo->x_start = start_point.x;
        neuronInfo->y_start = start_point.y;
        neuronInfo->z_start = start_point.z;
        neuronInfo->swcname = swc_file;
        neuronInfo->vertexCount = point_vector.size();
        neuronInfo->deleteList.clear();
        neuronInfo->addList.clear();
        neuronInfo->connectionList.clear();
    }

    static bool isInteracted(Sphere* ss,glm::dvec3 start_point,glm::dvec3 end_point){
        auto squared = [](float v){
            return v*v;
        };

        glm::dvec3 C1 = start_point;
        glm::dvec3 C2 = end_point;
        glm::dvec3 S = ss->position ;
        float R = ss->radius ;
        float dist_squared = R * R;

        if (S.x < C1.x) dist_squared -= squared(S.x - C1.x);
        else if (S.x > C2.x) dist_squared -= squared(S.x - C2.x);
        if (S.y < C1.y) dist_squared -= squared(S.y - C1.y);
        else if (S.y > C2.y) dist_squared -= squared(S.y - C2.y);
        if (S.z < C1.z) dist_squared -= squared(S.z - C1.z);
        else if (S.z > C2.z) dist_squared -= squared(S.z - C2.z);
        return dist_squared > 0;
    }

    static float calOverOne(float amax, float amin, float bmax, float bmin){
        float minMax = 0;
        float maxMin = 0;
        if (amax < bmax)//a物体在b物体左侧
        {
            minMax = amax;
            maxMin = bmin;
        }
        else //a物体在b物体右侧
        {
            minMax = bmax;
            maxMin = amin;
        }

        if (minMax > maxMin) {
            return minMax - maxMin;
        }
        else {
            return 0;
        }
    };

    static std::vector<float> calOverTotal(Vec3D<double> start_point0, Vec3D<double> end_point0,Vec3D<double> start_point1, Vec3D<double> end_point1){
        float xOver = calOverOne(end_point0.x, start_point0.x, end_point1.x, start_point1.x);
        float yOver = calOverOne(end_point0.y, start_point0.y, end_point1.y, start_point1.y);
        float zOver = calOverOne(end_point0.z, start_point0.z, end_point1.z, start_point1.z);
        std::vector<float> array = {xOver, yOver, zOver};
        return array;
    };

    static bool isInteracted(Vec3D<double> s1, double r1,Vec3D<double> s2, double r2,Vec3D<double> start_point,Vec3D<double> end_point){
        Vec3D<double> start_point0{}, end_point0{};
        start_point0.x = std::min(s1.x - r1 , s2.x - r2);
        start_point0.y = std::min(s1.y - r1 , s2.y - r2);
        start_point0.z = std::min(s1.z - r1 , s2.z - r2);

        end_point0.x = std::max(s1.x + r1 , s2.x + r2);
        end_point0.y = std::max(s1.y + r1 , s2.y + r2);
        end_point0.z = std::max(s1.z + r1 , s2.z + r2);

        auto array = calOverTotal(start_point0,end_point0,start_point,end_point);
        if(array[0] > 0 && array[1] > 0 && array[2] > 0 )
            return true;
        else
            return false;
    }

    void GetVolume(Vec3D<double> in_start_point,Vec3D<double> in_end_point,std::vector<Path> paths,int blocksize,std::string voldir){
        glm::dvec3 dstart_point(in_start_point.x,in_start_point.y,in_start_point.z);
        glm::dvec3 dend_point(in_end_point.x,in_end_point.y,in_end_point.z);

        std::cout << "new paths size :" << paths.size() << std::endl;
        //构建局部体素块
        BVH *bvh = new BVH(in_start_point.to_i32vec3(),
                           in_end_point.to_i32vec3(),
                              glm::dvec3(blocksize, blocksize, blocksize));

        double step = 0.0002;
        int window_offset = 0;

#pragma omp parallel for num_threads(24)
        for (int j = 0; j < paths.size(); j++) {

            const auto path = paths[j];
            for (int i = 1; i < path.path.size(); i++) {
                Vec3D<double> start_point = {path.path[i - 1].x, path.path[i - 1].y, path.path[i - 1].z};
                Vec3D<double> end_point = {path.path[i].x, path.path[i].y, path.path[i].z};
                if(!isInteracted(start_point,path.path[i - 1].radius,end_point,path.path[i].radius,in_start_point,in_end_point)){
                    std::cout << "skip" << std::endl;
                    continue;
                }

                auto direction = (end_point - start_point).normalized();
                auto vec_length = (end_point - start_point).length();
                std::vector<Vec3D<double>> points;
                std::vector<double> radius;
                std::vector<Sphere *> spheres;
                double r = path.path[i].radius - path.path[i - 1].radius;

                //在每两个点中间插值加入一系列点，位置和半径都是线性插值
                Vec3D<double> startP = (start_point);
                auto s = getSphere(startP, path.path[i - 1].radius);
                if(isInteracted(s,dstart_point,dend_point))
                    spheres.push_back(s);
                for (int t = 1; t < vec_length / step; t++) {
                    auto cur_point = direction * (t * step) + start_point;
                    Vec3D<double> startP = (cur_point);
                    s = getSphere(startP, r * (t * step) / vec_length + path.path[i - 1].radius);
                    if(isInteracted(s,dstart_point,dend_point))
                        spheres.push_back(s);
                }
                startP = end_point;
                s = getSphere(startP, path.path[i].radius);
                if(isInteracted(s,dstart_point,dend_point))
                    spheres.push_back(s);

                std::vector<BVHNode *> nodes;
                for (auto p = 0; p < spheres.size(); p++) {
                    nodes.clear();
                    bvh->getInteract(nodes, *spheres[p]);

                    for (auto begin = nodes.begin(); begin != nodes.end(); begin++) {
                        BVHNode *node = *begin;
#pragma omp critical
                        {
                            node->usingNode();
                            node->setData(spheres[p]);//将所有在球内的点标为1
                        };

                    }

                }

                std::cout << "Process : " << j << " in " << paths.size() << ", " << i << " in " << paths[j].path.size()
                          << std::endl;
            }
        }
        averageAll(bvh->getRoot());
        averageAll(bvh->getRoot());//平滑（获取体数据）
        changeAll(bvh->getRoot());//除去padding
        bvh->check();//保证所有相邻体素块相邻面重叠体素值相同，取两者较大值

        auto nodes = bvh->getNodes();
        for(auto item : *nodes){
            auto start = item->box->getStart();
            std::cout << item->id << " " << start.x << " " << start.y << " " << start.z << std::endl;
        }

        writeAll(bvh->getRoot(), voldir);
    }

    int filecount = 0;
    std::vector<int> mainPathsIndex;
};