#include <QtTest/QtTest>
#include <QItemSelectionModel>
#include <QThread>
#include <QFuture>
#include <QSignalSpy>

#include "OnScopeExit.h"

#include "BeginToExposePrivateMember.h"
#include "FileSystemModel.h"
#include "CustomTableView.h"
#include "EndToExposePrivateMember.h"

#include "PlainTestSuite.h"
#include "TDir.h"
#include <QDebug>

Q_DECLARE_METATYPE(Qt::Orientation)

bool headerDataChangedSignalParameterCheck(const QVariantList& actualParms, Qt::Orientation expectOri, int expectRow) {
  if (QMetaType::type("Qt::Orientation") == 0) { // check if type is registered
    LOG_W("Type not registered");
    return false;
  }
  QVariantList expectParams{
      QVariant::fromValue<Qt::Orientation>(expectOri),
      QVariant::fromValue<int>(expectRow),
      QVariant::fromValue<int>(expectRow),
  };
  if (actualParms == expectParams) {
    return true;
  }
  qDebug() << "actual: ";
  for (auto& var : actualParms) {
    qDebug() << "[" << var << "],";
  }
  qDebug() << "expect: ";
  for (auto& var : expectParams) {
    qDebug() << "[" << var << "],";
  }
  return false;
}

class FileSystemModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
  QDir mDir{tDir.path()};
  const QList<FsNodeEntry> nodeEntries  //
      {
          FsNodeEntry{"Chris Hemsworth", true, ""},                       //
          FsNodeEntry{"Chris Hemsworth Introduction.txt", false, "txt"},  //
          FsNodeEntry{"Chris Hemsworth Suit.png", false, "png"},          //
          FsNodeEntry{"Chris Hemsworth Wallpaper.jpg", false, "jpg"},     //
          FsNodeEntry{"Chris Hemsworth X.jpg", false, "jpg"},             //
      };
  QString pathRoot5{tDir.path()};
  QString pathSub0{tDir.itemPath("Chris Hemsworth")};

  FileSystemModel* model{nullptr};
  CustomTableView* view{nullptr};
  const QDir::Filters INITIAL_DIR_FILTERS{QDir::Filter::NoDotAndDotDot | QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Hidden};
 private slots:
  void initTestCase() {
    qRegisterMetaType<Qt::Orientation>("Qt::Orientation");

    QVERIFY(tDir.IsValid());
    QCOMPARE(tDir.createEntries(nodeEntries), nodeEntries.size());

    view = new (std::nothrow) CustomTableView{"FileSystemTableView"};
    QVERIFY(view != nullptr);

    model = new (std::nothrow) FileSystemModel{view};
    QVERIFY(model != nullptr);

    model->setFilter(INITIAL_DIR_FILTERS);
    model->sort(0, Qt::AscendingOrder);
    QVERIFY(model->_mPLogger == nullptr);
    QCOMPARE(model->supportedDropActions(), (Qt::MoveAction | Qt::CopyAction | Qt::LinkAction));
    QCOMPARE(model->supportedDragActions(), (Qt::MoveAction | Qt::CopyAction | Qt::LinkAction));
    model->BindLogger(nullptr);  // should not crash down
    QVERIFY(model->_mPLogger == nullptr);

    view->setModel(model);
    view->setSortingEnabled(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  }

  void cleanupTestCase() {
    tDir.remove();

    if (view != nullptr) {
      delete view;
      view = nullptr;
    }
  }

  void test_ToggleBetweenUniquePath_RowCountCorrect() {
    // path switch: pathRoot5 → pathSub0 → pathRoot5
    auto PathSwitch = [](QFileSystemModel* model, const QString& path) {
      QSignalSpy spy(model, &QFileSystemModel::directoryLoaded);
      QModelIndex rootIndex = model->setRootPath(path);
      return spy.wait(1000) ? model->rowCount(rootIndex) : -1;
    };
    // precondition
    QVERIFY(model->rootPath() != pathRoot5);

    QCOMPARE(PathSwitch(model, pathRoot5), 5);
    QCOMPARE(model->rootPath(), pathRoot5);

    QCOMPARE(PathSwitch(model, pathSub0), 0);
    QCOMPARE(model->rootPath(), pathSub0);

    QCOMPARE(PathSwitch(model, pathRoot5), 5);
    QCOMPARE(model->rootPath(), pathRoot5);
  }

  void reenterSamePathTwice_RowCountCorrect() {
    // precondition
    QString currentValidPath = model->rootPath();
    QVERIFY(currentValidPath != "");
    QVERIFY(QFileInfo{currentValidPath}.isDir());
    QModelIndex rootIndex;

    QSignalSpy renterSpy(model, &QFileSystemModel::directoryLoaded);
    rootIndex = model->setRootPath(currentValidPath);
    QCOMPARE(renterSpy.wait(1000), false);  // reenter(path unchange) directoryLoaded signal should not be emit
    QCOMPARE(renterSpy.count(), 0);

    QSignalSpy uniqueSpy(model, &QFileSystemModel::directoryLoaded);
    rootIndex = model->setRootPath("");
    rootIndex = model->setRootPath(currentValidPath);
    QCOMPARE(uniqueSpy.wait(1000), true);  // path changed directoryLoaded signal should not be emit
    QVERIFY(uniqueSpy.count() >= 1);

    QList<QVariant> dirloadedParams = uniqueSpy.last();
    QCOMPARE(dirloadedParams.size(), 1);
    QCOMPARE(dirloadedParams[0].toString(), currentValidPath);

    QDir curDir{currentValidPath, "", QDir::SortFlag::NoSort, model->filter()};
    QStringList itemsLst = curDir.entryList();
    QCOMPARE(itemsLst.size(), model->rowCount(rootIndex));
  }

  void test_dirFilter_nameFilterDisables_works_ok() {
    // precodition 1
    const auto& allItems = mDir.entryList(INITIAL_DIR_FILTERS);
    QCOMPARE(allItems.size(), 5);
    // precodition 2
    const auto& afterHideItems = mDir.entryList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    QCOMPARE(afterHideItems.size(), 1);

    // only 1 item with this filter and hide item that don't pass the filter directly
    QModelIndex rootIndex = model->setRootPath(pathRoot5);

    ON_SCOPE_EXIT {
      model->setFilter(INITIAL_DIR_FILTERS);
      model->setNameFilterDisables(true);
      QTRY_COMPARE(model->rowCount(rootIndex), 5);
    };

    model->setFilter(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
    model->setNameFilterDisables(true);
    QTRY_COMPARE(model->rowCount(rootIndex), 5);

    model->setNameFilterDisables(false);
    QTRY_COMPARE(model->rowCount(rootIndex), 1);
  }

  void test_basic_functions() {
    model->sort(0, Qt::AscendingOrder);
    QModelIndex rootIndex = model->setRootPath(pathRoot5);
    view->setRootIndex(rootIndex);
    QVERIFY(rootIndex.isValid());
    QCOMPARE(model->fileName(model->index(0, 0, rootIndex)), nodeEntries[0].relativePathToNode);
    QCOMPARE(model->fileName(model->index(1, 0, rootIndex)), nodeEntries[1].relativePathToNode);
    QCOMPARE(model->fileName(model->index(2, 0, rootIndex)), nodeEntries[2].relativePathToNode);
    QCOMPARE(model->fileName(model->index(3, 0, rootIndex)), nodeEntries[3].relativePathToNode);
    QCOMPARE(model->fileName(model->index(4, 0, rootIndex)), nodeEntries[4].relativePathToNode);

    const QModelIndex folderIndex{model->index(0, 0, rootIndex)};
    const QModelIndex fileIndex{model->index(1, 0, rootIndex)};
    // "Chris Hemsworth" folder
    QCOMPARE(model->fullInfo(folderIndex), nodeEntries[0].relativePathToNode + '\t' + "" + '\t' + model->rootPath());
    // "Chris Hemsworth Introduction.txt" file contents="txt"
    const QString fileFullInfo{model->fullInfo(fileIndex)};  // 3 字节
    QVERIFY(fileFullInfo.startsWith(nodeEntries[1].relativePathToNode + '\t' + "3"));
    QVERIFY(fileFullInfo.endsWith('\t' + model->rootPath()));

    QVERIFY(model->flags(folderIndex).testFlag(Qt::ItemFlag::ItemIsDropEnabled));
    QVERIFY(!model->flags(fileIndex).testFlag(Qt::ItemFlag::ItemIsDropEnabled));

    QVERIFY(model->flags(folderIndex).testFlag(Qt::ItemFlag::ItemIsDragEnabled));
    QVERIFY(model->flags(fileIndex).testFlag(Qt::ItemFlag::ItemIsDragEnabled));

    QMimeData emptyData;
    QVERIFY(!model->canDropMimeData(&emptyData, Qt::DropAction::MoveAction, folderIndex.row(), folderIndex.column(), rootIndex));

    // nothing happend
    model->DragRelease();
    QSignalSpy headerDataChangedSpy(model, &QFileSystemModel::headerDataChanged);
    model->DragHover(fileIndex);
    {
      QCOMPARE(headerDataChangedSpy.count(), 2);
      QVERIFY(headerDataChangedSignalParameterCheck(headerDataChangedSpy[0], Qt::Orientation::Vertical, -1));  // release the former one
      QVERIFY(headerDataChangedSignalParameterCheck(headerDataChangedSpy[1], Qt::Orientation::Vertical, fileIndex.row()));
    }
    model->DragHover(folderIndex);
    model->DragHover(folderIndex);  // still, folderIndex not changed
    {
      QCOMPARE(headerDataChangedSpy.count(), 2 + 2 + 0);
      QVERIFY(headerDataChangedSignalParameterCheck(headerDataChangedSpy[2], Qt::Orientation::Vertical, fileIndex.row()));  // release the former one
      QVERIFY(headerDataChangedSignalParameterCheck(headerDataChangedSpy[3], Qt::Orientation::Vertical, folderIndex.row()));
    }
    model->DragRelease();
    model->DragRelease();  // already released
    {
      QCOMPARE(headerDataChangedSpy.count(), 2 + 2 + 0 + 1 + 0);
      QVERIFY(headerDataChangedSignalParameterCheck(headerDataChangedSpy[4], Qt::Orientation::Vertical, folderIndex.row()));
    }
  }

  void test_CutSomething_BuiltInPaste() {
    QCOMPARE(model->rootPath(), pathRoot5);
    QVERIFY(Select2Files());
    QItemSelectionModel* selectionModel = view->selectionModel();
    model->CutSomething(selectionModel->selectedRows());

    // 1. paste cut in built-in model
    QModelIndex subIndex;
    {
      QSignalSpy spy(model, &QFileSystemModel::directoryLoaded);
      subIndex = model->setRootPath(pathSub0);
      view->setRootIndex(subIndex);
      if (spy.count() == 0) {
        QVERIFY(spy.wait(1000));
      }
      QVERIFY(subIndex.isValid());
    }
    {
      QSignalSpy spy(model, &QFileSystemModel::directoryLoaded);
      QVERIFY(mDir.rename("Chris Hemsworth Suit.png", "Chris Hemsworth/Chris Hemsworth Suit.png"));
      QVERIFY(mDir.rename("Chris Hemsworth Wallpaper.jpg", "Chris Hemsworth/Chris Hemsworth Wallpaper.jpg"));
      if (spy.count() == 0) {
        QVERIFY(spy.wait(1000));
      }
      QVERIFY(mDir.exists("Chris Hemsworth/Chris Hemsworth Suit.png"));
      QVERIFY(mDir.exists("Chris Hemsworth/Chris Hemsworth Wallpaper.jpg"));
      QCOMPARE(model->rowCount(subIndex), 2);
    }
    {
      // move files back
      QVERIFY(mDir.rename("Chris Hemsworth/Chris Hemsworth Suit.png", "Chris Hemsworth Suit.png"));
      QVERIFY(mDir.rename("Chris Hemsworth/Chris Hemsworth Wallpaper.jpg", "Chris Hemsworth Wallpaper.jpg"));
      QVERIFY(mDir.exists("Chris Hemsworth Suit.png"));
      QVERIFY(mDir.exists("Chris Hemsworth Wallpaper.jpg"));
      QSignalSpy spy(model, &QFileSystemModel::directoryLoaded);
      view->setRootIndex(model->setRootPath(pathRoot5));
      if (spy.count() == 0) {
        QVERIFY(spy.wait(1000));
      }
      QCOMPARE(model->rowCount(model->index(pathRoot5)), 5);
    }
    {  // cut / copy / cut switch
      view->selectAll();
      QModelIndexList inds{view->selectionModel()->selectedRows()};
      QCOMPARE(inds.size(), 5);
      model->CopiedSomething(inds);
      model->CutSomething(inds);
      model->CopiedSomething(inds);
    }
  }

  void test_CutSomething_FileSystemPaste() {
    QCOMPARE(model->rootPath(), pathRoot5);
    QVERIFY(Select2Files());
    QItemSelectionModel* selectionModel = view->selectionModel();
    model->CutSomething(selectionModel->selectedRows());

    // 1. paste cut in file system model
    // 2. wait for pathRoot5 loaded
    {
      QSignalSpy spy(model, &QFileSystemModel::rowsRemoved);
      QVERIFY(mDir.rename("Chris Hemsworth Suit.png", "Chris Hemsworth/Chris Hemsworth Suit.png"));
      QVERIFY(mDir.rename("Chris Hemsworth Wallpaper.jpg", "Chris Hemsworth/Chris Hemsworth Wallpaper.jpg"));
      QVERIFY(!mDir.exists("Chris Hemsworth Suit.png"));
      QVERIFY(!mDir.exists("Chris Hemsworth Wallpaper.jpg"));
      if (spy.count() == 0) {
        QVERIFY(spy.wait(1000));
      }
      QCOMPARE(model->rowCount(model->index(pathRoot5)), 3);
    }
    // into pathSub0
    {
      QSignalSpy spy(model, &QFileSystemModel::directoryLoaded);
      QModelIndex subIndex = model->setRootPath(pathSub0);
      view->setRootIndex(subIndex);
      if (spy.count() == 0) {
        QVERIFY(spy.wait(1000));
      }
      QCOMPARE(model->rowCount(subIndex), 2);
    }
    // into pathRoot5
    {
      QSignalSpy spy(model, &QFileSystemModel::directoryLoaded);
      QModelIndex rootIndex = model->setRootPath(pathRoot5);
      view->setRootIndex(rootIndex);
      QVERIFY(spy.wait(1000));
      QCOMPARE(model->rowCount(rootIndex), 3);
    }
    // cut/copy in pathRoot5 ok
    {
      view->selectAll();
      QModelIndexList inds{view->selectionModel()->selectedRows()};
      QCOMPARE(inds.size(), 3);
      model->CutSomething(inds);
      model->CopiedSomething(inds);
      model->CutSomething(inds);
    }
    // move files back ok
    {
      QSignalSpy spy(model, &QFileSystemModel::rowsInserted);
      QVERIFY(mDir.rename("Chris Hemsworth/Chris Hemsworth Suit.png", "Chris Hemsworth Suit.png"));
      QVERIFY(mDir.rename("Chris Hemsworth/Chris Hemsworth Wallpaper.jpg", "Chris Hemsworth Wallpaper.jpg"));
      QVERIFY(mDir.exists("Chris Hemsworth Suit.png"));
      QVERIFY(mDir.exists("Chris Hemsworth Wallpaper.jpg"));
      if (spy.count() == 0) {
        QVERIFY(spy.wait(1000));
      }
      QCOMPARE(model->rowCount(model->index(pathRoot5)), 5);
    }
  }

  void data_retrieve_correct() {
    CustomStatusBar customStatusBar;

    FileSystemModel fsModel;
    QVERIFY(fsModel._mPLogger == nullptr);
    fsModel.BindLogger(&customStatusBar);

    QCOMPARE(fsModel.headerData(0, Qt::Vertical, Qt::ItemDataRole::TextAlignmentRole).toInt(), (int)Qt::AlignRight);

    fsModel.m_draggedHoverIndex = fsModel.index(0, 0);
    QCOMPARE(fsModel.headerData(0, Qt::Vertical, Qt::ItemDataRole::DecorationRole).isNull(), false);  // drag icon
    fsModel.m_draggedHoverIndex = QModelIndex{};

    // empty ModelIndex should not crash down
    QCOMPARE(fsModel.data(QModelIndex(), Qt::ItemDataRole::DisplayRole).isNull(), true);

    // dropMimeData do nothing now
    QCOMPARE(fsModel.dropMimeData(nullptr, Qt::DropAction::LinkAction, 0, 0, {}), false);
    QMimeData emptyMimeData;
    QCOMPARE(fsModel.dropMimeData(&emptyMimeData, Qt::DropAction::LinkAction, 0, 0, {}), true);

    // directory loaded ok
    QSignalSpy spy(&fsModel, &QFileSystemModel::directoryLoaded);
    QModelIndex rootIndex = fsModel.setRootPath(pathRoot5);
    if (spy.count() == 0) {
      QVERIFY(spy.wait(1000));
    }
    QCOMPARE(fsModel.rowCount(rootIndex), 5);
    QModelIndex firstInd = fsModel.index(0, 0, rootIndex);

    // cut decoration icon ok
    QCOMPARE(fsModel.mCutIndexes.isEmpty(), true);
    fsModel.data(firstInd, Qt::ItemDataRole::DecorationRole);
    fsModel.CutSomething({firstInd});
    QCOMPARE(fsModel.mCutIndexes.isEmpty(), false);
    fsModel.data(firstInd, Qt::ItemDataRole::DecorationRole);
    fsModel.ClearCutDict();

    // copy decoration icon ok
    QCOMPARE(fsModel.mCopyIndexes.isEmpty(), true);
    fsModel.data(firstInd, Qt::ItemDataRole::DecorationRole);
    fsModel.CopiedSomething({firstInd});
    QCOMPARE(fsModel.mCopyIndexes.isEmpty(), false);
    fsModel.data(firstInd, Qt::ItemDataRole::DecorationRole);
    fsModel.ClearCopiedDict();
  }

 private:
  bool Select2Files() {
    model->sort(0, Qt::AscendingOrder);  // must sort every time rootpath changed
    view->clearSelection();
    const QModelIndex rootIndex{model->index(pathRoot5)};
    QModelIndex index2 = model->index(2, 0, rootIndex);
    QModelIndex index3 = model->index(3, 0, rootIndex);
    if (model->fileName(index2) != "Chris Hemsworth Suit.png") {
      return false;
    }
    if (model->fileName(index3) != "Chris Hemsworth Wallpaper.jpg") {
      return false;
    }
    QItemSelectionModel* selectionModel = view->selectionModel();
    selectionModel->select(model->index(2, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    selectionModel->select(model->index(3, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    return selectionModel->selectedRows() == (QModelIndexList{index2, index3});
  }
};

#include "FileSystemModelTest.moc"
REGISTER_TEST(FileSystemModelTest, false)
