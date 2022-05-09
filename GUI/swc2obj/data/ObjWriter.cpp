#include "ObjWriter.h"
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>

ObjWriter::ObjWriter(std::vector<Vertex> point_vec):point_vector(point_vec)
{
	for (auto i = 0; i < point_vector.size(); i++)
	{
		point_vector[i].index = i + 1;
		point_vector[i].degree = 0;
	}

}

ObjWriter::~ObjWriter()
{
}

void ObjWriter::searchPath()
{
	std::vector<int> leaf_nodes;
	std::map<int, int> vertex_hash;
	for (auto i = 0; i < point_vector.size(); i++)
	{
		vertex_hash[point_vector[i].current_id] = i;
	}
	for(auto& i: point_vector)
	{
		if(i.previous_id!=-1)
		{
			point_vector[vertex_hash[i.previous_id]].degree++;
		}
	}
	for (auto& i : point_vector)
	{
		if(i.degree==0 && i.previous_id!=-1)
		{
			leaf_nodes.push_back(i.current_id);
		}
	}
	//2. ��Ҷ�ӽ���������ǰ����·��
	auto path_index = 0;
	for(auto & left_node : leaf_nodes)
	{
		Path path;
		std::vector<Vertex> vertices;
		// auto & cur_vertex = point_vector[vertex_hash[left_node]];
		// cur_vertex.is_visited = true;
		// vertices.push_back(cur_vertex);

		auto cur_idx = left_node;
		while(cur_idx!=-1 && !point_vector[vertex_hash[cur_idx]].is_visited)
		{
			auto &buf_vertex = point_vector[vertex_hash[cur_idx]];
			buf_vertex.is_visited = true;
			vertices.push_back(buf_vertex);
			cur_idx = buf_vertex.previous_id;
		}

		if(cur_idx!=-1)
		{
			vertices.push_back(point_vector[vertex_hash[cur_idx]]);
		}

		path.path = vertices;
		path.path_type = path_index;
		path_index++;
		paths.push_back(path);
	}

	// auto sum = 0;
	// for(auto & path: paths)
	// {
	// 	sum += path.path.size();
	// }
	// std::cout << "Sum :\t" << sum << std::endl;
}

void ObjWriter::writeNormalizeToOneObj(const std::string & file_path, const int & x_start, const int & y_start, const int & z_start,
	const int & x_dim, const int & y_dim, const int & z_dim)
{

	std::ofstream write_file(file_path);

	write_file << "#vertex_num " << static_cast<unsigned int>(point_vector.size()) << std::endl;

	for(auto & vertex: point_vector)
	{
		
		write_file << "v " << (vertex.x-x_start)/ x_dim <<" "<< (vertex.y - y_start) / y_dim << " " << (vertex.z - z_start) / z_dim << std::endl;
	
		
	}

	for(auto j=0;j< paths.size();j++)
	{
		auto & path = paths[j].path;
		write_file << "g" << " path" << paths[j].path_type << std::endl;
		for (int i = path.size() - 1; i >= 1; i--)
		{
			write_file << "l"<< " " << path[i].index<< " " << path[i-1].index<< std::endl;
		}
	}
	write_file.close();
}


void ObjWriter::writeObj(const std::string & file_path, const double& x_space, const double& y_space, const double& z_space)
{

	std::ofstream write_file(file_path);

	write_file << "#vertex_num " << static_cast<unsigned int>(point_vector.size()) << std::endl;
	std::cout << "#vertex_num " << static_cast<unsigned int>(point_vector.size()) << std::endl;

    write_file.fill('0') ;
    write_file.precision(10) ;

	double max_x = -999999, min_x = 999999;
	double max_y = -999999, min_y = 999999;
	double max_z = -999999, min_z = 999999;

	for (auto & vertex : point_vector)
	{
		
		write_file << "v " << std::fixed << std::setprecision(8)<< (vertex.x) / x_space
		            << " " << std::fixed << std::setprecision(8) << (vertex.y) / y_space
		            << " " << std::fixed << std::setprecision(8) << (vertex.z) / z_space << std::endl;
		vertex.x /= x_space;
		vertex.y /= y_space;
		vertex.z /= z_space;

		max_x = std::max(max_x, vertex.x);
		max_y = std::max(max_y, vertex.y);
		max_z = std::max(max_z, vertex.z);
		min_x = std::min(min_x, vertex.x);
		min_y = std::min(min_y, vertex.y);
		min_z = std::min(min_z, vertex.z);
	}

	std::cout << "Bounding box : [" << min_x << ", " << min_y << ", " << min_z << "] --- [" << max_x << ", " << max_y << ", " << max_z << "]" << std::endl;

	for (auto j = 0; j < paths.size(); j++)
	{
		auto & path = paths[j].path;
		write_file << "g" << " path" << paths[j].path_type << std::endl;
		for (int i = path.size() - 1; i >= 1; i--)
		{
			write_file << "l" << " " << path[i].index << " " << path[i - 1].index << std::endl;
		}
	}
	write_file.close();
}

void ObjWriter::writeObjNormalization(const std::string & file_path, const double x_bounding, const double y_bounding, const double z_bounding)
{

	std::ofstream write_file(file_path);

	write_file << "#vertex_num " << static_cast<unsigned int>(point_vector.size()) << std::endl;

    write_file.fill('0') ;
	write_file.precision(10) ;


	for (auto & vertex : point_vector)
	{

		write_file << "v " << double(vertex.x /x_bounding) <<
		              " "  << double(vertex.y / y_bounding )<<
		              " "  << double(vertex.z / z_bounding) << std::endl;
		vertex.x /= x_bounding;
		vertex.y /= y_bounding;
		vertex.z /= z_bounding;
	}

	for (auto j = 0; j < paths.size(); j++)
	{
		auto & path = paths[j].path;
		write_file << "g" << " path" << paths[j].path_type << std::endl;
		for (int i = path.size() - 1; i >= 1; i--)
		{
			write_file << "l" << " " << path[i].index << " " << path[i - 1].index << std::endl;
		}
	}
	write_file.close();
}