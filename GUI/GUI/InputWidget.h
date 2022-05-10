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
#include "tf1deditor.h"
#include "TrivalVolume.hpp"

#include <VolumeSlicer/volume.hpp>
#include <VolumeSlicer/render.hpp>
#include <VolumeSlicer/slice.hpp>

#define INPUT_WIDTH 500
#define INPUT_HEIGHT 1000

using namespace vs;

class InputWidget:public QWidget{
    Q_OBJECT
public:
    explicit InputWidget(QWidget* parent = nullptr);

    void SWCLoaded(NeuronInfo* in_neuronInfo);

    void SetOtherWidget(SubwayMapWidget* in_subwayMapWidget);

    TF1DEditor* tf_editor_widget;
private:
    void GenTreeWidget(QTreeWidgetItem* parent, Path* path);

signals:
    void ChangeSelectionStateSignal(SubwayMapWidget::SelectionState state);
    void ChangeRenderOptionSignal(bool vol, bool obj, bool line);
    void ReloadObj(std::string);

public slots:
    void ChangeCurrentPoint(int id);
    void ChangeLastPoint(int id);
    void ChangeNextPoint(int id);

private slots:
    void ChangePath(QTreeWidgetItem* current, QTreeWidgetItem* previous);
    void GetButtonState(bool state);
    void GetRenderButtonState(bool state);
    void InterpolateRadius();
    void EditRadius();
    void ReconstructionSlot(bool);
private:
    NeuronInfo* neuronInfo;

    QCheckBox *ifRenderVolume,*ifRenderObj,*ifRenderLine;

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

    int lastPointid;
    int nextPointid;

    QPushButton* addButton;
    QPushButton* deleteButton;
//    QPushButton* connectButton;

    QPushButton* submitButton;

    std::vector<QPushButton*> buttons;

    std::shared_ptr<RawVolume> raw_volume;
    std::unique_ptr<TrivalVolume> trival_volume;
    std::vector<float> tf;

};
#endif //SYS_INPUTWIDGET_H
