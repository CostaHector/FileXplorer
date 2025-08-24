#include <QtTest/QtTest>
#include <QTableView>
#include <QItemSelectionModel>
#include <QThread>
#include <QFuture>

#include "FileSystemModel.h"
#include "MyTestSuite.h"
#include "TDir.h"

QMetaObject::Connection StartOneTimeConnectionDirLoaded(QMetaObject::Connection& conn, QFileSystemModel& model, QEventLoop& loop){
  return QObject::connect(&model, &QFileSystemModel::directoryLoaded, [&](const QString& curPath) {
    if (curPath == model.rootPath()) {
      qDebug("Rootpath loaded[%s]", qPrintable(curPath));
      loop.quit();
      QObject::disconnect(conn);
    } else {
      qDebug("Path loaded[%s] but rootpath not[%s]", qPrintable(curPath), qPrintable(model.rootPath()));
    }
  });
}
QMetaObject::Connection StartOneTimeConnectionRowInserted(QMetaObject::Connection& conn, QFileSystemModel& model, QEventLoop& loop){
  return QObject::connect(&model, &QFileSystemModel::rowsInserted, [&]() {
    loop.quit();
    qDebug("Path rows Inserted[%s]", qPrintable(model.rootPath()));
    QObject::disconnect(conn);
  });
}

class FileSystemModelTest : public MyTestSuite {
  Q_OBJECT
public:
  FileSystemModelTest(): MyTestSuite{false} {}
private slots:
  void testSelectedRowsCount() {
    TDir tDir;
    QDir mDir{tDir.path()};
    QVERIFY(mDir.exists());
    const QList<FsNodeEntry> nodeEntries//
        {
            FsNodeEntry{"Chris Hemsworth", true, ""}, //
            FsNodeEntry{"Chris Hemsworth Introduction.txt", false, "txt"}, //
            FsNodeEntry{"Chris Hemsworth Suit.png", false, "png"}, //
            FsNodeEntry{"Chris Hemsworth Wallpaper.jpg", false, "jpg"}, //
            FsNodeEntry{"Chris Hemsworth X.jpg", false, "jpg"}, //
        };
    QCOMPARE(tDir.createEntries(nodeEntries), nodeEntries.size());
    QCOMPARE(tDir.getEntries().size(), nodeEntries.size());

    QMetaObject::Connection connDL, connRI;
    QEventLoop loopDL, loopRI;

    FileSystemModel model;
    model.setFilter(QDir::Filter::NoDotAndDotDot//
                    | QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Hidden);
    connDL = StartOneTimeConnectionDirLoaded(connDL, model, loopDL);

    QModelIndex rootIndex = model.setRootPath(mDir.absolutePath());
    loopDL.exec();

    QTableView tableView;
    tableView.setModel(&model);
    tableView.setRootIndex(rootIndex);
    tableView.setSortingEnabled(true);
    tableView.setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView.setSelectionMode(QAbstractItemView::ExtendedSelection);

    model.sort(0, Qt::AscendingOrder);

    QCOMPARE(model.rowCount(rootIndex), nodeEntries.size());  // at least 3 items in rootpath
    QModelIndex index0 = model.index(0, 0, rootIndex);
    QModelIndex index1 = model.index(1, 0, rootIndex);
    QModelIndex index2 = model.index(2, 0, rootIndex);
    QModelIndex index3 = model.index(3, 0, rootIndex);
    QModelIndex index4 = model.index(4, 0, rootIndex);
    QVERIFY(index0.isValid());
    QVERIFY(index1.isValid());
    QVERIFY(index2.isValid());
    QVERIFY(index3.isValid());
    QVERIFY(index4.isValid());
    QCOMPARE(model.fileName(index2), nodeEntries[2].relativePathToNode);
    QCOMPARE(model.fileName(index3), nodeEntries[3].relativePathToNode);

    QItemSelectionModel* selectionModel = tableView.selectionModel();


    QModelIndex subfolderRootInd;
    QModelIndex rootInd2;

    // Test normal: 1. built-in widget: cut=>into new path=>paste=>into former path
    selectionModel->select(model.index(2, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    selectionModel->select(model.index(3, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    model.CutSomething(selectionModel->selectedRows());

    connDL = StartOneTimeConnectionDirLoaded(connDL, model, loopDL);
    subfolderRootInd = model.setRootPath(mDir.absoluteFilePath("Chris Hemsworth"));
    tableView.setRootIndex(subfolderRootInd);
    loopDL.exec();
    QCOMPARE(model.rowCount(subfolderRootInd), 0);

    connDL = StartOneTimeConnectionDirLoaded(connDL, model, loopDL);
    QVERIFY(mDir.rename("Chris Hemsworth Suit.png", "Chris Hemsworth/Chris Hemsworth Suit.png"));
    QVERIFY(mDir.rename("Chris Hemsworth Wallpaper.jpg", "Chris Hemsworth/Chris Hemsworth Wallpaper.jpg"));
    QVERIFY(mDir.exists("Chris Hemsworth/Chris Hemsworth Suit.png"));
    QVERIFY(mDir.exists("Chris Hemsworth/Chris Hemsworth Wallpaper.jpg"));
    loopDL.exec();
    QCOMPARE(model.rowCount(subfolderRootInd), 2);
    model.ClearCopyAndCutDict();

    connDL = StartOneTimeConnectionDirLoaded(connDL, model, loopDL);
    rootInd2 = model.setRootPath(mDir.absolutePath());
    tableView.setRootIndex(rootInd2);
    loopDL.exec();
    QCOMPARE(rootIndex, rootInd2);
    QCOMPARE(model.rowCount(rootInd2), 3);

    // move files back
    QVERIFY(mDir.rename("Chris Hemsworth/Chris Hemsworth Suit.png", "Chris Hemsworth Suit.png"));
    QVERIFY(mDir.rename("Chris Hemsworth/Chris Hemsworth Wallpaper.jpg", "Chris Hemsworth Wallpaper.jpg"));
    QVERIFY(mDir.exists("Chris Hemsworth Suit.png"));
    QVERIFY(mDir.exists("Chris Hemsworth Wallpaper.jpg"));

    selectionModel->select(index0, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    selectionModel->select(index1, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    model.CutSomething(selectionModel->selectedRows());
    selectionModel->clearSelection();

    // // Test normal: 2. outer system clipboard widget: cut=>in filesystem view past
    // rootIndex = tableView.rootIndex();
    // selectionModel->select(model.index(2, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // selectionModel->select(model.index(3, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // model.CutSomething(selectionModel->selectedRows());

    // qDebug("Start to move files in root path to its subfolder");
    // connDL = StartOneTimeConnectionDirLoaded(connDL, model, loopDL);
    // tableView.setRootIndex(rootIndex = model.setRootPath(mDir.absolutePath()));
    // QVERIFY(mDir.rename("Chris Hemsworth Suit.png", "Chris Hemsworth/Chris Hemsworth Suit.png"));
    // QVERIFY(mDir.rename("Chris Hemsworth Wallpaper.jpg", "Chris Hemsworth/Chris Hemsworth Wallpaper.jpg"));
    // QVERIFY(mDir.exists("Chris Hemsworth/Chris Hemsworth Suit.png"));
    // QVERIFY(mDir.exists("Chris Hemsworth/Chris Hemsworth Wallpaper.jpg"));
    // loopDL.exec();
    // qDebug("Root path should contains 3 item(s)");
    // QCOMPARE(model.rowCount(rootIndex), 3);

    // qDebug("Refresh into root path");
    // connDL = StartOneTimeConnectionDirLoaded(connDL, model, loopDL);
    // tableView.setRootIndex(rootIndex = model.setRootPath(mDir.absolutePath()));
    // loopDL.exec();

    // // move files back
    // qDebug("Start to move files in subfolder back to root path");
    // connDL = StartOneTimeConnectionDirLoaded(connDL, model, loopDL);
    // tableView.setRootIndex(rootIndex);
    // QVERIFY(mDir.rename("Chris Hemsworth/Chris Hemsworth Suit.png", "Chris Hemsworth Suit.png"));
    // QVERIFY(mDir.rename("Chris Hemsworth/Chris Hemsworth Wallpaper.jpg", "Chris Hemsworth Wallpaper.jpg"));
    // QVERIFY(mDir.exists("Chris Hemsworth Suit.png"));
    // QVERIFY(mDir.exists("Chris Hemsworth Wallpaper.jpg"));
    // loopDL.exec();
    // qDebug("Root path should contains 5 item(s)");
    // QCOMPARE(model.rowCount(rootIndex), 5);

    // selectionModel->select(model.index(0, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // selectionModel->select(model.index(1, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // model.CutSomething(selectionModel->selectedRows());
    // selectionModel->clearSelection();

    // // Test abnormal: 3. move 2 files to its subfolder in file-system model
    // selectionModel->select(model.index(3, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // selectionModel->select(model.index(4, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // model.CutSomething(selectionModel->selectedRows());

    // QVERIFY(mDir.rename("Chris Hemsworth Suit.png", "Chris Hemsworth/Chris Hemsworth Suit.png"));
    // QVERIFY(mDir.rename("Chris Hemsworth Wallpaper.jpg", "Chris Hemsworth/Chris Hemsworth Wallpaper.jpg"));
    // QVERIFY(mDir.exists("Chris Hemsworth/Chris Hemsworth Suit.png"));
    // QVERIFY(mDir.exists("Chris Hemsworth/Chris Hemsworth Wallpaper.jpg"));

    // selectionModel->select(model.index(0, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // selectionModel->select(model.index(1, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // selectionModel->select(model.index(2, 0, rootIndex), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    // model.CutSomething(selectionModel->selectedRows());
  }
};

#include "FileSystemModelTest.moc"
FileSystemModelTest g_FileSystemModelTest;
