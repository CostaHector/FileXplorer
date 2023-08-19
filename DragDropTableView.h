#ifndef DRAGDROPTABLEVIEW_H
#define DRAGDROPTABLEVIEW_H

#include <QTableView>
#include <QFileSystemModel>

#include <QPushButton>

#include "MyQFileSystemModel.h"

class DragDropTableView : public QTableView
{
public:
    DragDropTableView(MyQFileSystemModel* fsmModel, QPushButton*mouseSideKeyBackwardBtn, QPushButton*mouseSideKeyForwardBtn);

    void mouseMoveEvent(QMouseEvent* event) override{
        //mouseMoveEventCore(event);//
        QTableView::mouseMoveEvent(event);
    }

    auto InitViewSettings()->void;

    virtual void mousePressEvent(QMouseEvent* event) override;

    void dragMoveEvent(QDragMoveEvent* event) override;

private:
    QPushButton* backwardBtn; // will not takeover
    QPushButton* forwardBtn;// will not takeover
};

constexpr int ROW_SECTION_HEIGHT = 10;
#endif // DRAGDROPTABLEVIEW_H
