//
// Created by csh on 4/18/2022.
//

#ifndef SYS_OBJMODE_H
#define SYS_OBJMODE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QFile>
#include <QTextStream>

class ObjMode : QObject
{
public:
    ObjMode() = default;
    ~ObjMode() override = default;

    bool loadObjModel(QString nFileStr, QVector<float>& nVertexData, QVector<unsigned int>& index, std::vector<float>& box );
private:
    std::vector<float> m_VertexInfo;
    std::vector<float> m_NormalInfo;
};

#endif //SYS_OBJMODE_H
