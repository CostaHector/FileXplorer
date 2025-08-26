#include <QtTest/QtTest>
#include <QItemSelectionModel>
#include <QThread>
#include <QFuture>
#include <QSignalSpy>

#include "FileSystemModel.h"
#include "CustomTableView.h"
#include "MyTestSuite.h"
#include "TDir.h"

class FileSystemModelTest : public MyTestSuite {
  Q_OBJECT
public:
  FileSystemModelTest(): MyTestSuite{false} {
  }
  ~FileSystemModelTest() {
    if (view != nullptr) {
      delete view;
      view = nullptr;
    }
  }
  TDir tDir;
  QDir mDir;
  const QList<FsNodeEntry> nodeEntries//
      {
          FsNodeEntry{"Chris Hemsworth", true, ""}, //
          FsNodeEntry{"Chris Hemsworth Introduction.txt", false, "txt"}, //
          FsNodeEntry{"Chris Hemsworth Suit.png", false, "png"}, //
          FsNodeEntry{"Chris Hemsworth Wallpaper.jpg", false, "jpg"}, //
          FsNodeEntry{"Chris Hemsworth X.jpg", false, "jpg"}, //
      };
  QString pathRoot5, pathSub0;
  FileSystemModel* model{nullptr};
  CustomTableView* view{nullptr};
private slots:
  void initTestCase() {
    QVERIFY(tDir.IsValid());

    pathRoot5 = tDir.path();
    mDir = QDir{pathRoot5};
    QVERIFY(mDir.exists());
    pathSub0 = mDir.absoluteFilePath("Chris Hemsworth");

    QCOMPARE(tDir.createEntries(nodeEntries), nodeEntries.size());

    view = new (std::nothrow) CustomTableView{"FileSystemTableView"};
    QVERIFY(view != nullptr);

    model = new (std::nothrow) FileSystemModel{view};
    QVERIFY(model != nullptr);
    model->setFilter(QDir::Filter::NoDotAndDotDot | QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Hidden);
    model->sort(0, Qt::AscendingOrder);

    view->setModel(model);
    view->setSortingEnabled(true);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  }

  void cleanupTestCase() {
    tDir.remove();
  }

  void test_RowCountAfterPathSwitch() {
    // path switch: pathRoot5 → pathSub0 → pathRoot5
    QCOMPARE(PathSwitch(model, pathRoot5), 5);
    QCOMPARE(PathSwitch(model, pathSub0), 0);
    QCOMPARE(PathSwitch(model, pathRoot5), 5);
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

    QVERIFY(model->canItemsDroppedHere(folderIndex));
    QVERIFY(!model->canItemsDroppedHere(fileIndex));

    QVERIFY(model->canItemsBeDragged(folderIndex));
    QVERIFY(model->canItemsBeDragged(fileIndex));

    QMimeData emptyData;
    QVERIFY(!model->canDropMimeData(&emptyData, Qt::DropAction::MoveAction, folderIndex.row(), folderIndex.column(), rootIndex));
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
    { // cut / copy / cut switch
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
      if (spy.count() == 0) {
        QVERIFY(spy.wait(1000));
      }
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
private:
  int PathSwitch(QFileSystemModel* model, const QString &path) {
    QSignalSpy spy(model, &QFileSystemModel::directoryLoaded);
    QModelIndex rootIndex = model->setRootPath(path);
    bool waitResult{false};
    if (spy.count() == 0) {
      waitResult = spy.wait(1000);
    }
    return waitResult ? model->rowCount(rootIndex) : -1;
  }

  bool Select2Files() {
    model->sort(0, Qt::AscendingOrder); // must sort every time rootpath changed
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
FileSystemModelTest g_FileSystemModelTest;
