//
// Created by csh on 4/6/2022.
//

#include <QApplication>
#include "SysWidget.h"
int main(int argc,char** argv){
    QApplication app(argc,argv);
    SysWidget w;

    w.show();

    return app.exec();
    return 0;
}