//
// Created by csh on 4/13/2022.
//

#ifndef SYS_VOL2OBJ_H
#define SYS_VOL2OBJ_H

#include <iostream>
#include <io.h>
#include <algorithm>

#include "cmdline.h"
#include "BVH.h"
#include "config.h"
#include "ObjMerger.h"

#include <vtkImageReader.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkMarchingCubes.h>
#include <vtkPolyData.h>
#include <vtkAppendPolyData.h>
#include <vtkOBJWriter.h>
#include <vtkMetaImageReader.h>
#include <vtkReaderAlgorithm.h>
#include <vtkImageCast.h>
#include <vtkDecimatePro.h>
#include <vtkOBJReader.h>


class VOL2OBJ{

public:

    void vol2obj(std::string volume_dir,std::string obj_dir){
        appender = vtkSmartPointer<vtkAppendPolyData>::New();

        MarchingCubeForAllFiles(volume_dir,obj_dir);

        std::string objNameBeforeMerge = obj_dir + std::string("objBeforeMerge.obj");
        appender->Update();
        auto polyUnion = appender->GetOutput();
        WriteObj(polyUnion,objNameBeforeMerge);

        ObjMerger* merger = new ObjMerger(objNameBeforeMerge);
        std::string mergedObjName = obj_dir + std::string("mergedObj.obj");
        merger->writeNew1(mergedObjName);
        std::string mergedObjName1 = obj_dir + std::string("mergedObj1.obj");
        merger->writeNew(mergedObjName1);

        std::string decimatedObjName = obj_dir + std::string("simplifiedObj.obj");
        SimplifyObj(mergedObjName1,decimatedObjName);
    }

    void SimplifyObj(std::string infile,std::string outfile){
        vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
        reader->SetFileName(infile.c_str());
        reader->Update();

        vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
        decimate->SetInputConnection(reader->GetOutputPort());
        decimate->SetTargetReduction(0.6);
        decimate->Update();

        WriteObj(decimate->GetOutput(),outfile);
    }

    void MarchingCubeForAllFiles(std::string volume_dir,std::string obj_dir){
        std::vector<std::string> files;
        getFiles(volume_dir,files);
        for(auto file:files){
            auto pos1 = file.find_last_of("/");
            auto pos2 = file.find_last_of(".");
            std::string name = file.substr(pos1 + 1, pos2-pos1-1);
            std::string objName = obj_dir + name + ".obj";
            //MarchingCube(file,objName);
            MarchingCubeUsingMHD(file,objName);
        }
    }

    void MarchingCube(std::string file, std::string objname){
        std::cout << file << std::endl;

        vtkSmartPointer<vtkImageReader2> reader;

        if(!usingmhd){
            auto nameStart = file.find_last_of('/')+1;
            auto nameEnd = file.find_last_of('.')-1;
            std::string filename = file.substr(nameStart,nameEnd - nameStart +1);

            std::vector<std::string> elems;
            std::stringstream ss(filename);
            std::string elem;
            char delim = '_';
            while(std::getline(ss,elem,delim)){
                elems.push_back(elem);
            }
            int extent[6];
            extent[0] = std::stoi(elems[1]);
            extent[1] = extent[0] + 255;
            extent[2] = std::stoi(elems[2]);
            extent[3] = extent[2] + 255;
            extent[4] = std::stoi(elems[3]);
            extent[5] = extent[4] + 255;

            reader = vtkSmartPointer<vtkImageReader>::New();
            reader->SetFileName(file.c_str());
            reader->SetFileDimensionality(3);
            reader->SetDataScalarType(VTK_DOUBLE);
            //reader->SetDataExtent(extent);
            reader->SetDataExtent(0,255,0,255,0,255);
            reader->SetDataSpacing(0.5,0.5,0.5);
            //起点位置
            reader->SetDataOrigin(extent[0],extent[2],extent[4]);
            reader->Update();
        }
        else{
            reader = vtkSmartPointer<vtkMetaImageReader>::New();
            reader->SetFileName(file.c_str());
            reader->Update();


        }

        vtkImageData* imageData = reader->GetOutput();
        vtkMarchingCubes* iso = vtkMarchingCubes::New();
        //iso->SetInputData(imageData);
        iso->SetInputConnection(reader->GetOutputPort());
        iso->SetNumberOfContours(1);
        iso->SetValue(0,0.1);
        iso->ComputeGradientsOn();
        iso->ComputeNormalsOn();
        iso->ComputeScalarsOn();
        iso->Update();

        appender->AddInputConnection(iso->GetOutputPort());

        vtkPolyData* poly = iso->GetOutput();
        //allPoly.push_back(poly);
        WriteObj(poly,objname);
    }

    void MarchingCubeUsingMHD(std::string mhdname, std::string objname){
        vtkSmartPointer<vtkMetaImageReader> reader = vtkSmartPointer<vtkMetaImageReader>::New();
        reader->SetFileName(mhdname.c_str());
        reader->Update();

        vtkSmartPointer<vtkImageCast> caster = vtkSmartPointer<vtkImageCast>::New();
        caster->SetInputConnection(reader->GetOutputPort());
        caster->SetOutputScalarTypeToDouble();
        caster->Update();

        //vtkImageData* imageData = reader->GetOutput();
        vtkMarchingCubes* iso = vtkMarchingCubes::New();
        iso->SetInputConnection(caster->GetOutputPort());
        iso->SetNumberOfContours(1);
        iso->SetValue(0,0.1);
        iso->ComputeGradientsOn();
        iso->ComputeNormalsOn();
        iso->ComputeScalarsOn();
        iso->Update();

        appender->AddInputConnection(iso->GetOutputPort());

        vtkPolyData* poly = iso->GetOutput();
        WriteObj(poly,objname);
    }

    void WriteObj(vtkPolyData* poly, std::string objName) {
        vtkSmartPointer<vtkOBJWriter> writer = vtkSmartPointer<vtkOBJWriter>::New();
        writer->SetInputData(poly);
        writer->SetFileName(objName.c_str());
        writer->Update();
    }

    void CollectObj(std::string objPath){
        for(auto item : allPoly){
            appender->AddInputData(item);
        }
        auto polyUnion = appender->GetOutput();
        WriteObj(polyUnion,objPath);
    }

    void getFiles( std::string path, std::vector<std::string>& files)//raw 0, mhd 1
    {
        std::cout << path << std::endl;
        //文件句柄
        long long  hFile   =   0;
        //文件信息
        struct _finddata_t fileinfo;
        std::string p;
        if((hFile = _findfirst((path + std::string("*")).c_str(),&fileinfo)) !=  -1)
        {
            do
            {
                //如果是目录,迭代之
                //如果不是,加入列表
                if(!(fileinfo.attrib &  _A_SUBDIR))
                {
                    std::string name = path + fileinfo.name;
                    if(name.substr(name.find_last_of(".")+1,3) == (usingmhd?"mhd":"raw")){

                        files.push_back(name);
                    }

                }
            }while(_findnext(hFile, &fileinfo) == 0);
            _findclose(hFile);
        }
    }

    std::vector<vtkPolyData*> allPoly;
    vtkSmartPointer<vtkAppendPolyData> appender;
    bool usingmhd = true;
};


#endif //SYS_VOL2OBJ_H
