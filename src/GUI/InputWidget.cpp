//
// Created by csh on 4/20/2022.
//

#include "InputWidget.h"

InputWidget::InputWidget(QWidget *parent):QWidget(parent) {
    lastPointid = nextPointid = -1;

    this->setFixedSize(INPUT_WIDTH,INPUT_HEIGHT);

    auto widgetLayout = new QVBoxLayout;
    widgetLayout->setAlignment(Qt::AlignTop);

    //路径导航
    treeWidget = new QTreeWidget(this);
    treeWidget->setFixedSize(INPUT_WIDTH - 10, 200);
    treeWidget->setHeaderHidden(true);
    treeWidget->setStyleSheet("QTreeView::branch::selected{background-color:lightgray;} QTreeView::item::selected{background-color:lightgray;} ");
    widgetLayout->addWidget(treeWidget);

    auto buttonGroup = new QGroupBox("Edit");
    auto buttonLayout = new QHBoxLayout;
    buttonLayout->setAlignment(Qt::AlignLeft);
    buttonGroup->setLayout(buttonLayout);

    addButton = new QPushButton("a");
    addButton->setFixedSize(20,20);
    addButton->setCheckable(true);
    deleteButton = new QPushButton("d");
    deleteButton->setFixedSize(20,20);
    deleteButton->setCheckable(true);
    connectButton = new QPushButton("c");
    connectButton->setFixedSize(20,20);
    connectButton->setCheckable(true);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addWidget(connectButton);
    widgetLayout->addWidget(buttonGroup);

    //选中点信息
    auto pointInfoWidget = new QGroupBox;
    pointInfoWidget->setFixedSize(INPUT_WIDTH - 10,300);

    auto pointInfoLayout =new QVBoxLayout;
    pointInfoWidget->setLayout(pointInfoLayout);
    widgetLayout->addWidget(pointInfoWidget);

    //当前选中点
    {
        auto currentPointLayout =new QVBoxLayout;

        currentPointLabel = new QLabel("Current Vertex");
        currentPointLayout->addWidget(currentPointLabel);

        //位置
        auto currentPosLayout = new QHBoxLayout;

//        auto currentPosLabel = new QLabel("    position : none");
//        currentPosLayout->addWidget(currentPosLabel);

        currentPointPosLine = new QLabel("    position : none");
        currentPosLayout->addWidget(currentPointPosLine);

        currentPointLayout->addLayout(currentPosLayout);

        //半径
        auto currentRadiusLayout = new QHBoxLayout;

        auto currentRadiusLabel = new QLabel("    radius : ");
        currentRadiusLayout->addWidget(currentRadiusLabel);

        currentPointRadiusLine = new QLineEdit("0");
        currentPointRadiusLine->setFixedWidth(100);
        currentRadiusLayout->addWidget(currentPointRadiusLine);

        currentRadiusLayout->addStretch();
        currentPointLayout->addLayout(currentRadiusLayout);

        pointInfoLayout->addLayout(currentPointLayout);
    }

    //上一个点
    {
        auto lastPointLayout =new QVBoxLayout;

        auto lastTitleLayout = new QHBoxLayout;
        lastPointLabel = new QLabel("Last Vertex");
        lastTitleLayout->addWidget(lastPointLabel);

        lastButton = new QPushButton;
        lastButton->setCheckable(true);
        //lastButton->setAutoExclusive(true);
        lastButton->setFixedSize(20,20);
        QString lastButtonStyle = "QPushButton:checked{background-color:rgb(80,50,0);}";
        lastButton->setStyleSheet(lastButtonStyle);
        lastTitleLayout->addWidget(lastButton);
        lastTitleLayout->addStretch();

        lastPointLayout->addLayout(lastTitleLayout);

        //位置
        auto lastPosLayout = new QHBoxLayout;

        lastPointPosLine = new QLabel("    position : none");
        lastPosLayout->addWidget(lastPointPosLine);

        lastPointLayout->addLayout(lastPosLayout);

        //半径
        auto lastRadiusLayout = new QHBoxLayout;

        lastPointRadiusLine = new QLabel("    radius : 0");
        lastRadiusLayout->addWidget(lastPointRadiusLine);

        lastPointLayout->addLayout(lastRadiusLayout);

        pointInfoLayout->addLayout(lastPointLayout);
    }

    //下一个点
    {
        auto nextPointLayout =new QVBoxLayout;

        auto nextTitleLayout = new QHBoxLayout;
        nextPointLabel = new QLabel("Next Vertex");
        nextTitleLayout->addWidget(nextPointLabel);

        nextButton = new QPushButton;
        nextButton->setCheckable(true);
        //nextButton->setAutoExclusive(true);
        nextButton->setFixedSize(20,20);
        QString nextButtonStyle = "QPushButton:checked{background-color:rgb(0,160,0);}";
        nextButton->setStyleSheet(nextButtonStyle);
        nextTitleLayout->addWidget(nextButton);
        nextTitleLayout->addStretch();

        nextPointLayout->addLayout(nextTitleLayout);

        //位置
        auto nextPosLayout = new QHBoxLayout;

        nextPointPosLine = new QLabel("    position : none");
        nextPosLayout->addWidget(nextPointPosLine);

        nextPointLayout->addLayout(nextPosLayout);

        //半径
        auto nextRadiusLayout = new QHBoxLayout;

        nextPointRadiusLine = new QLabel("    radius : 0");
        nextRadiusLayout->addWidget(nextPointRadiusLine);

        nextPointLayout->addLayout(nextRadiusLayout);

        pointInfoLayout->addLayout(nextPointLayout);
    }

    //插值按键
    auto interpolateButtonLayout = new QHBoxLayout;
    interpolateButtonLayout->setAlignment(Qt::AlignCenter);
    interpolateButton = new QPushButton("interpolate");
    interpolateButton->setFixedWidth(100);
    interpolateButtonLayout->addWidget(interpolateButton);

    pointInfoLayout->addLayout(interpolateButtonLayout);

    submitButton = new QPushButton("submit");
    widgetLayout->addWidget(submitButton);

    this->setLayout(widgetLayout);

    buttons.push_back(deleteButton);
    buttons.push_back(lastButton);
    buttons.push_back(nextButton);

    //修改当前选中的路径
    connect(treeWidget,&QTreeWidget::currentItemChanged,this,&InputWidget::ChangePath);

    //插值半径的槽和信号连接
    connect(interpolateButton,&QPushButton::clicked,this,&InputWidget::InterpolateRadius);

    //修改半径的槽和信号连接
    connect(currentPointRadiusLine,&QLineEdit::editingFinished,this,&InputWidget::EditRadius);

//    connect(addButton,&QPushButton::toggled,this, &InputWidget::AddMethod);
    connect(deleteButton,&QPushButton::toggled,this, &InputWidget::GetButtonState);
//    connect(connectButton,&QPushButton::toggled,this, &InputWidget::ConnectMethod);

    connect(lastButton, &QPushButton::toggled,this,&InputWidget::GetButtonState);
    connect(nextButton, &QPushButton::toggled,this,&InputWidget::GetButtonState);

    connect(submitButton,&QPushButton::clicked,this,&InputWidget::ReconstructionSlot);

    neuronInfo = nullptr;
}

void InputWidget::ReconstructionSlot(bool) {
    neuronInfo->PartialReconstruction();
}

void InputWidget::EditRadius() {
    if(!currentPoint) return;
    neuronInfo->EditRadius(currentPoint->current_id,currentPointRadiusLine->text().toDouble());
}

void InputWidget::InterpolateRadius() {
    if(lastPointid != -1 && nextPointid != -1)
    neuronInfo->Interpolate(lastPointid, nextPointid);
}

void InputWidget::SetOtherWidget(SubwayMapWidget *in_subwayMapWidget) {
    this->subwayMapWidget = in_subwayMapWidget;

    //修改选中点的槽和信号连接
    connect(subwayMapWidget,&SubwayMapWidget::SelectVertexSignal,this,&InputWidget::ChangeCurrentPoint);
    connect(subwayMapWidget,&SubwayMapWidget::SelectLastVertexSignal,this,&InputWidget::ChangeLastPoint);
    connect(subwayMapWidget,&SubwayMapWidget::SelectNextVertexSignal,this,&InputWidget::ChangeNextPoint);

    connect(this,&InputWidget::ChangeSelectionStateSignal,subwayMapWidget,&SubwayMapWidget::ChangeSelectionState);
}

void InputWidget::GetButtonState(bool state) {
    if(state){
        auto senderButton = sender();
        for(auto button : buttons){
            if(senderButton == button) continue;
            if(button->isChecked())
                button->setChecked(false);
        }
    }
//
//    if(sender() == lastButton){
//        if(state && nextButton->isChecked()){
//            nextButton->setChecked(false);
//            deleteButton->setChecked(false);
//        }
//    }
//    else if(sender() == nextButton){
//        if(state && lastButton->isChecked()){
//            lastButton->setChecked(false);
//            deleteButton->setChecked(false);
//        }
//    }

    if(!subwayMapWidget) return;
    if(lastButton->isChecked()){
        emit ChangeSelectionStateSignal(SubwayMapWidget::SelectionState::LastPoint);
        return;
    }
    if(nextButton->isChecked()){
        emit ChangeSelectionStateSignal(SubwayMapWidget::SelectionState::NextPoint);
        return;
    }
    if(deleteButton->isChecked()){
        emit ChangeSelectionStateSignal(SubwayMapWidget::SelectionState::Delete);
        return;
    }
    emit ChangeSelectionStateSignal(SubwayMapWidget::SelectionState::Normal);
}

void InputWidget::ChangeCurrentPoint(int pathid, int vertexid) {
    if(pathid != -1){
        currentPoint = &neuronInfo->paths[pathid].path[vertexid];
        currentPointPathID = pathid;
        currentPointVertexID = vertexid;

        //修改文本显示
        //当前点
        currentPointLabel->setText("Current Point " + QString::number(currentPoint->current_id));
        QString currentPos = "    pos : ( " + QString::number(currentPoint->x) +
                             ", " + QString::number(currentPoint->y) +
                             ", " + QString::number(currentPoint->z) + ")";
        currentPointPosLine->setText(currentPos);

        currentPointRadiusLine->setText(QString::number(currentPoint->radius));
    }
    else{
        currentPointLabel->setText("Current Point");
        currentPointPosLine->setText("    position : none");
        currentPointRadiusLine->setText("0");
    }
}

void InputWidget::ChangeLastPoint(int pathid, int vertexid) {
    if(pathid != -1){
        const auto& lastPoint = neuronInfo->paths[pathid].path[vertexid];
        lastPointid = lastPoint.current_id;
        lastPointLabel->setText("Last Point " + QString::number(lastPointid));
        QString lastPos = "    pos : ( " + QString::number(lastPoint.x) +
                          ", " + QString::number(lastPoint.y) +
                          ", " + QString::number(lastPoint.z) + ")";
        lastPointPosLine->setText(lastPos);

        QString lastR = "    radius : " + QString::number(lastPoint.radius);
        lastPointRadiusLine->setText(lastR);
    }
    else{
        lastPointid = -1;
        lastPointLabel->setText("Last Point");
        lastPointPosLine->setText("    position : none");
        lastPointRadiusLine->setText("    radius : 0");
    }
}

void InputWidget::ChangeNextPoint(int pathid, int vertexid) {
    if(pathid != -1){
        const auto& nextPoint = neuronInfo->paths[pathid].path[vertexid];
        nextPointid = nextPoint.current_id;
        nextPointLabel->setText("Next Point " + QString::number(nextPointid));
        QString nextPos = "    pos : ( " + QString::number(nextPoint.x) +
                          ", " + QString::number(nextPoint.y) +
                          ", " + QString::number(nextPoint.z) + ")";
        nextPointPosLine->setText(nextPos);

        QString nextR = "    radius : " + QString::number(nextPoint.radius);
        nextPointRadiusLine->setText(nextR);
    }
    else{
        nextPointLabel->setText("Next Point");
        nextPointid = -1;
        nextPointPosLine->setText("    position : none");
        nextPointRadiusLine->setText("    radius : 0");
    }
}

void InputWidget::ChangePath(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    auto s = current->text(1);
    int pathIndex = s.toInt();
    subwayMapWidget->SelectPath(pathIndex);
    ChangeCurrentPoint(-1,-1);
    ChangeLastPoint(-1,-1);
    ChangeNextPoint(-1,-1);
    if(lastButton->isChecked()) lastButton->setChecked(false);
    if(nextButton->isChecked()) nextButton->setChecked(false);
}

void InputWidget::SWCLoaded(NeuronInfo *in_neuronInfo) {
    neuronInfo = in_neuronInfo;


    for(int i = 0;i < neuronInfo->mainPaths.size();i++){
        int mainIndex = neuronInfo->mainPaths[i];
        auto mainPath = &neuronInfo->paths[mainIndex];

        QStringList list;
        list.push_back("Neurite " + QString::number(i));
        list.push_back(QString::number(mainPath->path_type));
        //list.push_back(QString::number(mainPath->sub_paths_index.size()));
        auto treeItem = new QTreeWidgetItem(treeWidget,list);
        treeItem->setText(0,list[0]);
        GenTreeWidget(treeItem,mainPath);
    }


}

void InputWidget::GenTreeWidget(QTreeWidgetItem *parent, Path *path) {
    for(int i = 0;i < path->sub_paths_index.size();i++){
        QStringList list;
        list.push_back("branch " + QString::number(i));
        list.push_back(QString::number(path->sub_paths_index[i]));
        //list.push_back(QString::number(path->sub_paths_index.size()));
        auto item = new QTreeWidgetItem(parent,list);
        item->setText(0,list[0]);
        GenTreeWidget(item,&neuronInfo->paths[path->sub_paths_index[i]]);
    }
}
