#include <iostream>
#include "SWCReader.h"
#include "ObjWriter.h"
#include "cmdline.h"

int testSWCandOBJ(int argc, char* argv[])
{
	std::string swc_file = R"(E:\14193_30neurons\N023.swc)";
	std::string obj_file = R"(E:\14193_30neurons\N023.obj)";
	int x_dimension = 28452;
	int y_dimension = 21866;
	int z_dimension = 4834;

	double x_space = 0.32;
	double y_space = 0.32;
	double z_space = 2;

	int x_n = 889;
	int y_n = 683;
	int z_n = 151;

	int is_normalized = false;

	if(argc<2)
	{
		std::cout << "Please use command line parameter." << std::endl;
		//return 0;
	}
	else
	{
		cmdline::parser a;
		a.add<std::string>("swc", 's', "Input swc file name", true, "");
		a.add<int>("x", 'X', "dimension of x direction", false, 28452);
		a.add<int>("y", 'Y', "dimension of y direction", false, 21866);
		a.add<int>("z", 'Z', "dimension of z direction", false, 4834);

		a.add<double>("sx", 'x', "space of x direction", false, 0.32);
		a.add<double>("sy", 'y', "space of y direction", false, 0.32);
		a.add<double>("sz", 'z', "space of z direction", false, 2);

		a.add<int>("nx", 'i', "new dimension of x direction", false, 889);
		a.add<int>("ny", 'j', "new dimension of y direction", false, 683);
		a.add<int>("nz", 'k', "new dimension of z direction", false, 151);

		a.add<int>("normalize", 'n', "whether normalize the obj file to [0,1] or not", false , 0);

		a.parse_check(argc, argv);
		swc_file = a.get<std::string>("swc");
		x_dimension = a.get<int>("x");
		y_dimension = a.get<int>("y");
		z_dimension = a.get<int>("z");

		x_space = a.get<double>("sx");
		y_space = a.get<double>("sy");
		z_space = a.get<double>("sz");


		is_normalized = a.get<int>("normalize");

		obj_file = swc_file.substr(0, swc_file.find_last_of('.')) + ".obj";
	}

	double x_bounding = x_dimension * x_space;
	double y_bounding = y_dimension * y_space;
	double z_bounding = z_dimension * z_space;


	SWCReader swc_reader;
	swc_reader.readSWC(swc_file);

	auto point_vector = swc_reader.getPointVector();

	std::cout << "swc file " +swc_file+ " has been loaded." << std::endl;

	ObjWriter obj_writer(point_vector);

	obj_writer.searchPath();

	if(is_normalized)
	{
		obj_writer.writeObjNormalization(obj_file, x_bounding/(x_n), 
			y_bounding / (y_n), z_bounding / (z_n*z_space/x_space));
	}
	else
	{
		obj_writer.writeObj(obj_file,x_space,y_space,z_space);
	}
	auto is_normalized_to_one = false;
	if(is_normalized_to_one)
	{
		obj_writer.writeNormalizeToOneObj(obj_file, 6200, 6500, 2250, 512, 512, 512);
	}

	std::cout << "obj file" + obj_file + " has been generated." << std::endl;

	return 0 ;
}