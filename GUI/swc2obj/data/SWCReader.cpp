#include "SWCReader.h"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

SWCReader::SWCReader()
{
}

SWCReader::SWCReader(const std::string& file_name)
			:file_name(file_name)
{

}
void line_process(std::string &line, const std::string comment_str = "#")
{
	for (char &c : line)
	{
		if (c == '\t' || c == ',' || c == ';' || c == '\r' || c == '\n')
			c = ' ';
	}

	line.erase(0, line.find_first_not_of(" "));
	line.erase(line.find_last_not_of(" ") + 1);

	int n_comment_start = line.find_first_of(comment_str);
	if (n_comment_start != std::string::npos)
		line.erase(n_comment_start);
}

void SWCReader::readSWC(const std::string& file_name)
{
	this->file_name = file_name;

	std::ifstream read_file(file_name);
	if(read_file.is_open())
	{
		std::string line;
		while (std::getline(read_file, line))
		{
			line_process(line);
			if (line.empty()) continue;

			std::istringstream iss(line);

			struct Vertex point{};

			iss >> point.current_id >> point.point_type >> point.x >> 
				point.y >> point.z >> point.radius >> point.previous_id;
			
			point_vector.push_back(point);
		}
	}
	else
	{
		std::cout << "Error in opening file : " << file_name << std::endl;
	}

}

SWCReader::~SWCReader()
{
}

std::vector<Vertex>& SWCReader::getPointVector()
{
	return point_vector;
}
