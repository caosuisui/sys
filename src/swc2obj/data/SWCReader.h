#pragma once
#include <string>
#include <vector>
#include "based.h"

class SWCReader
{
public:
	SWCReader();
	SWCReader(const std::string& file_name);
	void readSWC(const std::string& file_name);
	~SWCReader();
	std::vector<Vertex>& getPointVector();

private:
	std::string				file_name;
	std::vector<Vertex>		point_vector;
};

