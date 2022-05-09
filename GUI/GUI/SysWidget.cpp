//
// Created by csh on 4/6/2022.
//
#include "SysWidget.h"

bool dirExists(const std::string& dirName_in) {
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  //something is wrong with your path!

    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // this is a directory!

    return false;    // this is not a directory!
}


SysWidget::SysWidget(QWidget* parent):QMainWindow(parent)
{
    setWindowTitle("Sys");
    resize(1700,1000);

    setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);

    CreateAction();
    CreateMenu();
    CreateWidgets();

    mSWC2VOL = new SWC2VOL();
}

void SysWidget::CreateAction()
{
    openSWCFileAction = new QAction(QStringLiteral("open swc"));
    connect(openSWCFileAction,&QAction::triggered,[this](){
        this->OpenSWC(
                QFileDialog::getOpenFileName(this,
                                             QStringLiteral("OpenFile"),
                                             QStringLiteral("."),
                                             QStringLiteral("swc files(*.swc)")
                ).toStdString()
        );
    });

    openOBJFileAction = new QAction(QStringLiteral("open obj"));
    connect(openOBJFileAction,&QAction::triggered,[this](){
        this->OpenOBJ(
                QFileDialog::getOpenFileName(this,
                                             QStringLiteral("OpenFile"),
                                             QStringLiteral("."),
                                             QStringLiteral("obj files(*.obj)")
                ).toStdString()
        );
    });

}

void SysWidget::CreateWidgets() {
    setDockOptions(QMainWindow::AnimatedDocks);
    setDockOptions(QMainWindow::AllowNestedDocks);
    setDockOptions(QMainWindow::AllowTabbedDocks);
    setDockNestingEnabled(true);

    renderWidget = new RenderWidget(this);
    renderDockWidget = new QDockWidget(QStringLiteral("Render"));
    renderDockWidget->setWidget(renderWidget);
    renderDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::BottomDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea,renderDockWidget);
    viewMenu->addAction(renderDockWidget->toggleViewAction());

    subwayMapWidget = new SubwayMapWidget(this);
    subwayMapDockWidget = new QDockWidget(QStringLiteral("SubwayMap"));
    subwayMapDockWidget->setWidget(subwayMapWidget);
    subwayMapDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::BottomDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea,subwayMapDockWidget);
    viewMenu->addAction(subwayMapDockWidget->toggleViewAction());

    splitDockWidget(renderDockWidget,subwayMapDockWidget,Qt::Vertical);

    inputWidget = new InputWidget(this);
    inputDockWidget = new QDockWidget(QStringLiteral("Info"));
    inputDockWidget->setWidget(inputWidget);
    inputDockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea,inputDockWidget);
    viewMenu->addAction(inputDockWidget->toggleViewAction());

    inputWidget->SetOtherWidget(subwayMapWidget);
    renderWidget->SetOtherWidget(subwayMapWidget,inputWidget);
}

void SysWidget::CreateMenu(){
    fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(openSWCFileAction);
    fileMenu->addAction(openOBJFileAction);
//    fileMenu->addAction(tr("Close"),this,[this](){
//    });
    viewMenu = menuBar()->addMenu("View");
}

void SysWidget::OpenSWC(std::string fileName){
    if(fileName.empty()) return;
    std::string inputswc = fileName;
    std::string j = inputswc.substr(inputswc.size() - 8,inputswc.size() - 4) ;
    int blocksize = 256;
    neuronInfo = new NeuronInfo();
    mSWC2VOL->GetNeuronInfo(inputswc, blocksize, neuronInfo);

    subwayMapWidget->SetPath(neuronInfo->paths,neuronInfo->point_vector.size(),neuronInfo->mainPaths,neuronInfo);
    subwayMapWidget->SelectPath(0);

    renderWidget->SWCLoaded(neuronInfo);
    inputWidget->SWCLoaded(neuronInfo);

    this->setWindowTitle(QString::fromStdString(fileName));
}

void SysWidget::OpenOBJ(std::string fileName) {
    if(fileName.empty()) return;
    renderWidget->SetObjFilePath(fileName);
}