//
// Created by 李源 on 2020-11-23.
//

#ifndef SWC2OBJ_BVH_H
#define SWC2OBJ_BVH_H

#include <glm/glm.hpp>
#include <vector>
#include <stack>
#include <set>
#include <assert.h>

#include "based.h"

struct BVHNode{
public:
    int id ;
    VoxelBox* box= nullptr;
    BVHNode* children[2] = {nullptr, nullptr};



    BVHNode(int nid,glm::dvec3 pmin,glm::dvec3 pmax){
        box = new VoxelBox(pmin,pmax) ;
        id = nid ;
    } ;
    BVHNode(int nid, VoxelBox* box_):id(nid),box(box_){}
    BVHNode(int nid,BVHNode* childrenL, BVHNode* childrenR) ;

    void usingNode() {
        assert(box!= nullptr) ;
        box->usingBox() ;
    }

    void setNewBox(VoxelBox* newbox)
    {
        if(box != nullptr)
        {
            box->free() ;
            delete box ;
            box = newbox ;
        }else{
            std::cout<<" error happend in set new box"<<std::endl ;
        }
    }

    glm::dvec3 getStart() { return box->pmin ; }
    void setInteractData(BVHNode* ano) ;

    void setBoxData(glm::dvec3 Position, float value) {
        box->setPositionData(Position,value) ;
    }

    float getBoxData(glm::dvec3 Position)
    {
        return box->getPositionData(Position) ;
    }

    bool isUsingNode() {
        assert(box!= nullptr) ;
        return box->data!= nullptr ;
    }
    std::vector<float>* getData() { return box->getBoxData() ; }
    void setData(Sphere* ss) ;
    glm::dvec3 getDimension() { return box->getDimension() ;}

    ~BVHNode(){
        box->free() ;
        if(children[0]!= nullptr) delete children[0] ;
        if(children[1]!= nullptr) delete children[1] ;
    }



};


class BVH {
private:
    BVHNode* root = nullptr ;
    std::vector<BVHNode*> nodes ;
public:
    BVH(glm::i32vec3 pmin, glm::i32vec3 pmax ,glm::dvec3 voxelBox) ;
    void getInteract(std::vector<BVHNode*>& nodes, Sphere& ss ) ;
    BVHNode* getRoot() {return root ;}
    std::vector<BVHNode*>* getNodes() { return &nodes ;} ;
    ~BVH() {
        delete root ;
    }

    void check()
    {
        for(int i = 0 ; i < nodes.size(); i++)
        {
            for(int j = i+1 ; j<nodes.size();j++)
            {
                if(nodes[i]->isUsingNode()&& nodes[j]->isUsingNode())
                {
                    nodes[i]->setInteractData(nodes[j]) ;
                    std::cout<<i<<" "<<j<<std::endl ;
                }
            }
        }
    }
};


#endif //SWC2OBJ_BVH_H
