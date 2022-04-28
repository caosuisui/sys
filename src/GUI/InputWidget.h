//
// Created by csh on 4/6/2022.
//

#ifndef SYS_INPUTWIDGET_H
#define SYS_INPUTWIDGET_H
#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QtWidgets/QtWidgets>

#include "NeuronInfo.h"
#include "SubwayMapWidget.h"

#define INPUT_WIDTH 500
#define INPUT_HEIGHT 600

class InputWidget:public QWidget{
    Q_OBJECT
public:
    explicit InputWidget(QWidget* parent = nullptr);

    void SWCLoaded(NeuronInfo* in_neuronInfo);

    void SetOtherWidget(SubwayMapWidget* in_subwayMapWidget);

private:
    void GenTreeWidget(QTreeWidgetItem* parent, Path* path);

signals:
    void ChangeSelectionStateSignal(SubwayMapWidget::SelectionState state);

public slots:
    void ChangeCurrentPoint(int pathid, int vertexid);
    void ChangeLastPoint(int pathid, int vertexid);
    void ChangeNextPoint(int pathid, int vertexid);

private slots:
    void ChangePath(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void GetButtonState(bool state);
    void InterpolateRadius();
    void EditRadius();
    void ReconstructionSlot(bool);
private:
    NeuronInfo* neuronInfo;

    QTreeWidget* treeWidget;
    std::vector<QTreeWidgetItem*> treeItems;

    SubwayMapWidget* subwayMapWidget;

    QLabel* currentPointLabel;
    QLabel* currentPointPosLine;
    QLabel* lastPointLabel;
    QLabel* lastPointPosLine;
    QLabel* lastPointRadiusLine;
    QPushButton* lastButton;
    QLabel* nextPointLabel;
    QLabel* nextPointPosLine;
    QLabel* nextPointRadiusLine;
    QPushButton* nextButton;

    QLineEdit* currentPointRadiusLine;

    QPushButton* interpolateButton;

    Vertex* currentPoint;
    int currentPointPathID;
    int currentPointVertexID;

    int lastPointid;
    int nextPointid;

    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* connectButton;

    QPushButton* submitButton;

    std::vector<QPushButton*> buttons;

};
#endif //SYS_INPUTWIDGET_H
