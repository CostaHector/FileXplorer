#ifndef VIEWSTYLESHEET_H
#define VIEWSTYLESHEET_H

#include <QString>

namespace ViewStyleSheet {
constexpr int ROW_SECTION_HEIGHT = 10;

const QString LISTVIEW_STYLESHEET =
    "QListView {"
    "    show-decoration-selected: 1;"
    "}"
    "QListView::item:alternate {"
    "}"
    "QListView::item:selected {"
    "    border-bottom: 1px inherit #FFFFFF;"
    "}"
    "QListView::item:selected:!active {"
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EEEEEE, stop: 1 #999999);"
    "    color: #000000;"
    "}"
    "QListView::item:selected:active {"
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #99D1FF, stop: 1 #99D1FF);"
    "    color: #000000;"
    "    border-top: 2px solid #CCEBFF;"
    "    border-bottom: 2px solid #CCEBFF;"
    "}"
    "QListView::item:hover {"
    "    background: #CCEBFF;"
    "}";

const QString TABLEVIEW_STYLESHEET =
    "QTableView {"
    "    show-decoration-selected: 1;"
    "}"
    "QTableView::item:alternate {"
    "}"
    "QTableView::item:selected {"
    "    border-bottom: 1px inherit #FFFFFF;"
    "}"
    "QTableView::item:selected:!active {"
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EEEEEE, stop: 1 #999999);"
    "    color: #000000;"
    "}"
    "QTableView::item:selected:active {"
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #99D1FF, stop: 1 #99D1FF);"
    "    color: #000000;"
    "    border-top: 2px solid #CCEBFF;"
    "    border-bottom: 2px solid #CCEBFF;"
    "}"
    "QTableView::item:hover {"
    "    background: #CCEBFF;"
    "}";

const QString TREEVIEW_STYLESHEET =
    "QTreeView {"
    "    show-decoration-selected: 1;"
    "}"
    "QTreeView::item:alternate {"
    "}"
    "QTreeView::item:selected {"
    "    border-bottom: 1px inherit #FFFFFF;"
    "}"
    "QTreeView::item:selected:!active {"
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EEEEEE, stop: 1 #999999);"
    "    color: #000000;"
    "}"
    "QTreeView::item:selected:active {"
    "    background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #99D1FF, stop: 1 #99D1FF);"
    "    color: #000000;"
    "    border-top: 2px solid #CCEBFF;"
    "    border-bottom: 2px solid #CCEBFF;"
    "}"
    "QTreeView::item:hover {"
    "    background: #CCEBFF;"
    "}";

}  // namespace ViewStyleSheet

#endif  // VIEWSTYLESHEET_H
