#include <iostream>
#include <windows.h>
#include <vector>

#include "config.h"
#include "swc2vol.h"
#include "vol2obj.h"
#include "cxxopts.hpp"


void read_directory(const std::string& name, std::vector<std::string>& v)
{
    std::string pattern(name);
    pattern.append("*");
    WIN32_FIND_DATA data;
    HANDLE hFind;
    if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
        do {
            if(data.cFileName[0]!='.')
            {
                v.push_back((name+data.cFileName));
                std::cout<<"add file: "<<data.cFileName<<std::endl ;
            }
        } while (FindNextFile(hFind, &data) != 0);
        FindClose(hFind);
    }
}

bool dirExists(const std::string& dirName_in) {
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!

    return false;    // this is not a directory!
}


void parse(char** argv,int argc,cxxopts::ParseResult& result)
{
    try {
        cxxopts::Options options(argv[0], " - example command line options");
        options
                .positional_help("[optional args]")
                .show_positional_help();

        options
                .allow_unrecognised_options()
                .add_options()
                        ("s,swcdir", "swcdir which you want to input ", cxxopts::value<std::string>())
                        ("o,outputdir", "objdir you want to output ", cxxopts::value<std::string>())
                        ("b,blocksize", "blocksize you want to use", cxxopts::value<int>());


        result = options.parse(argc, argv);

        if (result.count("swcdir")) {
            std::cout << "swcdir = " << result["s"].as<std::string>()
                      << std::endl;
        }

        if (result.count("outputdir")) {
            std::cout << "outputdir = " << result["o"].as<std::string>()
                      << std::endl;
        }

        if (result.count("blocksize")) {
            std::cout << "blocksize = " << result["b"].as<int>()
                      << std::endl;
        }


        std::cout << "Arguments remain = " << argc << std::endl;

        auto arguments = result.arguments();
        std::cout << "Saw " << arguments.size() << " arguments" << std::endl;
    } catch (const cxxopts::OptionException& e)
    {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }

}

int main(int argc , char ** argv) {
    cxxopts::ParseResult res ;
    parse(argv,argc,res) ;


    std::string swcdir ;
    if (res.count("swcdir")) {
       swcdir =  res["s"].as<std::string>() ;
    }


    std::string outputdir ;
    if (res.count("outputdir")) {
        outputdir = res["o"].as<std::string>() ;
    }

    int blocksize ;
    if (res.count("blocksize")) {
       blocksize =res["b"].as<int>() ;
    }

//    std::vector<std::string> swcfiles  ;
//    read_directory(swcdir,swcfiles);
//    for(auto i = swcfiles.begin() ;i < swcfiles.end() ; i++)
//    {
//        std::string j = (*i).substr((*i).size() - 8,(*i).size() - 4) ;
//        std::string volumedir = CONDIF_DIR+std::string("/newResult/"+j +"/");
//
//        if(dirExists(volumedir))
//            RemoveDirectory(volumedir.c_str()) ;
//        if(!dirExists(volumedir))
//            CreateDirectory(volumedir.c_str(),NULL) ;
//
//        swc2vol((*i), blocksize, volumedir) ;
//    }

    std::string inputswc = swcdir;
    std::string j = inputswc.substr(inputswc.size() - 8,4) ;

    if(dirExists(outputdir)){
        RemoveDirectory(outputdir.c_str()) ;
    }

    if(!dirExists(outputdir)){
        std:: cout << CreateDirectory(outputdir.c_str(),NULL)  << std::endl;
    }

    std::string volumedir = outputdir + std::string("vol/");
    std::string objdir = outputdir + std::string("obj/");

    bool genVol = true;
    if(genVol){
        std::cout<<"create volume "<<volumedir<<std::endl ;
        if(dirExists(volumedir)){
            RemoveDirectory(volumedir.c_str()) ;
        }

        if(!dirExists(volumedir)){
            std::cout << "create dir for vol" << std::endl;
            CreateDirectory(volumedir.c_str(),NULL);
        }
//        SWC2VOL* mSWC2VOL = new SWC2VOL();
//        mSWC2VOL->swc2vol(inputswc, blocksize, volumedir) ;
    }

    bool genObj = true;
    if(genObj){
        if(dirExists(objdir))
            RemoveDirectory(objdir.c_str()) ;
        if(!dirExists(objdir)){
            std::cout << "create dir for obj" << std::endl;
            CreateDirectory(objdir.c_str(),NULL);
        }

//        VOL2OBJ* mVOL2OBJ = new VOL2OBJ();
//        mVOL2OBJ->vol2obj(volumedir,objdir);
    }

    return 0;
}