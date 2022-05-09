//
// Created by csh on 4/18/2022.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include "objmode.h"

bool ObjMode::loadObjModel(QString nFileStr, QVector<float>& nVertexData, QVector<unsigned int>& index,std::vector<float>& box)
{
    QFile nObjFile(nFileStr);
    if (!nObjFile.exists())
        return false;

    if (!nObjFile.open(QFile::ReadOnly))
        return false;

    float xmin,xmax,ymin,ymax,zmin,zmax;
    xmin = ymin = zmin = 999999;
    xmax = ymax = zmax = -999999;

    //初始化
    nVertexData.clear();
    index.clear();

    m_NormalInfo.clear();
    m_VertexInfo.clear();
    std::time_t time,ptime;
    std::time(&ptime);

    std::ifstream nTextStream(nFileStr.toStdString());
    if(!nTextStream.is_open()){
        std::cout << " open file error" << std::endl;
    }

    for (;!nTextStream.eof();)
    {
        std::string nLineString;
        std::getline(nTextStream,nLineString);
        if (nLineString.length() <= 2)
            continue;

        std::istringstream iss(nLineString);
        std::string type;
        iss >> type;
        if (type == "v")
        {
            float x,y,z;
            iss >> x >> y >> z;
            m_VertexInfo.push_back(x);
            m_VertexInfo.push_back(y);
            m_VertexInfo.push_back(z);
        }
        else if(type == "vn"){
            float x,y,z;
            iss >> x >> y >> z;
            m_NormalInfo.push_back(x);
            m_NormalInfo.push_back(y);
            m_NormalInfo.push_back(z);
        }
        else if (type == "f")
        {
            std::string v[3];

            iss >> v[0] >> v[1] >> v[2];

            for (int i=0; i<3; ++i)
            {
                int posIndex;
                int normalIndex;

                std::string str;
                std::istringstream iss1(v[i]);

                std::getline(iss1,str,'/');
                posIndex = std::stoi(str);
                std::getline(iss1,str,'/');
                std::getline(iss1,str);
                normalIndex = std::stoi(str);

                posIndex -= 1;
                normalIndex -=1;

                float posx,posy,posz,norx,nory,norz;
                posx = m_VertexInfo[posIndex*3];
                posy = m_VertexInfo[posIndex*3+1];
                posz = m_VertexInfo[posIndex*3+2];
                norx = m_NormalInfo[normalIndex*3];
                nory = m_NormalInfo[normalIndex*3+1];
                norz = m_NormalInfo[normalIndex*3+2];

                nVertexData.push_back(posx);
                nVertexData.push_back(posy);
                nVertexData.push_back(posz);
                nVertexData.push_back(norx);
                nVertexData.push_back(nory);
                nVertexData.push_back(norz);

                xmin = std::min(posx,xmin);
                ymin = std::min(posy,ymin);
                zmin = std::min(posz,zmin);

                xmax = std::max(posx,xmax);
                ymax = std::max(posy,ymax);
                zmax = std::max(posz,zmax);
            }
        }
    }
    nObjFile.close();
    std::time(&time);
    std::cout << " read obj time : " << time - ptime << std::endl;

    std::cout << "obj bounding box: " << xmin << " " << ymin << " " << zmin << " to " << xmax << " "  << ymax << " " << zmax << std::endl;
    std::cout << "dimension " << xmax - xmin << " " << ymax - ymin << " "<< zmax - zmin << std::endl;

    box.push_back(xmin);
    box.push_back(ymin);
    box.push_back(zmin);
    box.push_back(xmax);
    box.push_back(ymax);
    box.push_back(zmax);
    return true;
}
