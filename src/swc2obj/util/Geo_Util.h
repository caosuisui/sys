//
// Created by 李源 on 2020-11-23.
//

#ifndef SWC2OBJ_GEO_UTIL_H
#define SWC2OBJ_GEO_UTIL_H
#include <algorithm>
#include "based.h"
#include <glm/gtx/string_cast.hpp>

inline float squared(float v) { return v * v; }
inline bool doesBoxIntersectSphere(VoxelBox box,Sphere ss)
{
    glm::dvec3 C1 = box.pmin ;
    glm::dvec3 C2 = box.pmax ;
    glm::dvec3 S = ss.position ;
    float R = ss.radius ;
    float dist_squared = R * R;

    if (S.x < C1.x) dist_squared -= squared(S.x - C1.x);
    else if (S.x > C2.x) dist_squared -= squared(S.x - C2.x);
    if (S.y < C1.y) dist_squared -= squared(S.y - C1.y);
    else if (S.y > C2.y) dist_squared -= squared(S.y - C2.y);
    if (S.z < C1.z) dist_squared -= squared(S.z - C1.z);
    else if (S.z > C2.z) dist_squared -= squared(S.z - C2.z);
    return dist_squared > 0;
}


#endif //SWC2OBJ_GEO_UTIL_H
