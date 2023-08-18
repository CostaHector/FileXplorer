#include "DragDropTableView.h"

#include <QHeaderView>
#include <QMouseEvent>
#include "PublicVariable.h"
#include "MyQFileSystemModel.h"


void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order){
    PreferenceSettings().setValue("HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX", logicalIndex);
    PreferenceSettings().setValue("HEARVIEW_SORT_INDICATOR_ORDER", MyQFileSystemModel::SortOrderEnum2String(order));
}

void on_sectionResized(int logicalIndex, int /*oldSize*/, int newSize){
    if (logicalIndex == 0){
        PreferenceSettings().setValue("NAME_COLUMN_WIDTH", newSize);
    }
}


auto MouseSidekeyBackwardForwardCore(Qt::MouseButton mousebutton, QPushButton* backwardBtn, QPushButton* forwardBtn)->bool{
    if (mousebutton == Qt::MouseButton::BackButton){
        emit backwardBtn->clicked();
        return true;
    } else if (mousebutton == Qt::MouseButton::ForwardButton) {
        emit forwardBtn->clicked();
        return true;
    }
    return false;
}

DragDropTableView::DragDropTableView(QFileSystemModel* fsmModel, QPushButton*mouseSideKeyBackwardBtn, QPushButton*mouseSideKeyForwardBtn):
    backwardBtn(mouseSideKeyBackwardBtn),
    forwardBtn(mouseSideKeyForwardBtn)
{
    setModel(fsmModel);
    InitViewSettings();

    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
    setDragDropMode(QAbstractItemView::DragDrop);
    setAcceptDrops(true);
    setDragEnabled(true);
    setDropIndicatorShown(true);

    installEventFilter(this);

    connect(horizontalHeader(), &QHeaderView::sectionResized, on_sectionResized);
}

auto DragDropTableView::InitViewSettings()->void{
    setShowGrid(false);
    setAlternatingRowColors(true);
    setSortingEnabled(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(ROW_SECTION_HEIGHT);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    setColumnWidth(0, PreferenceSettings().value("NAME_COLUMN_WIDTH", 100).toInt());
    horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Interactive);
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter);
    connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, onSortIndicatorChanged);
}

void DragDropTableView::mousePressEvent(QMouseEvent* event){
    if (MouseSidekeyBackwardForwardCore(event->button(), backwardBtn, forwardBtn)){
        return;
    }
    return QTableView::mousePressEvent(event);
}
