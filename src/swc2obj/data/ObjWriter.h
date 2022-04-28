#pragma once
#include "based.h"
#include <string>


class ObjWriter
{
public:
	ObjWriter(std::vector<Vertex> point_vector);
	~ObjWriter();

	void searchPath();
	void writeNormalizeToOneObj(const std::string& file_path, const int& x_start, const int& y_start,
	                            const int& z_start,
	                            const int& x_dim, const int& y_dim, const int& z_dim);
	void writeObj(const std::string& file_path, const double& x_space, const double& y_space, const double& z_space);
	void writeObjNormalization(const std::string& file_path, double x_bounding, double y_bounding, double z_bounding);

private:
	std::vector<Path>			paths;
	std::vector<Vertex>			point_vector;
};

