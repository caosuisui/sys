//
// Created by 李源 on 2020-11-23.
//

#include "BVH.h"
#include "Geo_Util.h"


std::vector<int> argsort(const glm::dvec3 array)
{
    const int array_len(3);
    std::vector<int> array_index(array_len, 0);
    for (int i = 0; i < array_len; ++i)
        array_index[i] = i;

    std::sort(array_index.begin(), array_index.end(),
              [&array](int pos1, int pos2) {return (array[pos1] < array[pos2]);});

    return array_index;
}



BVHNode::BVHNode(int nid,BVHNode* childrenL, BVHNode* childrenR)
{
    this->id = nid ;
    this->children[0] = childrenL ;
    this->children[1] = childrenR ;
    this->box = new VoxelBox() ;
    this->box->merge(*(childrenL->box) ) ;
    this->box->merge(*(childrenR->box) ) ;
}


void BVHNode::setData(Sphere* ss)
{
    VoxelBox* ss_box = ss->getBoundBox() ;
    VoxelBox* t = ss_box->interact(this->box) ;
    int count = 0 ;
    if(t != nullptr)
    {
        glm::i32vec3 boxdimension = this->getDimension() ;
//        std::cout<<" block dimension is "<<glm::to_string(boxdimension) <<std::endl ;
        glm::i32vec3 dimension = (t->pmax - t->pmin + glm::dvec3(0.5,0.5,0.5)) ;
//        std::cout<<" dimension is "<<glm::to_string(dimension) <<std::endl ;
        std::vector<float>* data = this->box->getBoxData() ;

        glm::dvec3 center = ss->position ;
        glm::i32vec3 t1 = glm::round(center - box->pmin);
        if(t1[0]>=0 && t1[1]>=0&&t1[2]>=0&& t1[0]<boxdimension[0] && t1[1]<boxdimension[1] && t1[2]<boxdimension[2])
        {
            (*data)[t1[2]*boxdimension[1]*boxdimension[0]+t1[1]*boxdimension[0]+t1[0]] = 1 ;
            count++ ;
        }//中心轴标记为1

        //所有在球内的点标为1
        for(int i = 0 ; i< dimension[2]; i++)
        {
            for(int j = 0; j < dimension[1]; j++)
            {
                for(int z = 0; z< dimension[0]; z++)
                {
                    if(ss->isInBox(glm::dvec3(z,j,i)+t->pmin))
                    {
                        count++ ;
                        glm::i32vec3 t1 = glm::dvec3(z,j,i)+ t->pmin - box->pmin;
                        (*data)[t1[2]*boxdimension[1]*boxdimension[0]+t1[1]*boxdimension[0]+t1[0]] = 1 ;
                    }
                }
            }
        }
        delete t ;
    }
//    std::cout<<"there is not zero "<<count<<" "<<ss->radius<<std::endl ;

}


//递归获取二叉树
BVHNode* splitBuild(std::vector<BVHNode*> nodes,int begin, int end)
{
    if(begin == end) {
        std::cout<<"begin shit"<<begin<<std::endl ;
        return nullptr;
    }

    VoxelBox box ;
    for(int i = begin ;i<end;i++)
    {
        box.merge(*(nodes[i]->box)) ;//合并所有经过的box
    }

    int size = end-begin ;
    if(size==1)
    {
        return nodes[begin] ;
    } else {
        std::vector<int> dimension = argsort(box.pmax-box.pmin) ;
        int dimension1 = dimension[2] ;//找到总box的最大边长

        std::sort(&nodes[begin],&nodes[end-1]+1,[dimension1](BVHNode*& node1, BVHNode*& node2){
            return node1->box->pmin[dimension1] < node2->box->pmin[dimension1]  ;
        }) ;//在最大边长上对所有box排序

        BVHNode* t = nodes[int((begin+end)/2)]  ;
        double split = t->box->pmin[dimension1] ;//找到中间box在该边上的坐标

        auto middle = std::partition(&nodes[begin],&nodes[end-1]+1,[split,dimension1](const BVHNode* node){
            return node->box->pmin[dimension1] < float(split) ;
        }) ;

        int mid = begin ;
        for(auto t = nodes[mid]; t->id!=(*middle)->id;t= nodes[++mid]) {};
//        std::cout<<"split mid is "<<mid <<"between "<<begin <<"and "<<end<<std::endl ;
//        std::cout<<"split mid is "<<mid <<"between "<<begin <<"and "<<end<<std::endl ;
        BVHNode* parent = new BVHNode(-1,
                                         splitBuild(nodes,begin,mid) ,
                                         splitBuild(nodes,mid,end)
        ) ;


        return parent ;
    }

}

//获取总box数
int getNumber(BVHNode* root)
{
    if(root== nullptr) return 0 ;
    if(root->id>=0) return 1 ;
    return getNumber(root->children[0]) +getNumber(root->children[1]) ;
}

BVH::BVH(glm::i32vec3 pmin, glm::i32vec3 pmax ,glm::dvec3 voxelBox)
{
    int padding = 4 ;
    voxelBox += glm::dvec3(4,4,4) ;
    glm::i32vec3 size = pmax - pmin ;//原始体素大小
    int next = padding+2 ;

    //由于每两个相邻box之间在没有padding的情况下也有2个单位的重合，为了独立表示重合的每个部分，需要对体素的存储空间进行扩张，保证存储空间的size是blocksize的整数倍
    if(int(size[0])%int(voxelBox[0]-next)!=0) size[0] = (int((size[0])/(voxelBox[0]-next))+1)*(voxelBox[0]-padding) ;
    if(int(size[1])%int(voxelBox[1]-next)!=0) size[1] = (int((size[1])/(voxelBox[1]-next))+1)*(voxelBox[1]-padding) ;
    if(int(size[2])%int(voxelBox[2]-next)!=0) size[2] = (int((size[2])/(voxelBox[2]-next))+1)*(voxelBox[2]-padding) ;//由于每两个相邻box之间在没有padding的保证size是blocksize的整数倍



    glm::dvec3 startPoint = glm::i32vec3(-padding/2,-padding/2,-padding/2) + pmin;
//    glm::dvec3 endPoint = glm::dvec3(size[0]-1,size[1]-1,size[2]-1) ;
    int countcode = 0 ;
//获取所有box
        for(int i = 0 ; i*(voxelBox[0]-next)<size[0] ;i++ )
        {
            for(int j = 0 ;j*(voxelBox[1]-next)<size[1];j++)
            {
                for(int k = 0; k*(voxelBox[2]-next)<size[2];k++)
                {

                    glm::dvec3 s = startPoint + glm::dvec3(i* (voxelBox[0]-next),
                                                         j* (voxelBox[1]-next),
                                                         k* (voxelBox[2]-next));//相邻box差blocksize-2

                    VoxelBox* t = new VoxelBox(s,s+voxelBox) ;//box大小为blocksize+padding
                    BVHNode* n = new BVHNode(nodes.size(),t) ;
//                    n->usingNode() ;
                    nodes.push_back(n) ;
//                    std::cout<<"box is "<<glm::to_string(s)<<" "<<glm::to_string(s+voxelBox)<<" "<<countcode++<<std::endl ;
                }
            }
    }//每两个blocksize（即box去掉padding的情况下）之间有2个单位的重合，另外blocksize之外还有padding
    this->root = splitBuild(nodes, 0, nodes.size()) ;//获取所有box的二叉树
    std::cout<<"build end"<<std::endl ;
    std::cout<<"there is all "<<getNumber(this->root)<<std::endl ;

}


void BVHNode::setInteractData(BVHNode* ano)
{

    glm::dvec3 res = ano->box->pmin - this->box->pmin ;
    float res1 = abs(glm::length(res)) ;
    if(res1!=(ano->getDimension()[0]-2))//确保相邻
    {
//        std::cout<<res1<<" is "<<ano->getDimension()[0]<<std::endl ;
        return ;
    }

    //保证两个轴距离小于等于0.1，一个大于0.1，即在一个轴上相邻
    std::cout<<"come in"<<std::endl ;
    std::vector<float> dim ;
    if(abs(res[0]) <= 0.1) dim.push_back(0) ;
    if(abs(res[1]) <= 0.1) dim.push_back(1) ;
    if(abs(res[2]) <= 0.1) dim.push_back(2) ;

    assert(dim.size()==2) ;

    if(abs(res[0]) > 0.1) dim.push_back(0) ;
    if(abs(res[1]) > 0.1) dim.push_back(1) ;
    if(abs(res[2]) > 0.1) dim.push_back(2) ;
    assert(dim.size()==3) ;//在该轴上相邻

    float fixed  ;
    float fixed1 ;


    //ano坐标比this小，则两者相交的面为this坐标（坐标指轴上的pmin）
    if(res[0]+res[1]+res[2]<0)
    {
        fixed = this->box->pmin[dim[2]] ;
        fixed1 = fixed+1 ;
//        fixed1_ano  = fixed-1 ;
    }else{
        fixed = ano->box->pmin[dim[2]] ;
        fixed1 = fixed + 1 ;
//        fixed1_this = fixed-1 ;
    }

    //对当前box的另外两个轴遍历，交换该面的两个面的值
    for(int i = this->box->pmin[dim[1]] ; i < this->box->pmax[dim[1]] ; i++)
    {
        for(int j = this->box->pmin[dim[0]] ; j < this->box->pmax[dim[0]] ; j++)
        {
            glm::dvec3 pos ;
            pos[dim[1]] = i ;
            pos[dim[0]] = j ;
            pos[dim[2]] = fixed ;
            float m = this->getBoxData(pos) ;
            float n = std::max(m,ano->getBoxData(pos)) ;

            this->setBoxData(pos,n) ;
            ano->setBoxData(pos,n) ;//取两者中的较大值
        }
    }

    for(int i = this->box->pmin[dim[1]] ; i < this->box->pmax[dim[1]] ; i++)
    {
        for(int j = this->box->pmin[dim[0]] ; j < this->box->pmax[dim[0]] ; j++)
        {
            glm::dvec3 pos ;
            pos[dim[1]] = i ;
            pos[dim[0]] = j ;
            pos[dim[2]] = fixed1 ;

            float m = this->getBoxData(pos) ;
            float n = std::max(m,ano->getBoxData(pos)) ;
//            std::cout<<glm::to_string(pos)<<" for "<<glm::to_string(ano->box->pmin) <<std::endl ;
//            std::cout<<glm::to_string(pos1)<<" for "<<glm::to_string(this->box->pmin) <<std::endl ;

            this->setBoxData(pos,n) ;
            ano->setBoxData(pos,n) ;
        }
    }


    std::cout<<"merge "<<glm::to_string(this->box->pmin)<<" "<<glm::to_string(ano->box->pmin)<<" in "<<dim[2]<<std::endl ;//差254
}

//递归获取所有与该球形相交的box
void BVH::getInteract(std::vector<BVHNode*>& othernodes, Sphere& ss )
{
    std::stack<BVHNode*> st ;
    st.push(root) ;


    while(st.size()>0)
    {
        BVHNode* top = st.top() ;
        st.pop() ;

        if(doesBoxIntersectSphere(*(top->box),ss))
        {
            if(top->id>=0) othernodes.push_back(top) ;
            else{
                if(doesBoxIntersectSphere(*(top->children[0]->box),ss)) st.push(top->children[0]) ;
                if(doesBoxIntersectSphere(*(top->children[1]->box),ss)) st.push(top->children[1]) ;
            }

        }
    }




}
