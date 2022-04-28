//
// Created by csh on 4/6/2022.
//

#ifndef SYS_SYSWIDGET_H
#define SYS_SYSWIDGET_H
#include <QMainWindow>
#include <QtWidgets>
#include "util/NeuronInfo.h"
#include "RenderWidget.h"
#include "SubwayMapWidget.h"
#include "InputWidget.h"
#include "swc2vol/swc2vol.h"

class SysWidget:public QMainWindow{
Q_OBJECT
public:
    explicit SysWidget(QWidget* parent = nullptr);

    void OpenSWC(std::string fileName);

    void OpenOBJ(std::string fileName);

private:
    void CreateMenu();
    void CreateAction();
    void CreateWidgets();

private:
    QMenu* fileMenu;
    QMenu* viewMenu;

    QAction* openSWCFileAction;
    QAction* openOBJFileAction;

    NeuronInfo* neuronInfo;

    RenderWidget* renderWidget;
    QDockWidget* renderDockWidget;

    SubwayMapWidget* subwayMapWidget;
    QDockWidget* subwayMapDockWidget;

    InputWidget* inputWidget;
    QDockWidget* inputDockWidget;

    SWC2VOL* mSWC2VOL;
};


#endif //SYS_SYSWIDGET_H
