#pragma once
#include <iostream>
#include <vector>
#include <math.h>
#include <glm/glm.hpp>

#include <limits.h>
#include <glm/gtx/string_cast.hpp>

class Vertex
{
public:
	int					current_id;
	int					point_type;
	double				x, y, z;
	double				radius;
	int					previous_id;
	int					index;
	int					degree = 0;
	bool				is_visited = false;
	int                 depth = -1;
	double              distance;
	double              sx,sy;
};

class Path
{
public:
	int					path_type;//即id
	std::vector<Vertex> path;
	bool 				use = true;
	std::vector<int>    sub_paths_index;
	int                 branch_point_index;//从分支的分叉点id（其在父路径中的id，不是pointvector中的）
	int                 parent_path;
	double              length;
};

template<typename T>
class Vec3D
{
public:
	T x, y, z;

	friend std::ostream& operator << (std::ostream& output, const Vec3D<T>& i)
	{
		output << "[" << i.x <<", "<< i.y << ", " << i.z << "]" << std::endl;
		return output;
	}
	Vec3D<T>& operator /= (Vec3D<T>& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}
	Vec3D<T>& operator *= (Vec3D<T>& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		return *this;
	}
	Vec3D<T> operator-(Vec3D<T>& b)
	{
		Vec3D<T> c = { static_cast<T>(x - b.x), static_cast<T>(y - b.y), static_cast<T>(z - b.z) };
		return c;
	}
	Vec3D<T> normalized()
	{
		auto d = sqrt((x* x + y * y + z * z)*1.0);
		Vec3D<T> c = {static_cast<T>(x / d), static_cast<T>(y / d), static_cast<T>(z / d)};
		return c;
	}
	double length()
	{
		return sqrt((x * x + y * y + z * z) * 1.0);
	}
	Vec3D<T> operator*(double a)
	{
		Vec3D<T> c = { static_cast<T>(x * a), static_cast<T>(y * a), static_cast<T>(z * a) };
		return c;
	}
	Vec3D<T> operator+(Vec3D<T>& a)
	{
		Vec3D<T> c = { static_cast<T>(x + a.x), static_cast<T>(y + a.y), static_cast<T>(z + a.z) };
		return c;
	}
	Vec3D<T> operator+(T a)
	{
		Vec3D<T> c = { static_cast<T>(x + a), static_cast<T>(y + a), static_cast<T>(z + a) };
		return c;
	}
	Vec3D<T> operator-(T a)
	{
		Vec3D<T> c = { static_cast<T>(x - a), static_cast<T>(y - a), static_cast<T>(z - a) };
		return c;
	}
	Vec3D<T>& operator-=(Vec3D<T>& a)
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
		return *this;
	}

	glm::i32vec3 to_i32vec3()
    {
	    return glm::i32vec3(int(x),int(y),int(z)) ;
    }

    glm::dvec3 to_vec3(){
	    return glm::dvec3(x,y,z) ;
	}
};


/*
    Here is the cone in cone space:

            +        ^
           /|\       |
          /*| \      | H
         /  |  \     |
        /       \    |
       +---------+   v

    * = alpha (angle from edge to axis)
*/





class VoxelBox{
public:
    glm::dvec3 pmin = glm::dvec3(std::numeric_limits<float>::max(),
                               std::numeric_limits<float>::max(),
                               std::numeric_limits<float>::max());
    glm::dvec3 pmax = glm::dvec3(std::numeric_limits<float>::min(),
                               std::numeric_limits<float>::min(),
                               std::numeric_limits<float>::min());;


    std::vector<float>* data = nullptr ;

    VoxelBox(){}
    VoxelBox(glm::dvec3 pmin_, glm::dvec3 pmax_):pmin(pmin_),pmax(pmax_){};
    VoxelBox(glm::i32vec3 pmin_, glm::i32vec3 pmax_):pmin(pmin_),pmax(pmax_){}

    glm::dvec3 getCenter() { return (pmin+pmax)*double(0.5) ; }

    void setPositionData(glm::dvec3 pos, float v) {
    	glm::dvec3 r = pos - pmin ;
    	glm::i32vec3 dim = pmax - pmin ;
		(*data)[int(r[2]*dim[1]*dim[0]+r[1]*dim[0]+r[0])] = v ;
    }

    float getPositionData(glm::dvec3 pos)
	{
		glm::dvec3 r = pos - pmin ;
		glm::i32vec3 dim = pmax - pmin ;
    	return (*data)[int(r[2]*dim[1]*dim[0]+r[1]*dim[0]+r[0])] ;
	}
    void merge(VoxelBox& box) {
//        std::cout<<"merge "<<glm::to_string(pmin) <<" "<<glm::to_string(box.pmin)<<std::endl ;
        pmin = glm::min(pmin,box.pmin) ;
        pmax = glm::max(pmax,box.pmax) ;
//        std::cout<<"merge "<<glm::to_string(pmin) <<" "<<glm::to_string(box.pmin)<<std::endl ;

    }
    bool isInteractWith(Vertex* first, Vertex* second) = delete;

    VoxelBox* interact(VoxelBox* ano)
    {
        glm::dvec3 mmin = glm::max(pmin, ano->pmin) ;
        glm::dvec3 mmax = glm::min(pmax, ano->pmax) ;

        if(mmin[0]<=mmax[0] && mmin[1]<=mmax[1] && mmin[2]<=mmax[2]) {
            return new VoxelBox(mmin,mmax) ;
        }
        return nullptr ;
    }


    void usingBox() {
        if(data == nullptr){
            glm::dvec3 size = pmax - pmin ;
            int boxsize = int(size[0])*int(size[1])*int(size[2]) ;
            data = new std::vector<float>(boxsize,0.0) ;
//            std::cout<<glm::to_string(pmax)<<" "<<glm::to_string(pmin)<<" "<<glm::to_string(size)<<" "<<boxsize<<" "<<1028*1028*1028<<std::endl ;
//            std::cout<<"render data for "<<boxsize<<std::endl ;
        }
    }
    std::vector<float>* getBoxData(){
        return data ;
    }

    glm::dvec3 getStart() { return pmin; }

    glm::dvec3 getDimension() { return pmax-pmin; }

    void free(){
        if(data!= nullptr) data->clear() ;
    }

};


class Cone // In cone space (important)
{
public:
    double H ;
    double radius ;
    glm::dvec3 dir ;
    glm::dvec3 position ;
};

class Plane
{
public:
    double u;
    double v;
    glm::dvec3 u_dir ;
    glm::dvec3 v_dir ;
    glm::dvec3 start;
};


class Sphere{
public:
    double radius ;
    glm::dvec3 position ;
    VoxelBox* box = nullptr ;
    Sphere(double radius_, glm::dvec3 position_):radius(radius_),position(position_) {
        box = new VoxelBox(position - glm::dvec3(radius,radius,radius),
                           position + glm::dvec3(radius,radius,radius)) ;

    } ;
    void setBoundBox(VoxelBox* ano) { this->box = ano ;}
    VoxelBox* getBoundBox() { return this->box ;}

    bool isInBox(glm::dvec3 node)
    {
        if(glm::length(node-position)<radius) {return true;} return false ;
    }
};


//inline bool intersect(Cone cone, glm::dvec3 dir, glm::dvec3 P)
//{
//    // Beware, indigest formulaes !
//    double sqTA = pow(cone.radius/cone.H,2) ;
//
//    double A = pow(dir[0],2) + pow(dir[1],2) - pow(dir[2],2) * sqTA;
//    double B = 2 * P[0]*dir[0] + 2 * P[1] * dir[1] - 2 * (cone.H - P[2]) * dir[2] * sqTA ;
//    double C = P[0] * P[0] + P[1] * P[1] - (cone.H - P[2]) * (cone.H - P[2]) * sqTA ;
//
//    double delta = B * B - 4 * A * C;
//    if(delta < 0)
//        return false;
//    else if(A != 0)
//    {
//        double t1 = (-B + sqrt(delta)) / (2 * A);
//        double z1 = P[2] + t1 * dir[2];
//        bool t1_intersect = (t1 >= 0 && t1 <= 1 && z1 >= 0 && z1 <= cone.H);
//
//        double t2 = (-B - sqrt(delta)) / (2 * A);
//        double z2 = P[2] + t2 * dir[2];
//        bool t2_intersect = (t2 >= 0 && t2 <= 1 && z2 >= 0 && z2 <= cone.H);
//
//        return t1_intersect || t2_intersect;
//    }
//    else if(B != 0)
//    {
//        double t = -C / B;
//        double z = P[2] + t * dir[2];
//        return t >= 0 && t <= 1 && z >= 0 && z <= cone.H;
//    }
//    else return C == 0;
//}


//bool intersect(Cone cone, Plane rect)
//{
//    bool intersection = intersect(cone, glm::normalize(rect.u_dir)*float(rect.u), rect.start)
//                        || intersect(cone, glm::normalize(rect.u_dir)*float(rect.u), rect.start + glm::normalize(rect.v_dir)*float(rect.v))
//                        || intersect(cone, glm::normalize(rect.v_dir)*float(rect.v), rect.start)
//                        || intersect(cone, glm::normalize(rect.v_dir)*float(rect.v), rect.start + glm::normalize(rect.u_dir)*float(rect.u));
//
//
//    if(!intersection)
//    {
//        // It is possible that either the part of the plan lie
//        // entirely in the cone, or the inverse. We need to check.
//        glm::dvec3 center = P + (u + v) / 2;
//
//        // Is the face inside the cone (<=> center is inside the cone) ?
//        if(center.Z >= 0 && center.Z <= cone.H)
//        {
//            double r = (H - center.Z) * tan(cone.alpha);
//            if(center.X * center.X + center.Y * center.Y <= r)
//                intersection = true;
//        }
//
//        // Is the cone inside the face (this one is more tricky) ?
//        // It can be resolved by finding whether the axis of the cone crosses the face.
//        // First, find the plane coefficient (descartes equation)
//        Vector3D n = rect.u.crossProduct(rect.v);
//        double d = -(rect.P.X * n.X + rect.P.Y * n.Y + rect.P.Z * n.Z);
//
//        // Now, being in the face (ie, coordinates in (u, v) are between 0 and 1)
//        // can be verified through scalar product
//        if(n.Z != 0)
//        {
//            Vector3D M(0, 0, -d/n.Z);
//            Vector3D MP = M - rect.P;
//            if(MP.scalar(rect.u) >= 0
//               || MP.scalar(rect.u) <= 1
//               || MP.scalar(rect.v) >= 0
//               || MP.scalar(rect.v) <= 1)
//                intersection = true;
//        }
//    }
//    return intersection;
//}

