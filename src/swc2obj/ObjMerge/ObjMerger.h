//
// Created by lidan on 4/12/2020.
//

#ifndef SWC2OBJ_OBJMERGER_H
#define SWC2OBJ_OBJMERGER_H

#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <sstream>


struct Point{
    int id ;
    glm::dvec3 poi ;

    Point(int id, glm::dvec3 p)
    {
        poi = p ;
        this->id = id ;
    }

    bool operator==(const struct  Point& ano) const
    {
//        if(poi[0] == ano.poi[0] && poi[1] == ano.poi[1] && poi[2] == ano.poi[2])
//        {
//            return true ;
//        }
        double value = 1 ;
//        if(abs(poi[0]-ano.poi[0]) < value && abs(poi[1]-ano.poi[1]) < value && abs(poi[2]-ano.poi[2])<value)
        if(poi[0] == ano.poi[0] && poi[1] == ano.poi[1] && poi[2] == ano.poi[2])
        {
            return true ;
        }
        return false ;

    }

    bool operator<(const struct Point & right) const
    {
        double value = 1 ;
//       if(poi[0]-right.poi[0]<value) return true ;
//       else if (poi[0] - right.poi[0]>value) return false ;
//       else
//       {
//           if(poi[1]-right.poi[1]<value) return true ;
//           else if (poi[1] - right.poi[1]>value) return false ;
//           else
//           {
//               if(poi[2]-right.poi[2]<value) return true ;
//               else if (poi[2] - right.poi[2]>value) return false ;
//               else
//               {
//                   return false ;
//               }
//           }
//       }

        if(poi[0]<right.poi[0]) return true ;
        else if (poi[0] > right.poi[0]) return false ;
        else if(poi[0] == right.poi[0])
        {
            if(poi[1]<right.poi[1]) return true ;
            else if (poi[1] > right.poi[1]) return false ;
            else if(poi[1] == right.poi[1])
            {
                if(poi[2]<right.poi[2]) return true ;
                else if (poi[2] > right.poi[2]) return false ;
                else if(poi[2] == right.poi[2])
                {
                    return false ;
                }
            }
        }

    }


    bool operator>(const struct Point & right) const
    {
        return !(*this<right || this->poi==right.poi) ;
    }


};

struct Trias {
    int id ;
    glm::i32vec3 idxs ;

    Trias(int id, glm::i32vec3 p)
    {
        idxs = p ;
        this->id = id ;
    }


};
class ObjMerger {
private:
    std::vector<Point> points ;
    std::vector<Point> vns ;
    std::vector<Trias> trias ;
    std::map<double,std::set<Point>> forrepeat ;
    std::map<int,int> ids ;
public:
    ObjMerger(std::string filename) ;
    void writeNew(std::string filename) ;
    void writeNew1(std::string filename) ;
    void writeOld(std::string filename) ;
    void removerepeat()
    {
        for(int i =0 ; i < points.size();i++)
        {
            if(forrepeat.count(points[i].poi[0])==0)
            {
                forrepeat[points[i].poi[0]] = std::set<Point>() ;
                forrepeat[points[i].poi[0]].insert(points[i]) ;
            }else{
                auto t = forrepeat[points[i].poi[0]].find(points[i]) ;
                if(t == forrepeat[points[i].poi[0]].end())
                    forrepeat[points[i].poi[0]].insert(points[i]) ;
                else{
                    ids[points[i].id] = t->id ;
//                    std::cout<<"map "<< points[i].id <<" "<<t->id <<std::endl;
//                    std::cout<<points[i].id << glm::to_string(points[i].poi)<<std::endl ;
//                    std::cout<<t->id << glm::to_string(t->poi)<<std::endl ;
                }
            }
        }

    }

};


#endif //SWC2OBJ_OBJMERGER_H
