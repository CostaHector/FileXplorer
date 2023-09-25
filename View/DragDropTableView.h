#ifndef DRAGDROPTABLEVIEW_H
#define DRAGDROPTABLEVIEW_H

#include <QTableView>
#include <QFileSystemModel>

#include <QPushButton>
#include <QListView>

#include "MyQFileSystemModel.h"
#include "PublicVariable.h"

#include <QDragMoveEvent>
#include <QMimeData>
#include <QToolTip>
#include <QDrag>
#include <QPainter>

constexpr int ROW_SECTION_HEIGHT = 10;
const QPoint TOOLTIP_MSG_PNG_DEV(70, 0);
const QString TOOLTIP_MSG_LINK("LINK:<font color=\"gray\" size=\"14\">%1</font>");
const QString TOOLTIP_MSG_CP("CP:<font color=\"gray\" size=\"14\">%1</font>");
const QString TOOLTIP_MSG_MV("MV:<font color=\"gray\" size=\"14\">%1</font>");

class View{
public:
    virtual void subscribe() = 0;

    virtual void InitViewSettings() = 0;

    virtual void SetViewColumnWidth() = 0;

    virtual void UpdateItemViewFontSize() = 0;

    static void on_sectionResized(const int logicalIndex, const int /* oldSize */, const int newSize){
        if (logicalIndex == MainKey::NAME_COLUMN){
            PreferenceSettings().setValue(MemoryKey::NAME_COLUMN_WIDTH.name, newSize);
        }
    }

    static void onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order){
        PreferenceSettings().setValue(MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.name, logicalIndex);
        PreferenceSettings().setValue(MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.name, HEADERVIEW_SORT_INDICATOR_ORDER::SortOrderEnum2String(order));

        qDebug("sort logicalIndex [%d]", PreferenceSettings().value(MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.name, MemoryKey::HEARVIEW_SORT_INDICATOR_LOGICAL_INDEX.v).toInt());
        qDebug("order [%s]", PreferenceSettings().value(MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.name, MemoryKey::HEARVIEW_SORT_INDICATOR_ORDER.v).toString().toStdString().c_str());
    }

    static auto onMouseSidekeyBackwardForward(Qt::MouseButton mousebutton, QPushButton* backwardBtn, QPushButton* forwardBtn)->bool{
        switch (mousebutton) {
        case Qt::MouseButton::BackButton:
            if (backwardBtn){
                emit backwardBtn->clicked();
            }
            return true;
        case Qt::MouseButton::ForwardButton:
            if (forwardBtn){
                emit forwardBtn->clicked();
            }
            return true;
        default:
            return false;
        }
    }

    static void UpdateItemViewFontSizeCore(QAbstractItemView* view){
        if (view == nullptr){
            qDebug("UpdateItemViewFontSizeCore view* pointer is nullptr");
            return;
        }
        const auto fontSize = PreferenceSettings().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
        QFont defaultFont(view->font());
        defaultFont.setPointSize(fontSize);
        view->setFont(defaultFont);
    }

    static QModelIndexList selectedIndexes(QAbstractItemView* view){  // ignore other column, keep the first column
        if (qobject_cast<QListView*>(view) != nullptr){ // TODO check here
            return view->selectionModel()->selectedIndexes();
        }
        return view->selectionModel()->selectedRows();
    }

    static void changeDropAction(QDropEvent* event, QPoint pnt, const QString& name, QWidget* w){
        if (event->keyboardModifiers().testFlag(Qt::AltModifier)){
            event->setDropAction(Qt::DropAction::LinkAction);
            QToolTip::showText(pnt, TOOLTIP_MSG_LINK.arg(name), w);
        } else if (event->keyboardModifiers() & Qt::ControlModifier){
            event->setDropAction(Qt::DropAction::CopyAction);
            QToolTip::showText(pnt, TOOLTIP_MSG_CP.arg(name), w);
        } else {
            event->setDropAction(Qt::DropAction::MoveAction);
            QToolTip::showText(pnt, TOOLTIP_MSG_MV.arg(name), w, QRect());
        }
    }

    static void dropEventCore(QAbstractItemView* view, QDropEvent* event){
        auto * _model = dynamic_cast<MyQFileSystemModel* > (view->model());
        if (_model == nullptr){
            qDebug("_model is nullptr");
            return;
        }
        const QPoint& pnt = event->pos();
        const QModelIndex& ind = view->indexAt(pnt);
        if (_model->rootPath().isEmpty()){
            qDebug("Ignore. You cannot drag move/drag enter/drop into the path[%s]", _model->rootPath().toStdString().c_str());
            return;
        }
        if (not (_model->flags(ind).testFlag(Qt::ItemIsDropEnabled))){
            qDebug("Ignore. Flags(ind)[%d] NOT Meet [%d].", int(_model->flags(ind)), int(Qt::ItemIsDropEnabled));
            return;
        }
        QPoint tooltipPosition = view->mapToGlobal(event->pos() + TOOLTIP_MSG_PNG_DEV);
        QString destinationPath;
        if (ind.isValid()){  // drop into the rootpath of current view model
            destinationPath = _model->fileInfo(ind).fileName();
        } else{  // drop into the rootpath of current view model
            destinationPath = _model->rootPath();
        }
        View::changeDropAction(event, tooltipPosition, destinationPath, view);
    }

    static void dragEnterEventCore(QAbstractItemView* view, QDragEnterEvent* event){
        auto * _model = dynamic_cast<MyQFileSystemModel* > (view->model());
        if (_model == nullptr){
            qDebug("_model is nullptr");
            return;
        }
        const QPoint& pnt = event->pos();
        const QModelIndex& ind = view->indexAt(pnt);
        if (_model->rootPath().isEmpty()){
            qDebug("Ignore. You cannot drag move/drag enter/drop into the path[%s]", _model->rootPath().toStdString().c_str());
            return;
        }
        if (not (_model->flags(ind).testFlag(Qt::ItemIsDropEnabled))){
            qDebug("Ignore. Flags(ind)[%d] NOT Meet [%d].", int(_model->flags(ind)), int(Qt::ItemIsDropEnabled));
            return;
        }
        QPoint tooltipPosition = view->mapToGlobal(event->pos() + TOOLTIP_MSG_PNG_DEV);
        QString destinationPath;
        if (ind.isValid()){  // drop into the rootpath of current view model
            destinationPath = _model->fileInfo(ind).fileName();
        } else{  // drop into the rootpath of current view model
            destinationPath = _model->rootPath();
        }
        View::changeDropAction(event, tooltipPosition, destinationPath, view);
    }

        static  void dragMoveEventCore(QAbstractItemView* view, QDragMoveEvent* event){
            auto * _model = dynamic_cast<MyQFileSystemModel* > (view->model());
            if (_model == nullptr){
                qDebug("_model is nullptr");
                return;
            }
            const QPoint& pnt = event->pos();
            const QModelIndex& ind = view->indexAt(pnt);
            if (_model->rootPath().isEmpty()){
                qDebug("Ignore. You cannot drag move/drag enter/drop into the path[%s]", _model->rootPath().toStdString().c_str());
                return;
            }
            if (not (_model->flags(ind).testFlag(Qt::ItemIsDropEnabled))){
                qDebug("Ignore. Flags(ind)[%d] NOT Meet [%d].", int(_model->flags(ind)), int(Qt::ItemIsDropEnabled));
                return;
            }
            QPoint tooltipPosition = view->mapToGlobal(event->pos() + TOOLTIP_MSG_PNG_DEV);
            QString destinationPath;
            if (ind.isValid()){  // drop into the rootpath of current view model
                destinationPath = _model->fileInfo(ind).fileName();
            } else{  // drop into the rootpath of current view model
                destinationPath = _model->rootPath();
            }
            View::changeDropAction(event, tooltipPosition, destinationPath, view);

            event->accept();
            qDebug("\tdragMoveEvent [%d]", int(event->dropAction()));
        }

        static QPixmap PaintDraggedFilesFolders(const int selectedCnt){
            QPixmap folderPixmap = QPixmap(":/themes/DRAG_FOLDERS").scaled(QSize(64, 64));
            QPixmap filesPixmap = QPixmap(":/themes/DRAG_FILES").scaled(QSize(64, 64));

            QPainter painter(&folderPixmap); // TODO Visual Experience not work
            painter.drawPixmap(QPoint(8, 8), filesPixmap);

            if (selectedCnt > 1){
                QFont font;
                font.setPointSize(18);
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(QRect(0, 0, 64, 64), Qt::AlignHCenter | Qt::AlignVCenter, QString::number(selectedCnt));
                painter.end();
            }
            return folderPixmap;
        }

        static void mouseMoveEventCore(QAbstractItemView* view, QMouseEvent* event){
            if (event->buttons() != Qt::MouseButton::LeftButton){
                return;
            }
//            if isinstance(view->model(), QSqlTableModel): // TODOFunction
//                return
            QModelIndexList mixed = View::selectedIndexes(view);
            int nCnt = mixed.size();
            if (mixed.isEmpty()){
                event->ignore();
                return;
            }

            QMimeData* mime = new QMimeData;
            auto * _model = dynamic_cast<MyQFileSystemModel* > (view->model());
            if (_model == nullptr){
                qDebug("_model is nullptr");
                return;
            }
            if (_model->rootPath().isEmpty()){
                qDebug("Ignore. You cannot mouse move into the path[%s]", _model->rootPath().toStdString().c_str());
                return;
            }

            QList<QUrl> localFilesLst;
            for (const auto& ind: mixed){
                localFilesLst.append(QUrl::fromLocalFile(_model->filePath(ind)));
            }
            mime->setUrls(localFilesLst);

            QDrag drag(view);
            drag.setMimeData(mime);

            QPixmap folderPixmap = View::PaintDraggedFilesFolders(nCnt);
            drag.setPixmap(folderPixmap);
            drag.setHotSpot(folderPixmap.rect().center());
            drag.exec(Qt::DropAction::LinkAction | Qt::DropAction::CopyAction | Qt::DropAction::MoveAction);
        }


};

class DragDropTableView : public QTableView, public View
{
public:
    DragDropTableView(MyQFileSystemModel* fsmModel, QPushButton* mouseSideKeyBackwardBtn, QPushButton* mouseSideKeyForwardBtn);

    void subscribe() override;

    auto InitViewSettings()->void override;
    auto SetViewColumnWidth()->void override;
    auto UpdateItemViewFontSize()->void override;

    void dropEvent(QDropEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;

    void dragMoveEvent(QDragMoveEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;

    void mouseMoveEvent(QMouseEvent* event) override;
private:
    QPushButton* backwardBtn; // will not takeover
    QPushButton* forwardBtn;// will not takeover
};

#endif // DRAGDROPTABLEVIEW_H
