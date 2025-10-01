#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "ClipboardGuard.h"
#include "TDir.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"

#include "BeginToExposePrivateMember.h"
#include "FileSystemModel.h"
#include "FileSystemTableView.h"
#include "EndToExposePrivateMember.h"
#include "FileOpActs.h"
#include "MimeDataHelper.h"
#include "AddressBarActions.h"
#include "ViewActions.h"

#include <QDir>
#include <QDirIterator>

class FileSystemTableViewTest : public PlainTestSuite {
  Q_OBJECT
public:
  static bool checkAfterCopyMimeData(FileSystemModel& fsModel, FileSystemTableView& fsView, QClipboard* pClip, const int expectItemCnt) {
    pClip->clear(); // prevent COM error 0x800401D in windows platform error message

    // select all
    auto& fileOpActsInst = FileOpActs::GetInst();
    fsView.selectAll();
    auto mimeDataMember = MimeDataHelper::GetMimeDataMemberFromSourceModel<FileSystemModel>(fsModel,  //
                                                                                            fsView.selectionModel()->selectedRows());
    const QString expectPaths = mimeDataMember.texts.join('\n');
    { // here would modify fsModel header decoration role return value
      if (!MimeDataHelper::FillCutCopySomething<FileSystemModel>(fsModel, mimeDataMember.srcIndexes, Qt::DropAction::MoveAction)) {
        return false;
      }
      SelectionsRangeHelper::ROW_RANGES_LST rowRangeList = fsModel.mCutIndexes.GetTopBottomRange();
      if (rowRangeList.isEmpty()) {
        return false;
      }
    }

    // 1. copy
    {
      emit fileOpActsInst.COPY->triggered();  // this signal not connected to any slot yet
      MimeDataHelper::WriteIntoSystemClipboard(mimeDataMember, Qt::DropAction::CopyAction);
      auto* afterCopyMimeData = pClip->mimeData();  // check to see if system clipboard is ok;
      if (afterCopyMimeData == nullptr) {
        LOG_E("afterCopyMimeData is nullptr");
        return false;
      }
      const QList<QUrl>& actualCopySelectedUrls = afterCopyMimeData->urls();
      if (actualCopySelectedUrls.size() != expectItemCnt) {
        LOG_E("after copy urls count inequal, actual[%d], expectd[%d]", actualCopySelectedUrls.size(), expectItemCnt);
        return false;
      }
      const QString& actualCopyPaths = afterCopyMimeData->text();
      if (actualCopyPaths != expectPaths) {
        LOG_E("after copy texts inequal, actual[%s], expectd[%s]", qPrintable(actualCopyPaths), qPrintable(expectPaths));
        return false;
      }
      pClip->clear();
    }

    // 2. cut
    {
      emit fileOpActsInst.CUT->triggered();  // this signal not connected to any slot yet
      MimeDataHelper::WriteIntoSystemClipboard(mimeDataMember, Qt::DropAction::MoveAction);
      auto* afterCutMimeData = pClip->mimeData();  // check to see if system clipboard is ok;
      if (afterCutMimeData == nullptr) {
        LOG_E("afterCutMimeData is nullptr");
        return false;
      }
      const QList<QUrl>& actualCutSelectedUrls = afterCutMimeData->urls();
      if (actualCutSelectedUrls.size() != expectItemCnt) {
        LOG_E("after cut urls count inequal, actual[%d], expectd[%d]", actualCutSelectedUrls.size(), expectItemCnt);
        return false;
      }
      const QString& actualCutPaths = afterCutMimeData->text();
      if (actualCutPaths != expectPaths) {
        LOG_E("after cut texts inequal, actual[%s], expectd[%s]", qPrintable(actualCutPaths), qPrintable(expectPaths));
        return false;
      }
      pClip->clear();
    }

    // 3. link
    {
      MimeDataHelper::WriteIntoSystemClipboard(mimeDataMember, Qt::DropAction::LinkAction);
      auto* afterLinkMimeData = pClip->mimeData();  // check to see if system clipboard is ok;
      if (afterLinkMimeData == nullptr) {
        LOG_E("afterLinkMimeData is nullptr");
        return false;
      }
      const QList<QUrl>& actualLinkSelectedUrls = afterLinkMimeData->urls();
      if (actualLinkSelectedUrls.size() != expectItemCnt) {
        LOG_E("after link urls count inequal, actual[%d], expectd[%d]", actualLinkSelectedUrls.size(), expectItemCnt);
        return false;
      }
      const QString& actualLinkPaths = afterLinkMimeData->text();
      if (actualLinkPaths != expectPaths) {
        LOG_E("after link texts inequal, actual[%s], expectd[%s]", qPrintable(actualLinkPaths), qPrintable(expectPaths));
        return false;
      }
      pClip->clear();
    }
    return true;
  }

private slots:
  void cut_copy_in_view_mimedata_correct() {
    // auto clean clipboard
    ClipboardGuard clipboardGuard;
    QVERIFY(clipboardGuard);
    auto* pClip = clipboardGuard.clipBoard();
    QVERIFY(pClip != nullptr);
    pClip->clear();

    // precondition
    const QDir::Filters defaultFilters{MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v.toInt()};
    QString currentPath = QFileInfo(__FILE__).absolutePath();
    QDir currentDir{currentPath, "", QDir::SortFlag::NoSort, defaultFilters};
    QStringList itemsInCurrentDir = currentDir.entryList();
    const int expectItemCnt = itemsInCurrentDir.size();
    QVERIFY(expectItemCnt > 0);

    FileSystemModel fsModel;
    fsModel.setFilter(currentDir.filter());  // need call manually
    FileSystemTableView fsView{&fsModel};

    QObject::connect(&fsModel, &QFileSystemModel::directoryLoaded, this, [&](const QString& path){
      bool checkResult = checkAfterCopyMimeData(fsModel, fsView, pClip, expectItemCnt);
#ifndef _WIN32
      QVERIFY2(checkResult, qPrintable(path)); // clipboard is extremely unreliable in windows
#endif
    });

    // wait till directory loaded ok
    QSignalSpy directoyLoadedSigSpy(&fsModel, &QFileSystemModel::directoryLoaded);
    const QModelIndex& curIndex = fsModel.setRootPath(currentPath);
    fsView.setRootIndex(curIndex);
    QCOMPARE(directoyLoadedSigSpy.wait(1000), true);
    QCOMPARE(directoyLoadedSigSpy.count(), 1);

    QList<QVariant> params = directoyLoadedSigSpy.back();
    QCOMPARE(params.size(), 1);
    QCOMPARE(params[0].toString(), currentPath);
  }

  void dragMove_and_drop_and_dragLeave_protection_ok() {
    // precondition
    const QDir::Filters defaultFilters{MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v.toInt()};

    FileSystemModel fsModel;
    fsModel.setFilter(defaultFilters);  // need call manually
    FileSystemTableView fsView{&fsModel};
    const QPoint centerPnt = fsView.geometry().center();

    // prepare environment for cut or copy
    TDir tDir;
    QList<FsNodeEntry> nodes{
      {"folder", true, ""},
      {"file.txt", false, "hello"},
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
    const QString rootPath = tDir.path();

    // 0. no urls no need drag/drop
    QMimeData nourlsMoveMimeData;
    nourlsMoveMimeData.setText("no urls");
    QDragEnterEvent dragEnterIgnoreEvent(centerPnt, Qt::DropAction::MoveAction, &nourlsMoveMimeData, Qt::LeftButton,
                                         Qt::KeyboardModifier::NoModifier);
    fsView.dragEnterEvent(&dragEnterIgnoreEvent);
    QCOMPARE(dragEnterIgnoreEvent.isAccepted(), false);  // ignore drag enter

    // 1. a normal path event pos index is a file
    {
      QSignalSpy firstDirLoad(&fsModel, &QFileSystemModel::directoryLoaded);
      const QModelIndex rootIndex = fsModel.setRootPath(rootPath);
      fsView.setRootIndex(rootIndex);
      QVERIFY(firstDirLoad.wait(1000));
      QCOMPARE(firstDirLoad.count(), 1);
      QCOMPARE(firstDirLoad.back().size(), 1);
      QCOMPARE(firstDirLoad.back()[0].toString(), rootPath);
    }
    QMimeData urlsMoveMimeData;
    urlsMoveMimeData.setText("has urls");
    QList<QUrl> moveUrls;
    moveUrls << QUrl::fromLocalFile(tDir.itemPath("folder"));
    urlsMoveMimeData.setUrls(moveUrls);

    QString filePath = tDir.itemPath("file.txt");
    QModelIndex fileIndex = fsModel.index(filePath);
    QVERIFY(fileIndex.isValid());
    QRect fileRect = fsView.visualRect(fileIndex);  // position of file.txt
    QVERIFY(fileRect.isValid());
    QPoint fileCenterPnt = fileRect.center();
    QCOMPARE(fileCenterPnt.isNull(), false);

    QString folderPath = tDir.itemPath("folder");
    QModelIndex folderIndex = fsModel.index(folderPath);
    QVERIFY(folderIndex.isValid());
    QRect folderRect = fsView.visualRect(folderIndex);  // position of folder
    QVERIFY(folderRect.isValid());
    QPoint folderCenterPnt = folderRect.center();
    QCOMPARE(folderCenterPnt.isNull(), false);

    // 1.1.1 drag move on a file. event get reject
    QDragMoveEvent dragMoveOnFileIgnoredEvent(fileCenterPnt, Qt::DropAction::MoveAction, &urlsMoveMimeData, Qt::LeftButton,
                                              Qt::KeyboardModifier::NoModifier);
    fsView.dragMoveEvent(&dragMoveOnFileIgnoredEvent);
    QCOMPARE(dragMoveOnFileIgnoredEvent.isAccepted(), false);
    QVERIFY(!tDir.exists("file.txt/folder"));

    // 1.1.1 drag move on itself. event get reject
    fsView.selectionModel()->select(folderIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    QDragMoveEvent dragMoveOnItselfIgnoredEvent(fileCenterPnt, Qt::DropAction::MoveAction, &urlsMoveMimeData, Qt::LeftButton,
                                                Qt::KeyboardModifier::NoModifier);
    fsView.dragMoveEvent(&dragMoveOnItselfIgnoredEvent);
    QCOMPARE(dragMoveOnItselfIgnoredEvent.isAccepted(), false);
    QVERIFY(!tDir.exists("file.txt/folder"));
    fsView.selectionModel()->clearSelection();

    // 1.2.1 drop on a file, event get reject
    QDropEvent dropOnFileIgnoredEvent(fileCenterPnt, Qt::DropAction::MoveAction, &urlsMoveMimeData, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
    fsView.dropEvent(&dropOnFileIgnoredEvent);
    QCOMPARE(dropOnFileIgnoredEvent.isAccepted(), false);
    QVERIFY(!tDir.exists("file.txt/folder"));

    // 1.2.2 drop on itself(folder was selected), event get reject
    fsView.selectionModel()->select(folderIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    QDropEvent dropItselfIgnoredEvent(folderCenterPnt, Qt::DropAction::MoveAction, &urlsMoveMimeData, Qt::LeftButton,
                                      Qt::KeyboardModifier::NoModifier);
    fsView.dropEvent(&dropItselfIgnoredEvent);
    QCOMPARE(dropItselfIgnoredEvent.isAccepted(), false);
    QVERIFY(!tDir.exists("file.txt/folder"));
    fsView.selectionModel()->clearSelection();

    // 1.4 drag leave, event get accept
    QDragLeaveEvent dragLeaveEvent;
    fsView.dragLeaveEvent(&dragLeaveEvent);
    QCOMPARE(dragLeaveEvent.isAccepted(), true);
    QVERIFY(!tDir.exists("file.txt/folder"));

    // 2. drop file on an empty path and event().position not at an valid index. reject drop
    QString emptyPath = "";
    {
      const QModelIndex emptyPathIndex = fsModel.setRootPath(emptyPath);
      fsView.setRootIndex(emptyPathIndex);
      QCOMPARE(fsModel.rootPath(), "");
    }

    QDropEvent dropEmptyPathIgnoredEvent(centerPnt, Qt::DropAction::MoveAction, &urlsMoveMimeData, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
    fsView.dropEvent(&dropEmptyPathIgnoredEvent);
    QCOMPARE(dropEmptyPathIgnoredEvent.isAccepted(), false);
    QVERIFY(!QFile("folder").exists());
  }

  void drag_enter_and_drop_event_function_ok() {
    // precondition
    const QDir::Filters defaultFilters{MemoryKey::DIR_FILTER_ON_SWITCH_ENABLE.v.toInt()};

    FileSystemModel fsModel;
    fsModel.setFilter(defaultFilters);  // need call manually
    FileSystemTableView fsView{&fsModel};
    const QPoint pnt = fsView.geometry().center();

    // prepare environment for cut or copy
    TDir tDir;
    QList<FsNodeEntry> nodes{
      {"subfolder/folder", true, ""},
      {"subfolder/file.txt", false, "hello"},
    };
    QCOMPARE(tDir.createEntries(nodes), nodes.size());
    const QString rootPath = tDir.path();
    const QString subPath = tDir.itemPath("subfolder");
    QDir subDir{subPath};

    // 1. drag enter/drop ok, cut from "rootPath/subfolder/*" to "rootPath/*"
    {
      QSignalSpy secondDirLoad(&fsModel, &QFileSystemModel::directoryLoaded);
      const QModelIndex rootIndex = fsModel.setRootPath(rootPath);
      fsView.setRootIndex(rootIndex);
      QVERIFY(secondDirLoad.wait(1000));
      QCOMPARE(secondDirLoad.count(), 1);
      QCOMPARE(secondDirLoad.back().size(), 1);
      QCOMPARE(secondDirLoad.back()[0].toString(), rootPath);
    }
    QMimeData urlsMoveMimeData;
    urlsMoveMimeData.setText("has urls");
    QList<QUrl> moveUrls;
    moveUrls << QUrl::fromLocalFile(tDir.itemPath("subfolder/folder"));
    moveUrls << QUrl::fromLocalFile(tDir.itemPath("subfolder/file.txt"));
    urlsMoveMimeData.setUrls(moveUrls);

    // 1.1 drag enter
    QDragEnterEvent dragEnterAcceptEventMove(pnt, Qt::DropAction::MoveAction, &urlsMoveMimeData, Qt::LeftButton, Qt::KeyboardModifier::NoModifier);
    fsView.dragEnterEvent(&dragEnterAcceptEventMove);
    QCOMPARE(dragEnterAcceptEventMove.isAccepted(), true);
    QVERIFY(!tDir.exists("folder"));
    QVERIFY(!tDir.exists("file.txt"));
    QVERIFY(subDir.exists("folder"));
    QVERIFY(subDir.exists("file.txt"));

    // 1.2 drop
    QDropEvent dropAcceptEventMove{pnt, Qt::DropAction::MoveAction, &urlsMoveMimeData, Qt::LeftButton, Qt::KeyboardModifier::NoModifier};
    fsView.dropEvent(&dropAcceptEventMove);
    QCOMPARE(dropAcceptEventMove.isAccepted(), true);
    QVERIFY(tDir.exists("folder"));
    QVERIFY(tDir.exists("file.txt"));
    QVERIFY(!subDir.exists("folder"));
    QVERIFY(!subDir.exists("file.txt"));

    // 2. drag enter/drop ok, copy from "rootPath/*" to "rootPath/subfolder/*"
    {
      QSignalSpy secondDirLoad(&fsModel, &QFileSystemModel::directoryLoaded);
      const QModelIndex subPathIndex = fsModel.setRootPath(subPath);
      fsView.setRootIndex(subPathIndex);
      QVERIFY(secondDirLoad.wait(1000));
      QCOMPARE(secondDirLoad.count(), 1);
      QCOMPARE(secondDirLoad.back().size(), 1);
      QCOMPARE(secondDirLoad.back()[0].toString(), subPath);
    }

    QMimeData urlsCopyMimeData;
    urlsCopyMimeData.setText("has urls");
    QList<QUrl> copyUrls;
    copyUrls << QUrl::fromLocalFile(tDir.itemPath("folder"));
    copyUrls << QUrl::fromLocalFile(tDir.itemPath("file.txt"));
    urlsCopyMimeData.setUrls(copyUrls);

    // 2.1 drag enter
    QDragEnterEvent dragEnterAcceptEventCopy(pnt, Qt::DropAction::CopyAction, &urlsCopyMimeData, Qt::LeftButton,
                                             Qt::KeyboardModifier::ControlModifier);
    fsView.dragEnterEvent(&dragEnterAcceptEventCopy);
    QCOMPARE(dragEnterAcceptEventCopy.isAccepted(), true);
    QVERIFY(tDir.exists("folder"));
    QVERIFY(tDir.exists("file.txt"));
    QVERIFY(!subDir.exists("folder"));
    QVERIFY(!subDir.exists("file.txt"));

    // 2.2 drop
    QDropEvent dropAcceptEventCopy{pnt, Qt::DropAction::CopyAction, &urlsCopyMimeData, Qt::LeftButton, Qt::KeyboardModifier::ControlModifier};
    fsView.dropEvent(&dropAcceptEventCopy);
    QCOMPARE(dropAcceptEventCopy.isAccepted(), true);
    QVERIFY(tDir.exists("folder"));
    QVERIFY(tDir.exists("file.txt"));
    QVERIFY(subDir.exists("folder"));
    QVERIFY(subDir.exists("file.txt"));

    // 3. drag enter/drop ok, lnk file from "rootPath/subfolder/*" to "rootPath/*"
    {
      QSignalSpy thirdDirLoad(&fsModel, &QFileSystemModel::directoryLoaded);
      const QModelIndex rootIndex = fsModel.setRootPath(rootPath);
      fsView.setRootIndex(rootIndex);
      QVERIFY(thirdDirLoad.wait(1000));
      QCOMPARE(thirdDirLoad.count(), 1);
      QCOMPARE(thirdDirLoad.back().size(), 1);
      QCOMPARE(thirdDirLoad.back()[0].toString(), rootPath);
    }
    QMimeData urlsLinkMimeData;
    urlsLinkMimeData.setText("has urls");
    QList<QUrl> linkUrls;
    linkUrls << QUrl::fromLocalFile(tDir.itemPath("subfolder/file.txt"));
    urlsLinkMimeData.setUrls(moveUrls);

    // 3.1 drag enter
    QDragEnterEvent dragEnterAcceptEventLink(pnt, Qt::DropAction::LinkAction, &urlsLinkMimeData, Qt::LeftButton, Qt::KeyboardModifier::AltModifier);
    fsView.dragEnterEvent(&dragEnterAcceptEventLink);
    QCOMPARE(dragEnterAcceptEventLink.isAccepted(), true);
    QVERIFY(!tDir.exists("file.txt.lnk"));

    QDragMoveEvent dragMoveAcceptEventLink(pnt, Qt::DropAction::LinkAction, &urlsLinkMimeData, Qt::LeftButton, Qt::KeyboardModifier::AltModifier);
    fsView.dragMoveEvent(&dragMoveAcceptEventLink);
    QCOMPARE(dragMoveAcceptEventLink.isAccepted(), true);
    QVERIFY(!tDir.exists("file.txt.lnk"));

    // 3.2 drop
    QDropEvent dropAcceptEventLink{pnt, Qt::DropAction::LinkAction, &urlsLinkMimeData, Qt::LeftButton, Qt::KeyboardModifier::AltModifier};
    fsView.dropEvent(&dropAcceptEventLink);
    QCOMPARE(dropAcceptEventLink.isAccepted(), true);
    QVERIFY(tDir.exists("file.txt.lnk"));
  }

  void mouseSideClick_NavigationSignals() {
    CustomTableView view("CustomTableViewMouseSideKey");

    auto& addressInst = g_addressBarActions();
    auto& viewInst = g_viewActions();

    QSignalSpy backAddressSpy(addressInst._BACK_TO, &QAction::triggered);
    QSignalSpy forwardAddressSpy(addressInst._FORWARD_TO, &QAction::triggered);
    QSignalSpy backViewSpy(viewInst._VIEW_BACK_TO, &QAction::triggered);
    QSignalSpy forwardViewSpy(viewInst._VIEW_FORWARD_TO, &QAction::triggered);

    {
      QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::BackButton, Qt::BackButton, Qt::NoModifier);
      view.mousePressEvent(&event);
      QCOMPARE(backAddressSpy.count(), 1);
      QCOMPARE(event.isAccepted(), true);
    }

    {
      QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::ForwardButton, Qt::ForwardButton, Qt::NoModifier);
      view.mousePressEvent(&event);
      QCOMPARE(forwardAddressSpy.count(), 1);
      QCOMPARE(event.isAccepted(), true);
    }

    {
      QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::BackButton, Qt::BackButton, Qt::ControlModifier);
      view.mousePressEvent(&event);
      QCOMPARE(backViewSpy.count(), 1);
      QCOMPARE(event.isAccepted(), true);
    }

    {
      QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::ForwardButton, Qt::ForwardButton, Qt::ControlModifier);
      view.mousePressEvent(&event);
      QCOMPARE(forwardViewSpy.count(), 1);
      QCOMPARE(event.isAccepted(), true);
    }

    // Alt+back: nothing happen
    {
      QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::BackButton, Qt::BackButton, Qt::AltModifier);
      view.mousePressEvent(&event);
      QCOMPARE(backAddressSpy.count(), 1);
      QCOMPARE(backViewSpy.count(), 1);
    }

    // left click: nothing happen
    {
      QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
      view.mousePressEvent(&event);
      QCOMPARE(backAddressSpy.count(), 1);
      QCOMPARE(forwardViewSpy.count(), 1);
    }

    // all signal params ok
    QCOMPARE(backAddressSpy.first()[0].toBool(), false);
    QCOMPARE(forwardAddressSpy.first()[0].toBool(), false);
    QCOMPARE(backViewSpy.first()[0].toBool(), false);
    QCOMPARE(forwardViewSpy.first()[0].toBool(), false);
  }
};

#include "FileSystemTableViewTest.moc"
REGISTER_TEST(FileSystemTableViewTest, false)
