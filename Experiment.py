import sys
from PySide2.QtWidgets import QWidget, QTableView, QMainWindow, QApplication, QToolBar, QLabel, QAction, QActionGroup, QLineEdit, QStyleOptionViewItem, QStyledItemDelegate
from PySide2.QtCore import Qt, QObject, QFile, QFileInfo, QAbstractTableModel, QModelIndex, QDirIterator, QDir
from PySide2.QtGui import QPixmap

from dataclasses import dataclass

@dataclass
class SCENE_INFO:
    filename: str
    size: int
    like: int

    def __less__(self, rhs) -> int:
        if self.filename > rhs.filename:
            return 1
        elif self.filename < rhs.filename:
            return -1
        else:
            return 0


class ScenesTableModel(QAbstractTableModel):
    def __init__(self, object: QObject = None):
        super().__init__(object)
        self.mPageIndex: int = -1
        self.mSCENES_CNT_ROW: int = -1
        self.mEntryList: list[SCENE_INFO] = [SCENE_INFO("A DSDIT: PEERTYPE=xx, PEERID=xx;", 20, 100),
                                                     SCENE_INFO("B LSF:;", 19, 32),
                                                     SCENE_INFO("C LSP:;", 18, 72),
                                                     SCENE_INFO("D LS", 17, 84),
                                                     SCENE_INFO("E LS;", 16, 95),
                                                     SCENE_INFO("F LS:;", 15, 71),
                                                     SCENE_INFO("G LSVAUDIT:;", 14, 97),
                                                     SCENE_INFO("H LSP:;", 13, 100),
                                                     SCENE_INFO("I LS", 12, 95),
                                                     SCENE_INFO("J LS;", 11, 99),
                                                     SCENE_INFO("K LS:;", 10, 95),
                                                     SCENE_INFO("L neme=stDAme", 9, 0),
                                                     SCENE_INFO("M PPbiDARBa", 8, 10),
                                                     ]
        self.mEntryListFiltered: list[SCENE_INFO] = list()
        self.mEntryListFiltered += self.mEntryList
        self.mEntryListCur: list[SCENE_INFO] = list()
        self.mEntryListCur += self.mEntryList
        # QList<INFO>::iterator mCurBegin{nullptr}, mCurEnd{nullptr};
        # mCurBegin = mEntryList.cbegin();
        # mCurEnd = mEntryList.cend();
        self.mPattern: str = ""
        self.mSCENES_CNT_COLUMN = 4
        self.mFilterEnable = False

    def rowCount(self, parent: QModelIndex = QModelIndex()) -> int:
        lst: list[SCENE_INFO] = self.GetEntryList()
        begin, end = self.GetEntryIndexBE(len(lst))
        scenesCnt = end - begin
        return scenesCnt // self.mSCENES_CNT_COLUMN + int(scenesCnt % self.mSCENES_CNT_COLUMN != 0)

    def columnCount(self, parent: QModelIndex = QModelIndex()) -> int:
        return self.mSCENES_CNT_COLUMN

    def data(self, index: QModelIndex, role: Qt.ItemDataRole = Qt.DisplayRole):
        if not index.isValid() or index.row() >= self.rowCount() or index.column() >= self.columnCount():
            return None
        lst: list[SCENE_INFO] = self.mEntryListCur
        linearInd: int = index.row() * self.mSCENES_CNT_COLUMN + index.column()
        if linearInd >= len(lst):
            return None

        """
            if (mCurBegin + linearInd >= mCurEnd) return {};
            if (role == Qt::ItemDataRole::DisplayRole) {
                return mCurBegin[linearInd].filename;
            }
        """

        if role == Qt.ItemDataRole.DisplayRole:
            return "%s\n%d\n%03d" % (lst[linearInd].filename, lst[linearInd].size, lst[linearInd].like)
        if role == Qt.ItemDataRole.DecorationRole:
            pm = QPixmap("WinWordLogo.scale-180.png")
            return pm.scaledToWidth(180)
        return None

    def headerData(self, section: int, orientation: Qt.Orientation, role: Qt.ItemDataRole = Qt.DisplayRole):
        if role == Qt.TextAlignmentRole:
            if orientation == Qt.Vertical:
                return Qt.AlignRight
        if role == Qt.DisplayRole:
            if orientation == Qt.Orientation.Horizontal:
                return section + 1
            return section + 1
        return super().headerData(section, orientation, role)

    def RowsCountStartChange(self, beforeRow: int, afterRow: int):
        if beforeRow < afterRow:
            self.beginInsertRows(QModelIndex(), beforeRow, afterRow - 1)
        elif beforeRow > afterRow:
            self.beginRemoveRows(QModelIndex(), afterRow, beforeRow - 1)
        else:
            return

    def RowsCountEndChange(self, beforeRow: int, afterRow: int):
        if beforeRow < afterRow:
            self.endInsertRows()
        elif beforeRow > afterRow:
            self.endRemoveRows()
        else:
            self.dataChanged.emit(self.index(0, 0), self.index(afterRow - 1, self.columnCount() - 1), [])

    def ColumnsBeginChange(self, beforeColumnCnt: int, afterColumnCnt: int):
        if beforeColumnCnt < afterColumnCnt:
            self.beginInsertColumns(QModelIndex(), beforeColumnCnt, afterColumnCnt - 1)
        elif beforeColumnCnt > afterColumnCnt:
            self.beginRemoveColumns(QModelIndex(), afterColumnCnt, beforeColumnCnt - 1)
        else:
            return

    def ColumnsEndChange(self, beforeColumnCnt: int, afterColumnCnt: int):
        if beforeColumnCnt < afterColumnCnt:
            self.endInsertColumns()
        elif beforeColumnCnt > afterColumnCnt:
            self.endRemoveColumns()
        else:
            self.dataChanged.emit(self.index(0, 0), self.index(self.rowCount() - 1, afterColumnCnt - 1), [])

    def setRootPath(self, path: str):
        it = QDirIterator(path, "*", QDir.Filter.AllEntries | QDir.Filter.NoDotAndDotDot, QDirIterator.IteratorFlag.Subdirectories)
        newEntryList: list[SCENE_INFO] = list()
        newFilteredList: list[SCENE_INFO] = list()
        while it.hasNext():
            it.next()
            fi: QFileInfo = it.fileInfo()
            newEntryList.append(SCENE_INFO(fi.fileName(), fi.size(), fi.size()))
            if self.mFilterEnable:
                if self.mPattern in fi.fileName():
                    newFilteredList.append(newEntryList[-1])
        print("new path[%s], element[%d], filtered[%d]" % (path, len(newEntryList), len(newFilteredList)))

        """if mFilterEnable: also update mEntryListFiltered, and set mCurBegin in mEntryListFiltered instead of mEntryList"""

        ELE_N = len(newFilteredList) if self.mFilterEnable else len(newEntryList)
        newBegin, newEnd = self.GetEntryIndexBE(ELE_N)
        newScenesCnt = newEnd - newBegin

        beforeRow: int = self.rowCount()
        afterRow: int = newScenesCnt // self.mSCENES_CNT_COLUMN + int(newScenesCnt % self.mSCENES_CNT_COLUMN != 0)
        if self.mFilterEnable:
            print("filtered[%s] element: %d->%d, RowCountChanged: %d->%d" % (self.mPattern, len(self.mEntryListFiltered), len(newFilteredList), beforeRow, afterRow))
        else:
            print("entry element: %d->%d, RowCountChanged: %d->%d" % (len(self.mEntryList), len(newEntryList), beforeRow, afterRow))
        self.RowsCountStartChange(beforeRow, afterRow)

        self.mEntryList.clear()
        self.mEntryList += newEntryList

        self.mEntryListFiltered.clear()
        self.mEntryListFiltered += newFilteredList

        self.mEntryListCur.clear()
        self.mEntryListCur += self.mEntryListFiltered[newBegin:newEnd] if self.mFilterEnable else self.mEntryList[newBegin:newEnd]
        """
        if (self.mFilterEnable) {
            mCurBegin = mEntryListFiltered.cbegin() + newBegin;
            mCurEnd = mEntryListFiltered.cbegin() + newEnd;
        } else {
            mCurBegin = mEntryList.cbegin() + newBegin;
            mCurEnd = mEntryList.cbegin() + newEnd;
        }
        """
        self.RowsCountEndChange(beforeRow, afterRow)

    def ChangeColumnsCnt(self, newColumnCnt: int = 5, newPageIndex: int = -1) -> bool:
        if newColumnCnt <= 0 or newPageIndex < -1:
            return True
        lst: list[SCENE_INFO] = self.GetEntryList()
        TOTAL_N = self.GetEntryListLen()
        begin: int = 0
        end: int = 0
        if self.mSCENES_CNT_ROW == -1:
            print("won't display by group")
            beforeRowCnt: int = self.rowCount()
            afterRowCnt: int = TOTAL_N // newColumnCnt + int(TOTAL_N % newColumnCnt != 0)
            beforeColumnCnt: int = self.columnCount()
            afterColumnCnt: int = newColumnCnt
            begin = 0
            end = TOTAL_N
        else:
            begin = min(self.mSCENES_CNT_ROW * newColumnCnt * self.mPageIndex, TOTAL_N)
            end = min(self.mSCENES_CNT_ROW * newColumnCnt * (self.mPageIndex + 1), TOTAL_N)
            TOTAL_PAGE_IND = end - begin

            beforeRowCnt: int = self.rowCount()
            afterRowCnt: int = self.mSCENES_CNT_ROW
            beforeColumnCnt: int = self.columnCount()
            afterColumnCnt: int = TOTAL_PAGE_IND // self.mSCENES_CNT_ROW + int(TOTAL_PAGE_IND % self.mSCENES_CNT_ROW != 0)
        print("ChangeColumnsCnt. columnCnt: %d->%d" % (beforeColumnCnt, afterColumnCnt))
        print("ChangeColumnsCnt. rowCnt: %d->%d" % (beforeRowCnt, afterRowCnt))

        self.ColumnsBeginChange(beforeColumnCnt, afterColumnCnt)
        self.RowsCountStartChange(beforeRowCnt, afterRowCnt)

        self.mSCENES_CNT_COLUMN = newColumnCnt
        self.mPageIndex = newPageIndex
        self.mEntryListCur = lst[begin:end]
        """
            mCurBegin = lst.cbegin() + begin;
            mCurEnd = lst.cbegin() + end;
        """
        self.RowsCountEndChange(beforeRowCnt, afterRowCnt)
        self.ColumnsEndChange(beforeColumnCnt, afterColumnCnt)

        print("==============dimension: %dx%d==============" % (self.rowCount(), self.columnCount()))
        return True

    def SortOrder(self, reverse: bool = False):
        lst: list[SCENE_INFO] = self.GetEntryList()
        lst.sort(key=lambda v: v.filename, reverse=reverse)
        self.dataChanged.emit(self.index(0, 0), self.index(self.rowCount() - 1, self.columnCount() - 1), [])

    def ChangeRowsCnt(self, newRowCnt: int = -1, newPageIndex: int = -1) -> bool:
        if newRowCnt <= 0 or newPageIndex == -1:
            return True

        lst: list[SCENE_INFO] = self.GetEntryList()
        TOTAL_N = len(lst)
        begin = min(self.mSCENES_CNT_COLUMN * newRowCnt * newPageIndex, TOTAL_N)
        end = min(self.mSCENES_CNT_COLUMN * newRowCnt * (newPageIndex + 1), TOTAL_N)
        TOTAL_PAGE_IND = end - begin

        beforeRowCnt: int = self.rowCount()
        afterRowCnt: int = TOTAL_PAGE_IND // self.mSCENES_CNT_COLUMN + int(TOTAL_PAGE_IND % self.mSCENES_CNT_COLUMN != 0)
        beforeColumnCnt: int = self.columnCount()
        afterColumnCnt: int = self.mSCENES_CNT_COLUMN
        print("Change to page %d" % newPageIndex)
        print("ChangeRowsCnt. columnCnt: %d->%d" % (beforeColumnCnt, afterColumnCnt))
        print("ChangeRowsCnt. rowCnt: %d->%d" % (beforeRowCnt, afterRowCnt))
        self.ColumnsBeginChange(beforeColumnCnt, afterColumnCnt)
        self.RowsCountStartChange(beforeRowCnt, afterRowCnt)

        self.mSCENES_CNT_ROW = newRowCnt
        self.mPageIndex = newPageIndex
        self.mEntryListCur = lst[begin:end]
        """
            mCurBegin = lst.cbegin() + begin;
            mCurEnd = lst.cbegin() + end;
        """
        self.RowsCountEndChange(beforeRowCnt, afterRowCnt)
        self.ColumnsEndChange(beforeColumnCnt, afterColumnCnt)

        print("==============dimension: %dx%d==============" % (self.rowCount(), self.columnCount()))
        return True

    def SetPageIndex(self, newPageIndex: int = -1) -> bool:
        if newPageIndex == -1:
            return False
        if self.mSCENES_CNT_ROW == -1:
            print("not display by page")
            return False
        lst: list[SCENE_INFO] = self.GetEntryList()
        TOTAL_N = len(lst)
        begin = min(self.mSCENES_CNT_COLUMN * self.mSCENES_CNT_ROW * newPageIndex, TOTAL_N)
        end = min(self.mSCENES_CNT_COLUMN * self.mSCENES_CNT_ROW * (newPageIndex + 1), TOTAL_N)
        TOTAL_PAGE_IND = end - begin

        beforeRowCnt: int = self.rowCount()
        afterRowCnt: int = TOTAL_PAGE_IND // self.mSCENES_CNT_COLUMN + int(TOTAL_PAGE_IND % self.mSCENES_CNT_COLUMN != 0)
        print("SetPageIndex, rowCnt:%d->%d" % (beforeRowCnt, afterRowCnt))
        self.RowsCountStartChange(beforeRowCnt, afterRowCnt)
        self.mPageIndex = newPageIndex
        self.mEntryListCur = lst[begin:end]
        """
            mCurBegin = lst.cbegin() + begin;
            mCurEnd = lst.cbegin() + end;
        """
        self.RowsCountEndChange(beforeRowCnt, afterRowCnt)

        print("==============dimension: %dx%d==============" % (self.rowCount(), self.columnCount()))
        return True

    def GetEntryIndexBE(self, maxLen: int) -> tuple[int, int]:
        if self.mPageIndex == -1 or self.mSCENES_CNT_ROW == -1:
            return 0, maxLen

        begin = self.mSCENES_CNT_COLUMN * self.mSCENES_CNT_ROW * self.mPageIndex
        end = self.mSCENES_CNT_COLUMN * self.mSCENES_CNT_ROW * (self.mPageIndex + 1)
        return min(begin, maxLen), min(end, maxLen)

    def GetPageCnt(self) -> int:
        if self.mSCENES_CNT_ROW == -1:
            return 1
        N = self.GetEntryListLen()
        return N // (self.mSCENES_CNT_ROW * self.mSCENES_CNT_COLUMN) + int(N % (self.mSCENES_CNT_ROW * self.mSCENES_CNT_COLUMN) != 0)

    def GetEntryList(self) -> list[SCENE_INFO]:
        if self.mFilterEnable:
            return self.mEntryListFiltered
        else:
            return self.mEntryList

    def GetEntryListLen(self) -> int:
        return len(self.GetEntryList())

    def setFilterRegularExpression(self, pattern: str):
        self.mPattern = pattern
        if not pattern:
            print("filter now disabled", pattern)
            newBegin, newEnd = self.GetEntryIndexBE(len(self.mEntryList))
            newScenesCnt = newEnd - newBegin
            beforeRow: int = self.rowCount()
            afterRow: int = newScenesCnt // self.mSCENES_CNT_COLUMN + int(newScenesCnt % self.mSCENES_CNT_COLUMN != 0)
            self.RowsCountStartChange(beforeRow, afterRow)
            self.mFilterEnable = False
            self.mEntryListCur.clear()
            self.mEntryListCur = self.mEntryList[newBegin:newEnd]
            """
                mCurBegin = mEntryList.cbegin() + newBegin;
                mCurEnd = mEntryList.cbegin() + newEnd;
            """
            self.RowsCountEndChange(beforeRow, afterRow)
            print("==============dimension: %dx%d==============" % (self.rowCount(), self.columnCount()))
            return

        print("filter now enable pattern", pattern)
        newCurrentList: list[SCENE_INFO] = list()
        for items in self.mEntryList:
            if pattern in items.filename:
                newCurrentList.append(items)

        newBegin, newEnd = self.GetEntryIndexBE(len(newCurrentList))
        newScenesCnt = newEnd - newBegin

        beforeRow: int = self.rowCount()
        afterRow: int = newScenesCnt // self.mSCENES_CNT_COLUMN + int(newScenesCnt % self.mSCENES_CNT_COLUMN != 0)
        print("setRootPath. element: %d->%d, RowCountChanged: %d->%d" % (len(self.mEntryListFiltered), len(newCurrentList), beforeRow, afterRow))
        self.RowsCountStartChange(beforeRow, afterRow)
        self.mEntryListFiltered.clear()
        self.mEntryListFiltered += newCurrentList
        self.mEntryListCur.clear()
        self.mEntryListCur += self.mEntryListFiltered[newBegin:newEnd]
        """
            mCurBegin = mEntryListFiltered.cbegin() + newBegin;
            mCurEnd = mEntryListFiltered.cbegin() + newEnd;
        """
        self.mFilterEnable = True
        self.RowsCountEndChange(beforeRow, afterRow)


class SceneSortProxy(QSortFilterProxyModel):
    def __init__(self, parent: QObject = None):
        super().__init__(parent)


class AlignDelegate(QStyledItemDelegate):
    def initStyleOption(self, option: QStyleOptionViewItem, index: QModelIndex) -> None:
        super().initStyleOption(option, index)
        option.decorationPosition = QStyleOptionViewItem.Position.Bottom


class SceneTable(QTableView):
    def __init__(self, sceneModel: ScenesTableModel, parent: QWidget = None):
        super().__init__(parent)

        self.model = sceneModel
        self.setModel(self.model)

        self.horizontalHeader().setDefaultSectionSize(256)
        self.horizontalHeader().setVisible(True)
        self.verticalHeader().setDefaultSectionSize(128)
        self.horizontalHeader().setStretchLastSection(False)

        delegate = AlignDelegate(self)
        self.setItemDelegate(delegate)


class BearingWidget(QMainWindow):
    def __init__(self, parent: QWidget = None):
        super(BearingWidget, self).__init__()
        self.setWindowTitle("BearingWidget")
        self.setMinimumSize(1780, 768)

        self.ascending = QAction("Ascending", self)
        self.ascending.setCheckable(True)
        self.descending = QAction("Descending", self)
        self.descending.setCheckable(True)
        self.orderAG = QActionGroup(self)
        self.orderAG.addAction(self.ascending)
        self.orderAG.addAction(self.descending)
        self.orderAG.setExclusionPolicy(QActionGroup.ExclusionPolicy.ExclusiveOptional)

        self.groupByPageAction = QAction("Rows:", self)
        self.groupByPageAction.setCheckable(True)
        self.maxScenesPerColumnInput = QLineEdit("5", self)
        self.theLastPage = QAction("The Last>>", self)
        self.lastPage = QAction("Last<", self)
        self.nextPage = QAction("Next>", self)
        self.theFirstPage = QAction("The First<<", self)
        self.pageIndexInput = QLineEdit("0", self)

        self.maxScenesPerRowInput = QLineEdit("5", self)

        self.filterStrInput = QLineEdit("", self)
        self.newPathInput = QLineEdit("", self)

        self.pagesSelectTB = QToolBar("Page Select", self)
        self.pagesSelectTB.addWidget(self.maxScenesPerColumnInput)
        self.pagesSelectTB.addActions([self.theFirstPage, self.lastPage])
        self.pagesSelectTB.addSeparator()
        self.pagesSelectTB.addWidget(self.pageIndexInput)
        self.pagesSelectTB.addSeparator()
        self.pagesSelectTB.addActions([self.nextPage, self.theLastPage])

        self.sceneTB = QToolBar("scene toolbar", self)
        self.sceneTB.addActions(self.orderAG.actions())
        self.sceneTB.addSeparator()
        self.sceneTB.addAction(self.groupByPageAction)
        self.sceneTB.addWidget(self.pagesSelectTB)
        self.sceneTB.addSeparator()
        self.sceneTB.addWidget(QLabel("Columns:", self))
        self.sceneTB.addWidget(self.maxScenesPerRowInput)
        self.sceneTB.addSeparator()
        self.sceneTB.addWidget(self.filterStrInput)
        self.sceneTB.addWidget(self.newPathInput)
        self.addToolBar(Qt.ToolBarArea.TopToolBarArea, self.sceneTB)

        self.model = ScenesTableModel()
        self.tb = SceneTable(self.model, self)
        self.setCentralWidget(self.tb)

        self.orderAG.triggered[QAction].connect(self.SortIt)

        self.groupByPageAction.triggered.connect(self.SetScenesGroupByPage)
        self.maxScenesPerColumnInput.textChanged.connect(self.SetScenesPerColumn)
        self.pageIndexInput.textChanged.connect(self.SetPageIndex)
        self.maxScenesPerRowInput.textChanged.connect(self.SetScenesPerRow)
        self.filterStrInput.textChanged.connect(self.model.setFilterRegularExpression)
        self.newPathInput.returnPressed.connect(lambda: self.IntoANewPath(self.newPathInput.text()))

        self.pagesSelectTB.actionTriggered[QAction].connect(self.PageIndexIncDec)
        self.groupByPageAction.setChecked(False)
        self.groupByPageAction.triggered[bool].emit(False)

        self.IntoANewPath(r"D:\Downloads")

    def PageIndexIncDec(self, pageAct: QAction):
        beforeIndexStr: str = self.pageIndexInput.text()
        if not beforeIndexStr.isnumeric():
            print("Error before index")
            return False
        beforePageInd: int = int(beforeIndexStr)
        dstPageInd = beforePageInd
        maxPage = self.model.GetPageCnt()
        if pageAct == self.nextPage:
            dstPageInd += 1
        elif pageAct == self.lastPage:
            dstPageInd -= 1
        elif pageAct == self.theFirstPage:
            dstPageInd = 0
        elif pageAct == self.theLastPage:
            dstPageInd = self.model.GetPageCnt() - 1
        else:
            print("nothing triggered")
            return False
        dstPageInd %= maxPage
        if dstPageInd == beforePageInd:
            print("Page remains %d, ignore switch page" % beforePageInd)
            return True
        print("page index changed: %d->%d" % (beforePageInd, dstPageInd))
        self.pageIndexInput.setText(str(dstPageInd))
        return True

    def SetScenesGroupByPage(self, groupByPageAction: bool):
        self.pagesSelectTB.setEnabled(groupByPageAction)
        if groupByPageAction:
            self.SetScenesPerColumn()

    def SetPageIndex(self):
        pageIndStr = self.pageIndexInput.text()
        if not pageIndStr.isnumeric():
            return False
        self.model.SetPageIndex(int(pageIndStr))

    def SetScenesPerColumn(self):
        rowCntStr = self.maxScenesPerColumnInput.text()
        pageIndStr = self.pageIndexInput.text()
        if not rowCntStr.isnumeric():
            return False
        if not pageIndStr.isnumeric():
            return False
        self.model.ChangeRowsCnt(int(rowCntStr), int(pageIndStr))

    def SetScenesPerRow(self):
        columnCntStr = self.maxScenesPerRowInput.text()
        pageIndStr = self.pageIndexInput.text()
        if not columnCntStr.isnumeric():
            return False
        if not pageIndStr.isnumeric():
            return False
        self.model.ChangeColumnsCnt(int(columnCntStr), int(pageIndStr))

    def SortIt(self, triggerAct: QAction):
        self.model.SortOrder(reverse=(triggerAct.text() == "Descending"))

    def IntoANewPath(self, path: str):
        self.model.setRootPath(path)
        # self.resizeColumnsToContents()
