#include <QtTest/QtTest>
#include "MountHelper.h"
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ViewsStackedWidget.h"
#include "EndToExposePrivateMember.h"

#include "StackedAddressAndSearchToolBar.h"
#include "ViewSwitchHelper.h"
#include "TDir.h"
#include "SceneInfoManager.h"
#include "FileTool.h"
#include "ViewActions.h"
#include "ViewSwitchToolBar.h"
#include "ArchiveFilesActions.h"
#include "CastPsonFileHelper.h"
#include "Logger.h"
#include "MovieDBModelField.h"
#include "ScenesListModel.h"

#include "SystemPath.h"
#include "BehaviorKey.h"
#include "PathKey.h"
#include "Configuration.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

    bool compareModelIndexList(const QModelIndexList& lhs, const QModelIndexList& rhs, bool bIgnoreOrder = false) {
  if (lhs.size() != rhs.size()) {
    LOG_D("array size unequal[%d!=%d]", lhs.size(), rhs.size());
    return false;
  }
  QStringList lStrs, rStrs;
  lStrs.reserve(lhs.size());
  lStrs.reserve(lhs.size());
  for (int i = 0; i < lhs.size(); ++i) {
    const QString lStr{lhs[i].data(Qt::DisplayRole).toString()};
    const QString rStr{rhs[i].data(Qt::DisplayRole).toString()};
    lStrs.push_back(lStr);
    rStrs.push_back(rStr);
  }
  if (bIgnoreOrder) {
    std::sort(lStrs.begin(), lStrs.end());
    std::sort(rStrs.begin(), rStrs.end());
  }
  for (int i = 0; i < lhs.size(); ++i) {
    const QString lStr{lStrs[i]};
    const QString rStr{rStrs[i]};
    if (lStr != rStr) {
      LOG_D("The %dth element differs: actual[%s] expects[%s].", i, qPrintable(lStr), qPrintable(rStr));
      return false;
    }
  }
  return true;
}

class ViewsStackedWidgetTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
private slots:
  void initTestCase() {  //
    QVERIFY(mTDir.IsValid());
    Configuration().clear();
  }

  void cleanupTestCase() {  //
    Configuration().clear();
  }

  void init() {
    mTDir.ClearAll();
    GlobalMockObject::reset();
  }

  void cleanup() {  //
    GlobalMockObject::verify();
  }

  void default_constructor_will_not_crash_down() {
    CurrentRowPreviewer rowPreviewer;
    ViewsStackedWidget viewStkWid(&rowPreviewer);
    QCOMPARE(viewStkWid.GetVt(), ViewTypeTool::DEFAULT_VIEW_TYPE);
    viewStkWid.SetVt(ViewTypeTool::ViewType::LIST);
    QCOMPARE(viewStkWid.GetVt(), ViewTypeTool::ViewType::LIST);
    viewStkWid.SetVt(ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(viewStkWid.GetVt(), ViewTypeTool::DEFAULT_VIEW_TYPE);

    QCOMPARE(viewStkWid.currentWidget(), nullptr);
    QCOMPARE(viewStkWid.GetCurView(), nullptr);
    viewStkWid.BindNavigationAddressBar(nullptr);
    viewStkWid.BindDatabaseSearchToolBar(nullptr);
    viewStkWid.BindAdvanceSearchToolBar(nullptr);
    viewStkWid.BindCastSearchToolBar(nullptr);

    viewStkWid.disconnectSelectionChanged();


    QCOMPARE(viewStkWid.LocateToSpecifiedFile(0), false); // protection:
  }

  void onActionAndViewNavigate_ok() {
    QList<FsNodeEntry> nodes{
                             {"root/sub/Kaka.jpg", false, ""},
                             {"root/emptyFolder", true, ""},
                             };
    QCOMPARE(mTDir.createEntries(nodes), 2);
    const QString workPath{mTDir.itemPath("root/sub")};
    const QString emptyWorkPath{mTDir.itemPath("root/emptyFolder")};

    QModelIndex rootSubPathIndex, emptyPathIndex;

    QWidget wid;
    QVBoxLayout lo{&wid};
    CurrentRowPreviewer m_previewFolder{&wid};                               // previewer in docker
    StackedAddressAndSearchToolBar m_stackedBar{"searchToolBarTest", &wid};  // searchToolBar
    ViewsStackedWidget m_fsPanel{&m_previewFolder, &wid};                    // main widget
    NavigationToolBar navigationToolbar{"NavigationToolBar", &wid};
    ViewSwitchHelper m_viewSwitchHelper{&m_stackedBar, &m_fsPanel, &navigationToolbar, &wid};  // view/searchToolBar switcher
    lo.addWidget(&m_previewFolder);
    lo.addWidget(&m_stackedBar);
    lo.addWidget(&m_fsPanel);

    QVERIFY(m_fsPanel.m_fsModel != nullptr);

    // 预期 路径切换 正常
    {
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TABLE);                 // default
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);       // already table view

      QCOMPARE(m_fsPanel.onActionAndViewNavigate("", true), true);
      QCOMPARE(m_stackedBar.m_addressBar->m_addressLine->pathFromLineEdit(), "");  // 空路径ok
      emptyPathIndex = m_fsPanel.m_fsModel->GetRootIndex();
      QVERIFY(!emptyPathIndex.isValid());
      QCOMPARE(m_fsPanel.m_fsTableView->rootIndex(), emptyPathIndex);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), "");

      QCOMPARE(m_fsPanel.onActionAndViewNavigate("path/not/exist", true), false);  // 不存在的路径

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);           // 存在的路径
      QCOMPARE(m_stackedBar.m_addressBar->m_addressLine->pathFromLineEdit(), workPath);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);
      rootSubPathIndex = m_fsPanel.m_fsModel->GetRootIndex();
      QVERIFY(rootSubPathIndex.isValid());
    }

    // 预期: 点击undo/redo 回来的路径, 视为老路径, 不入撤销/重做栈
    // 预期: 双击视图, 路径栏Enter, 点击导航图标 进入的路径, 视为新路径, 入撤销/重做栈
    // 预期: 路径栏Enter信号已连接
    // TABLE(empty path, workPath)
    {
      QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, false), true);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);

      emit m_stackedBar.m_addressBar->m_addressLine->pathActionsTriggeredOrLineEditReturnPressed("", true);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), "");

      emit m_stackedBar.m_addressBar->m_addressLine->pathActionsTriggeredOrLineEditReturnPressed(workPath, true);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);
      QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);

      QCOMPARE(m_fsPanel.m_fsTableView->rootIndex(), rootSubPathIndex);
    }

    // LIST(empty path, workPath)->TREE(empty path, workPath)
    {
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::LIST);
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::LIST);
      QCOMPARE(m_fsPanel.m_fsListView->rootIndex(), rootSubPathIndex);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(emptyWorkPath, true), true);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), emptyWorkPath);

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);


      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TREE);
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TREE);
      QCOMPARE(m_fsPanel.m_fsTreeView->rootIndex(), rootSubPathIndex);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(emptyWorkPath, true), true);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), emptyWorkPath);

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);
    }

    // SEARCH(empty path, workPath)->JSON(empty path, workPath)->SCENE(empty path, workPath)
    {
      // --SEARCH
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SEARCH);
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::SEARCH);
      QCOMPARE(m_fsPanel.m_searchSrcModel->rootPath(), workPath);
      // search 视图下 不可切换路径

      // --JSON
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::JSON);
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::JSON);
      QCOMPARE(m_fsPanel.m_jsonModel->rootPath(), workPath);

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(emptyWorkPath, true), true);
      QCOMPARE(m_fsPanel.m_jsonModel->rootPath(), emptyWorkPath);

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);
      QCOMPARE(m_fsPanel.m_jsonModel->rootPath(), workPath);
      // --SCENE
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SCENE);
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::SCENE);
      QCOMPARE(m_fsPanel.m_scenesModel->rootPath(), workPath);

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(emptyWorkPath, true), true);
      QCOMPARE(m_fsPanel.m_scenesModel->rootPath(), emptyWorkPath);

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);
      QCOMPARE(m_fsPanel.m_scenesModel->rootPath(), workPath);
    }
  }

  void on_searchTextChanged_searchEnterKey_ok() {
    const QByteArray mullerJson{
                                R"({
  "Name": "Thomas Muller",
  "Studio": "",
  "Cast": []
})"};
    const QByteArray lewandowskiJson{
                                     R"({
  "Name": "Robert Lewandowski",
  "Studio": "",
  "Cast": []
})"};
    QList<FsNodeEntry> nodes{
                             {"root/sub/Kaka.jpg", false, ""},
                             {"root/sub/Cristiano Ronaldo & Kaka.jpg", false, ""},
                             {"root/sub/Cristiano Ronaldo.mp4", false, ""},
                             {"root/sub/Muller.json", false, mullerJson},
                             {"root/sub/Lewandowski.json", false, lewandowskiJson},
                             };
    QCOMPARE(mTDir.createEntries(nodes), 5);
    const QString rootPath{mTDir.itemPath("root")};
    const QString workPath{mTDir.itemPath("root/sub")};
    {
      using namespace SceneInfoManager;
      ScnMgr scnMgr1;
      scnMgr1(workPath);
      QCOMPARE(ScnMgr::UpdateScnFiles(workPath), 1);  // 1 scn file[sub.scn] writed ok
      QVERIFY(mTDir.exists("root/sub/sub.scn"));
    }

    QWidget wid;
    QVBoxLayout lo{&wid};
    CurrentRowPreviewer m_previewFolder{&wid};                               // previewer in docker
    StackedAddressAndSearchToolBar m_stackedBar{"searchToolBarTest", &wid};  // searchToolBar
    ViewsStackedWidget m_fsPanel{&m_previewFolder, &wid};                    // main widget
    NavigationToolBar navigationToolbar{"NavigationToolBar", &wid};
    ViewSwitchHelper m_viewSwitchHelper{&m_stackedBar, &m_fsPanel, &navigationToolbar, &wid};  // view/searchToolBar switcher
    lo.addWidget(&m_previewFolder);
    lo.addWidget(&m_stackedBar);
    lo.addWidget(&m_fsPanel);

    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TABLE);  // default
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TABLE);

    QCOMPARE(BehaviorKey::DIR_FILTER_ON_SWITCH_ENABLE.v.data.i, -1);
    const int expectFilterIntVal{(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Drives | QDir::Filter::Hidden | QDir::Filter::NoDotAndDotDot)};
    const QDir::Filters filters{m_fsPanel.m_fsModel->filter()};
    QVERIFY(filters.testFlag(QDir::Filter::Files));
    QVERIFY(filters.testFlag(QDir::Filter::Dirs));
    QVERIFY(filters.testFlag(QDir::Filter::Drives));
    QVERIFY(filters.testFlag(QDir::Filter::Hidden));
    QVERIFY(filters.testFlag(QDir::Filter::NoDotAndDotDot));
    QCOMPARE(m_fsPanel.m_fsModel->nameFilterDisables(), false);
    {
      QSignalSpy spy{m_fsPanel.m_fsModel, &QFileSystemModel::directoryLoaded};
      QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);  // 存在的路径
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);
      const QModelIndex rootInd{m_fsPanel.m_fsModel->GetRootIndex()};
      QCOMPARE(m_fsPanel.on_searchTextChanged("Kaka"), true);
      // 目录异步加载完成 whenDirectoryLoaded, onAfterDirectoryLoaded
      QVERIFY(spy.wait(1000)); // 依赖QFileSystemModel的加载耗时, 这里不检查
      const int sigCnt = spy.count();
      QVERIFY(sigCnt >= 1);
      // QCOMPARE(spy.takeFirst(), (QVariantList{workPath}));
      spy.clear();
      // QVERIFY(textInStatusBar.contains("Total 2 item(s) |"));  // "Kaka.jpg" "Cristiano Ronaldo & Kaka.jpg"
      // QCOMPARE(m_fsPanel.m_fsModel->rowCount(rootInd), 2);
      QCOMPARE(m_fsPanel.m_fsModel->nameFilterDisables(), false);
      QVERIFY(m_fsPanel.on_searchTextChanged(""));
    }

    {
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::JSON);
      QCOMPARE(m_fsPanel.m_jsonModel->rowCount(), 2);
      QCOMPARE(m_fsPanel.m_jsonProxyModel->rowCount(), 2);
      QVERIFY(m_fsPanel.on_searchTextChanged("Kaka"));
      QCOMPARE(m_fsPanel.m_jsonProxyModel->rowCount(), 0);
      QVERIFY(m_fsPanel.on_searchTextChanged("Lewandowski"));
      QCOMPARE(m_fsPanel.m_jsonProxyModel->rowCount(), 1);
      QVERIFY(m_fsPanel.on_searchTextChanged("Muller"));
      QCOMPARE(m_fsPanel.m_jsonProxyModel->rowCount(), 1);
      QVERIFY(m_fsPanel.on_searchTextChanged(""));
      QCOMPARE(m_fsPanel.m_jsonProxyModel->rowCount(), 2);

      QVERIFY(m_fsPanel.on_searchEnterKey("Muller"));  // ignored
      QCOMPARE(m_fsPanel.m_jsonProxyModel->rowCount(), 2);
      QVERIFY(m_fsPanel.on_searchTextChanged(""));
    }

    {
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SCENE);
      QCOMPARE(m_fsPanel.m_scenesModel->rowCount(), 2);
      QCOMPARE(m_fsPanel.m_sceneProxyModel->rowCount(), 2);
      QVERIFY(m_fsPanel.on_searchEnterKey("Kaka"));
      QCOMPARE(m_fsPanel.m_sceneProxyModel->rowCount(), 0);
      QVERIFY(m_fsPanel.on_searchEnterKey("Lewandowski"));
      QCOMPARE(m_fsPanel.m_sceneProxyModel->rowCount(), 1);
      QVERIFY(m_fsPanel.on_searchEnterKey("Muller"));
      QCOMPARE(m_fsPanel.m_sceneProxyModel->rowCount(), 1);
      QVERIFY(m_fsPanel.on_searchEnterKey(""));
      QCOMPARE(m_fsPanel.m_sceneProxyModel->rowCount(), 2);

      QVERIFY(m_fsPanel.on_searchTextChanged("Muller"));  // ignored
      QCOMPARE(m_fsPanel.m_sceneProxyModel->rowCount(), 2);
      QVERIFY(m_fsPanel.on_searchTextChanged(""));
    }

    {
      // CAST not support on_searchTextChanged/on_searchEnterKey
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::CAST);
      QVERIFY(!m_fsPanel.on_searchTextChanged("Muller"));
      QVERIFY(!m_fsPanel.on_searchEnterKey("Muller"));
      QVERIFY(!m_fsPanel.on_searchTextChanged(""));
    }
  }

  void key_press_event_ok() {  // 双击， backspace加
    const QByteArray kakaJson{
                              R"({
  "Name": "Kaka",
  "Studio": "",
  "Cast": []
})"};
    const QByteArray kakaPson{CastPsonFileHelper::CastValues2PsonStr("Kaka", 10, "Son of God", "Football", "lvl0")};
    QList<FsNodeEntry> nodes{
        {"lvl0/Kaka/Kaka.har", false, ""},         //
        {"lvl0/Kaka/Kaka.jpg", false, ""},         //
        {"lvl0/Kaka/Kaka.json", false, kakaJson},  //
        {"lvl0/Kaka/Kaka.pson", false, kakaPson},  //
        {"lvl0/Kaka/Kaka.qz", false, ""},          //
        {"lvl0/Kaka/Kaka.torrent", false, ""},     //
    };
    QCOMPARE(mTDir.createEntries(nodes), 6);
    const QString lvl0Path{mTDir.itemPath("lvl0")};
    const QString lvl1Path{mTDir.itemPath("lvl0/Kaka")};

    QWidget wid;
    QVBoxLayout lo{&wid};
    CurrentRowPreviewer m_previewFolder{&wid};                               // previewer in docker
    StackedAddressAndSearchToolBar m_stackedBar{"searchToolBarTest", &wid};  // searchToolBar
    ViewsStackedWidget m_fsPanel{&m_previewFolder, &wid};                    // main widget
    NavigationToolBar navigationToolbar{"NavigationToolBar", &wid};
    ViewSwitchHelper m_viewSwitchHelper{&m_stackedBar, &m_fsPanel, &navigationToolbar, &wid};  // view/searchToolBar switcher
    ViewSwitchToolBar m_viewSwitcher{"View Switcher", &wid};

    lo.addWidget(&m_previewFolder);
    lo.addWidget(&m_stackedBar);
    lo.addWidget(&m_fsPanel);

    // 主动模仿FileXplorer连接, 再Cast视图中双击卡片, 会通过_TABLE_VIEW->toggle()来释放viewTypeChanged信号, 最后切换到table视图
    connect(&m_viewSwitcher, &ViewSwitchToolBar::viewTypeChanged, &m_viewSwitchHelper, &ViewSwitchHelper::onSwitchByViewType);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);

    m_fsPanel.m_fsModel->sort(0, Qt::AscendingOrder);
    // Backspace press event ok
    {
      QSignalSpy spy(m_fsPanel.m_fsModel, &QFileSystemModel::directoryLoaded);
      QCOMPARE(m_fsPanel.onActionAndViewNavigate(lvl1Path, true), true);  // 存在的路径
      const QModelIndex lvl1Ind{m_fsPanel.m_fsModel->GetRootIndex()};
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), lvl1Path);
      QVERIFY(spy.wait(1000));
      QVERIFY(spy.count() >= 1);
      // QCOMPARE(spy.takeLast(), (QVariantList{lvl1Path})); // 依赖QFileSystemModel行为, 不检查
      spy.clear();
      QCOMPARE(wid.windowTitle(), lvl1Path);  // 标题会更新为最新路径
      // QCOMPARE(m_fsPanel.m_fsModel->rowCount(lvl1Ind), 6);

      QKeyEvent backspacePathUpEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
      m_fsPanel.keyPressEvent(&backspacePathUpEvent);
      const QModelIndex lvl0Ind{m_fsPanel.m_fsModel->GetRootIndex()};
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), lvl0Path);
      QVERIFY(spy.wait(1000));
      QVERIFY(spy.count() > 0);
      spy.clear();
      // QCOMPARE(spy.takeLast(), (QVariantList{lvl0Path}));
      // QCOMPARE(wid.windowTitle(), lvl0Path);                // 标题会更新为最新路径
      // QCOMPARE(m_fsPanel.m_fsModel->rowCount(lvl0Ind), 1);  // folder[1: lvl1]
    }

    {  // return/enter press event, double click event
      // no current index
      m_fsPanel.GetCurView()->setCurrentIndex({});
      QKeyEvent enterPathIntoEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
      m_fsPanel.keyPressEvent(&enterPathIntoEvent);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), lvl0Path);
      QModelIndex lvl0Index = m_fsPanel.m_fsModel->GetRootIndex();
      QCOMPARE(wid.windowTitle(), lvl0Path);

      // has current index on folder
      QModelIndex firstItemlvl0Index = m_fsPanel.m_fsModel->index(0, 0, lvl0Index);
      QCOMPARE(firstItemlvl0Index.data(Qt::DisplayRole).toString(), "Kaka");
      m_fsPanel.GetCurView()->setCurrentIndex(firstItemlvl0Index);
      QVERIFY(m_fsPanel.GetCurView()->currentIndex().isValid());
      QKeyEvent returnPathIntoEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
      m_fsPanel.keyPressEvent(&returnPathIntoEvent);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), lvl1Path);
      QModelIndex lvl1Index = m_fsPanel.m_fsModel->GetRootIndex();
      QCOMPARE(wid.windowTitle(), lvl1Path);

      // has current index on file Kaka.jpg
      QModelIndex harIndex = m_fsPanel.m_fsModel->index(0, 0, lvl1Index);
      QCOMPARE(harIndex.data(Qt::DisplayRole).toString(), "Kaka.har");

      QModelIndex jpgIndex = m_fsPanel.m_fsModel->index(1, 0, lvl1Index);
      QCOMPARE(jpgIndex.data(Qt::DisplayRole).toString(), "Kaka.jpg");
      const QString jpgFileAbsPath{mTDir.itemPath("lvl0/Kaka/Kaka.jpg")};

      QModelIndex jsonIndex = m_fsPanel.m_fsModel->index(2, 0, lvl1Index);
      QCOMPARE(jsonIndex.data(Qt::DisplayRole).toString(), "Kaka.json");
      const QString jsonFileAbsPath{mTDir.itemPath("lvl0/Kaka/Kaka.json")};

      QModelIndex psonIndex = m_fsPanel.m_fsModel->index(3, 0, lvl1Index);
      QCOMPARE(psonIndex.data(Qt::DisplayRole).toString(), "Kaka.pson");
      const QString psonFileAbsPath{mTDir.itemPath("lvl0/Kaka/Kaka.pson")};

      QModelIndex qzIndex = m_fsPanel.m_fsModel->index(4, 0, lvl1Index);
      QCOMPARE(qzIndex.data(Qt::DisplayRole).toString(), "Kaka.qz");

      QModelIndex torrentIndex = m_fsPanel.m_fsModel->index(5, 0, lvl1Index);
      QCOMPARE(torrentIndex.data(Qt::DisplayRole).toString(), "Kaka.torrent");
      const QString torrentFileAbsPath{mTDir.itemPath("lvl0/Kaka/Kaka.torrent")};

      {
        m_fsPanel.GetCurView()->setCurrentIndex(jpgIndex);
        MOCKER(FileTool::OpenLocalImageFile).expects(exactly(1)).with(jpgFileAbsPath).will(returnValue(true));
        QKeyEvent returnFileOpenEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        m_fsPanel.keyPressEvent(&returnFileOpenEvent);
      }

      {
        QSignalSpy harViewSpy{ViewActions::GetInst()._HAR_VIEW, &QAction::triggered};
        m_fsPanel.on_cellDoubleClicked(harIndex);
        QCOMPARE(harViewSpy.count(), 1);
        QCOMPARE(harViewSpy.takeLast(), (QVariantList{false}));
      }

      {
        MOCKER(FileTool::OpenLocalFileUsingDesktopService).expects(exactly(1)).with(jsonFileAbsPath).will(returnValue(true));
        m_fsPanel.on_cellDoubleClicked(jsonIndex);
      }

      {
        QSignalSpy qzViewSpy{g_AchiveFilesActions().ARCHIVE_PREVIEW, &QAction::toggled};
        m_fsPanel.on_cellDoubleClicked(qzIndex);
        QCOMPARE(qzViewSpy.count(), 1);
        QCOMPARE(qzViewSpy.takeLast(), (QVariantList{true}));
      }

      {
        MOCKER(FileTool::OpenLocalTorrentFile).expects(exactly(1)).with(torrentFileAbsPath).will(returnValue(true));
        m_fsPanel.on_cellDoubleClicked(torrentIndex);
      }
    }

    {  // other key ignored
      QKeyEvent escapePressEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
      m_fsPanel.keyPressEvent(&escapePressEvent);
    }

    {  // 双击CastView的行, 切换到FileSystem TableView, 并修改到对应路径
      QCOMPARE(m_fsPanel.onActionAndViewNavigate(lvl0Path, true), true);  // 存在的路径
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), lvl0Path);

      const QString imageHostPath = mTDir.path();
      setConfig(PathKey::PERFORMER_IMAGEHOST_LOCATE, imageHostPath);
      const QString dbName{mTDir.itemPath("DoubleClickOnCastViewTest.db")};
      const QString connName{"DoubleClickOnCastViewTestConn"};
      MOCKER(SystemPath::PEFORMERS_DATABASE).expects(exactly(1)).will(returnValue(dbName));
      MOCKER(SystemPath::CastDBConnection).expects(exactly(1)).will(returnValue(connName));
      {
        const QString tableName{DB_TABLE::PERFORMERS};
        CastBaseDb castDb{dbName, connName};
        QVERIFY(castDb.IsValid());
        QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
        QVERIFY(castDb.IsTableExist(tableName));
        QVERIFY(castDb.IsTableEmpty(tableName));
        QCOMPARE(castDb.LoadFromPsonFile(imageHostPath), 1);
      }
      ViewActions::GetInst()._SCENE_VIEW->setChecked(true);
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::CAST);
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::CAST);
      QCOMPARE(m_fsPanel.m_castDbModel->rowCount(), 1);

      QModelIndex kakaNameInCastModel = m_fsPanel.m_castDbModel->index(0, CastDbModelField::Name);
      QCOMPARE(kakaNameInCastModel.data(Qt::DisplayRole).toString(), "Kaka");
      QModelIndex kakaRateInCastModel = m_fsPanel.m_castDbModel->index(0, CastDbModelField::Rate);
      QCOMPARE(kakaRateInCastModel.data(Qt::DisplayRole).toInt(), 10);

      QSignalSpy tableViewSpy{ViewActions::GetInst()._TABLE_VIEW, &QAction::toggled};
      QCOMPARE(m_fsPanel.on_cellDoubleClicked(kakaNameInCastModel), true); // ViewActions::GetInst()._TABLE_VIEW->toggle()
      QCOMPARE(tableViewSpy.count(), 1);
      QCOMPARE(tableViewSpy.takeLast(), (QVariantList{true}));
      QCOMPARE(m_stackedBar.m_addressBar->m_addressLine->pathFromLineEdit(), lvl1Path);
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TABLE);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), lvl1Path);
    }
  }

  void select_related_ok() {
    const QByteArray kakaJson{R"({
    "Name": "Kaka",
    "Studio": "",
    "Cast": [],
    "VidName": "Kaka.mp4",
    "Duration": 6000,
    "Rate": 10,
    "Tags": ["Football"],
    "MD5": "01234567890123456789012345678901"
})"};
    const QByteArray neymarJson{R"({
    "Name": "Neymar",
    "Studio": "",
    "Cast": [],
    "VidName": "Neymar.mp4",
    "Duration": 12000,
    "Rate": 9,
    "Tags": ["Football"],
    "MD5": "01234567890123456789012345678902"
})"};

    const QByteArray kakaPson{CastPsonFileHelper::CastValues2PsonStr("Kaka", /*Rate*/10, /*Aka*/"Son of God", /*Tags*/"Football", /*Ori*/"FootballOri")};
    const QByteArray neymarPson{CastPsonFileHelper::CastValues2PsonStr("Neymar", /*Rate*/9, /*Aka*/"Best", /*Tags*/"Football", /*Ori*/"FootballOri")};
    QList<FsNodeEntry> nodes{
        {"FootballOri/Kaka/Kaka.jpg", false, ""},         //
        {"FootballOri/Kaka/Kaka.json", false, kakaJson},  //
        {"FootballOri/Kaka/Kaka.mp4", false, ""},         //
        {"FootballOri/Kaka/Kaka.pson", false, kakaPson},  //
        {"FootballOri/Neymar/Neymar.json", false, neymarJson},  //
        {"FootballOri/Neymar/Neymar.mp4", false, ""},         //
        {"FootballOri/Neymar/Neymar.png", false, ""},         //
        {"FootballOri/Neymar/Neymar.pson", false, neymarPson},  //
    };
    QCOMPARE(mTDir.createEntries(nodes), 8);

    const QString footballOriPath{mTDir.itemPath("FootballOri")};
    const QString kakaPath{mTDir.itemPath("FootballOri/Kaka")};
    const QString neymarPath{mTDir.itemPath("FootballOri/Neymar")};

    QWidget wid;
    QVBoxLayout lo{&wid};
    CurrentRowPreviewer m_previewFolder{&wid};                               // previewer in docker
    StackedAddressAndSearchToolBar m_stackedBar{"searchToolBarTest", &wid};  // searchToolBar
    ViewsStackedWidget m_fsPanel{&m_previewFolder, &wid};                    // main widget
    NavigationToolBar navigationToolbar{"NavigationToolBar", &wid};
    ViewSwitchHelper m_viewSwitchHelper{&m_stackedBar, &m_fsPanel, &navigationToolbar, &wid};  // view/searchToolBar switcher

    lo.addWidget(&m_previewFolder);
    lo.addWidget(&m_stackedBar);
    lo.addWidget(&m_fsPanel);

    // 初始化并主动模仿FileXplorer连接
    // 这里不验证点击toolbar中的按钮来切换视图, 所以无需连接
    // ViewSwitchToolBar m_viewSwitcher{"View Switcher", &wid};
    // connect(&m_viewSwitcher, &ViewSwitchToolBar::viewTypeChanged, &m_viewSwitchHelper, &ViewSwitchHelper::onSwitchByViewType);

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);
    m_fsPanel.m_fsModel->sort(0, Qt::AscendingOrder);

    {  // prepare
      QSignalSpy spy(m_fsPanel.m_fsModel, &QFileSystemModel::directoryLoaded);
      QCOMPARE(m_fsPanel.onActionAndViewNavigate(kakaPath, true), true);  // 存在的路径
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), kakaPath);
      QVERIFY(spy.wait(1000));
      QVERIFY(spy.count() >= 1);
      // QCOMPARE(spy.takeLast(), (QVariantList{kakaPath}));
      spy.clear();
      // QCOMPARE(m_fsPanel.m_fsModel->rowCount(m_fsPanel.m_fsModel->GetRootIndex()), 4);
      m_fsPanel.m_fsModel->sort(0, Qt::AscendingOrder);
    }

    const QStringList modelRootPaths{kakaPath, kakaPath, kakaPath, kakaPath};
    const QStringList fileNames{"Kaka.jpg", "Kaka.json", "Kaka.mp4", "Kaka.pson"};
    const QStringList fileFullPaths{mTDir.itemPath("FootballOri/Kaka/Kaka.jpg"),
                                    mTDir.itemPath("FootballOri/Kaka/Kaka.json"),
                                    mTDir.itemPath("FootballOri/Kaka/Kaka.mp4"),
                                    mTDir.itemPath("FootballOri/Kaka/Kaka.pson")};
    const std::pair<QString, QStringList> expectRootAndRel2Name{kakaPath, fileNames};
    const QList<QUrl> expectUrls{QUrl::fromLocalFile(mTDir.itemPath("FootballOri/Kaka/Kaka.jpg")),
                                 QUrl::fromLocalFile(mTDir.itemPath("FootballOri/Kaka/Kaka.json")),
                                 QUrl::fromLocalFile(mTDir.itemPath("FootballOri/Kaka/Kaka.mp4")),
                                 QUrl::fromLocalFile(mTDir.itemPath("FootballOri/Kaka/Kaka.pson"))};

    // LIST/TABLE/TREE (jpgInd, jsonInd, mp4Ind, psonInd)
    {
      const QModelIndex rootIndex{m_fsPanel.m_fsModel->index(kakaPath)};
      const QModelIndex jpgInd{m_fsPanel.m_fsModel->index(0, 0, rootIndex)};
      const QModelIndex jsonInd{m_fsPanel.m_fsModel->index(1, 0, rootIndex)};
      const QModelIndex mp4Ind{m_fsPanel.m_fsModel->index(2, 0, rootIndex)};
      const QModelIndex psonInd{m_fsPanel.m_fsModel->index(3, 0, rootIndex)};
      QCOMPARE(jpgInd.data(Qt::DisplayRole).toString(), "Kaka.jpg");
      QCOMPARE(jsonInd.data(Qt::DisplayRole).toString(), "Kaka.json");
      QCOMPARE(mp4Ind.data(Qt::DisplayRole).toString(), "Kaka.mp4");
      QCOMPARE(psonInd.data(Qt::DisplayRole).toString(), "Kaka.pson");
      const QModelIndexList selectedRowsIndexes{jpgInd, jsonInd, mp4Ind, psonInd};

      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);
      {
        m_fsPanel.m_fsTableView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
        QCOMPARE(m_fsPanel.getRootPath(), kakaPath);
        QVERIFY(!m_fsPanel.hasSelection());
        QCOMPARE(m_fsPanel.GetCurView(), m_fsPanel.m_fsTableView);

        m_fsPanel.m_fsTableView->selectAll();
        QVERIFY(m_fsPanel.hasSelection());

        QModelIndex leftTop, rightDown;
        std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
        QCOMPARE(leftTop, jpgInd);
        QCOMPARE(rightDown.row(), psonInd.row());

        QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
        QCOMPARE(m_fsPanel.getFullRecords().size(), 4);
        QCOMPARE(m_fsPanel.getFileNamesRelativeToRootPath(), fileNames);
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jpgInd), mTDir.itemPath("FootballOri/Kaka/Kaka.jpg"));
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jsonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(mp4Ind), mTDir.itemPath("FootballOri/Kaka/Kaka.mp4"));
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(psonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.pson"));
        QCOMPARE(m_fsPanel.getFileFullPaths(), fileFullPaths);
        QCOMPARE(m_fsPanel.getModelRootPath(), modelRootPaths);

        const MimeDataHelper::MimeDataMember pathsAndUrls = m_fsPanel.getFileFullPathsAndUrls();
        QCOMPARE(pathsAndUrls.texts, fileFullPaths);
        QCOMPARE(pathsAndUrls.urls, expectUrls);
        QCOMPARE(pathsAndUrls.srcIndexes, selectedRowsIndexes);
        QModelIndexList srcIndexes;
        QCOMPARE(m_fsPanel.getRootPathAndRel2Name(srcIndexes), expectRootAndRel2Name);

        m_fsPanel.getFileFullPathsAndUrls();

        QCOMPARE(m_fsPanel.getSelectedRowsCount(), 4);
        m_fsPanel.m_fsTableView->setCurrentIndex(jpgInd);
        QCOMPARE(m_fsPanel.getCurFileFullPath(), mTDir.itemPath("FootballOri/Kaka/Kaka.jpg"));
        QCOMPARE(m_fsPanel.getCurFileName(), "Kaka.jpg");
        QCOMPARE(m_fsPanel.getFileInfoAtIndex(jpgInd), (QFileInfo{mTDir.itemPath("FootballOri/Kaka/Kaka.jpg")}));

        // LocateToSpecifiedFile ok (jpgInd, jsonInd, mp4Ind, psonInd)
        {
          m_fsPanel.m_fsTableView->setCurrentIndex(jpgInd); // -> 2 time. Find!
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), true);
          QCOMPARE(m_fsPanel.m_fsTableView->currentIndex().row(), mp4Ind.row());
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), false); // -> Reach End. Not find!
          QCOMPARE(m_fsPanel.m_fsTableView->currentIndex().row(), mp4Ind.row());

          m_fsPanel.m_fsTableView->setCurrentIndex(psonInd); // <- 1 time. Find!
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), true);
          QCOMPARE(m_fsPanel.m_fsTableView->currentIndex().row(), mp4Ind.row());
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), false); // <- Reach Begin. Not find!
          QCOMPARE(m_fsPanel.m_fsTableView->currentIndex().row(), mp4Ind.row());
        }
      }

      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::LIST);
      {
        QCOMPARE(m_fsPanel.getRootPath(), kakaPath);
        QVERIFY(!m_fsPanel.hasSelection());
        QCOMPARE(m_fsPanel.GetCurView(), m_fsPanel.m_fsListView);

        m_fsPanel.m_fsListView->selectAll();
        QVERIFY(m_fsPanel.hasSelection());

        QModelIndex leftTop, rightDown;
        std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
        QCOMPARE(leftTop, jpgInd);
        QCOMPARE(rightDown.row(), psonInd.row());

        QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
        QCOMPARE(m_fsPanel.getFullRecords().size(), 4);
        QCOMPARE(m_fsPanel.getFileNamesRelativeToRootPath(), fileNames);
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jpgInd), mTDir.itemPath("FootballOri/Kaka/Kaka.jpg"));
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jsonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jsonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(psonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.pson"));
        QCOMPARE(m_fsPanel.getFileFullPaths(), fileFullPaths);
        QCOMPARE(m_fsPanel.getModelRootPath(), modelRootPaths);

        const MimeDataHelper::MimeDataMember pathsAndUrls = m_fsPanel.getFileFullPathsAndUrls();
        QCOMPARE(pathsAndUrls.texts, fileFullPaths);
        QCOMPARE(pathsAndUrls.urls, expectUrls);
        QCOMPARE(pathsAndUrls.srcIndexes, selectedRowsIndexes);
        QModelIndexList srcIndexes;
        QCOMPARE(m_fsPanel.getRootPathAndRel2Name(srcIndexes), expectRootAndRel2Name);

        m_fsPanel.getFileFullPathsAndUrls();

        QCOMPARE(m_fsPanel.getSelectedRowsCount(), 4);
        m_fsPanel.m_fsListView->setCurrentIndex(jpgInd);
        QCOMPARE(m_fsPanel.getCurFileFullPath(), mTDir.itemPath("FootballOri/Kaka/Kaka.jpg"));
        QCOMPARE(m_fsPanel.getCurFileName(), "Kaka.jpg");
        QCOMPARE(m_fsPanel.getFileInfoAtIndex(jpgInd), (QFileInfo{mTDir.itemPath("FootballOri/Kaka/Kaka.jpg")}));

        // LocateToSpecifiedFile ok (jpgInd, jsonInd, mp4Ind, psonInd)
        {
          m_fsPanel.m_fsListView->setCurrentIndex(jpgInd); // -> 2 time. Find!
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), true);
          QCOMPARE(m_fsPanel.m_fsListView->currentIndex().row(), mp4Ind.row());
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), false); // -> Reach End. Not find!
          QCOMPARE(m_fsPanel.m_fsListView->currentIndex().row(), mp4Ind.row());

          m_fsPanel.m_fsListView->setCurrentIndex(psonInd); // <- 1 time. Find!
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), true);
          QCOMPARE(m_fsPanel.m_fsListView->currentIndex().row(), mp4Ind.row());
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), false); // <- Reach Begin. Not find!
          QCOMPARE(m_fsPanel.m_fsListView->currentIndex().row(), mp4Ind.row());
        }
      }

      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TREE);
      {
        m_fsPanel.m_fsTreeView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
        QCOMPARE(m_fsPanel.getRootPath(), kakaPath);
        QVERIFY(!m_fsPanel.hasSelection());
        QCOMPARE(m_fsPanel.GetCurView(), m_fsPanel.m_fsTreeView);

        m_fsPanel.m_fsTreeView->selectAll();
        QVERIFY(m_fsPanel.hasSelection());

        QModelIndex leftTop, rightDown;
        std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
        QCOMPARE(leftTop, jpgInd);
        QCOMPARE(rightDown.row(), psonInd.row());

        QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
        QCOMPARE(m_fsPanel.getFullRecords().size(), 4);
        QCOMPARE(m_fsPanel.getFileNamesRelativeToRootPath(), fileNames);
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jpgInd), mTDir.itemPath("FootballOri/Kaka/Kaka.jpg"));
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jsonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jsonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
        QCOMPARE(m_fsPanel.getFileFullPathAtIndex(psonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.pson"));
        QCOMPARE(m_fsPanel.getFileFullPaths(), fileFullPaths);
        QCOMPARE(m_fsPanel.getModelRootPath(), modelRootPaths);

        const MimeDataHelper::MimeDataMember pathsAndUrls = m_fsPanel.getFileFullPathsAndUrls();
        QCOMPARE(pathsAndUrls.texts, fileFullPaths);
        QCOMPARE(pathsAndUrls.urls, expectUrls);
        QCOMPARE(pathsAndUrls.srcIndexes, selectedRowsIndexes);
        QModelIndexList srcIndexes;
        QCOMPARE(m_fsPanel.getRootPathAndRel2Name(srcIndexes), expectRootAndRel2Name);

        m_fsPanel.getFileFullPathsAndUrls();

        QCOMPARE(m_fsPanel.getSelectedRowsCount(), 4);
        m_fsPanel.m_fsTreeView->setCurrentIndex(jpgInd);
        QCOMPARE(m_fsPanel.getCurFileFullPath(), mTDir.itemPath("FootballOri/Kaka/Kaka.jpg"));
        QCOMPARE(m_fsPanel.getCurFileName(), "Kaka.jpg");
        QCOMPARE(m_fsPanel.getFileInfoAtIndex(jpgInd), (QFileInfo{mTDir.itemPath("FootballOri/Kaka/Kaka.jpg")}));

        // LocateToSpecifiedFile ok (jpgInd, jsonInd, mp4Ind, psonInd)
        {
          m_fsPanel.m_fsTreeView->setCurrentIndex(jpgInd); // -> 2 time. Find!
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), true);
          QCOMPARE(m_fsPanel.m_fsTreeView->currentIndex().row(), mp4Ind.row());
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), false); // -> Reach End. Not find!
          QCOMPARE(m_fsPanel.m_fsTreeView->currentIndex().row(), mp4Ind.row());

          m_fsPanel.m_fsTreeView->setCurrentIndex(psonInd); // <- 1 time. Find!
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), true);
          QCOMPARE(m_fsPanel.m_fsTreeView->currentIndex().row(), mp4Ind.row());
          QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), false); // <- Reach Begin. Not find!
          QCOMPARE(m_fsPanel.m_fsTreeView->currentIndex().row(), mp4Ind.row());
        }
      }
    }

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SEARCH);
    {
      QCOMPARE(m_fsPanel.m_searchSrcModel->rowCount(), 4);
      m_fsPanel.m_advanceSearchView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
      QCOMPARE(m_fsPanel.getRootPath(), kakaPath);
      QCOMPARE(m_fsPanel.GetCurView(), m_fsPanel.m_advanceSearchView);

      const QModelIndex jpgInd{m_fsPanel.m_searchProxyModel->index(0, 0)};
      const QModelIndex jsonInd{m_fsPanel.m_searchProxyModel->index(1, 0)};
      const QModelIndex mp4Ind{m_fsPanel.m_searchProxyModel->index(2, 0)};
      const QModelIndex psonInd{m_fsPanel.m_searchProxyModel->index(3, 0)};
      QCOMPARE(jpgInd.data(Qt::DisplayRole).toString(), "Kaka.jpg");
      QCOMPARE(jsonInd.data(Qt::DisplayRole).toString(), "Kaka.json");
      QCOMPARE(mp4Ind.data(Qt::DisplayRole).toString(), "Kaka.mp4");
      QCOMPARE(psonInd.data(Qt::DisplayRole).toString(), "Kaka.pson");
      const QModelIndexList selectedRowsIndexes{jpgInd, jsonInd, mp4Ind, psonInd};
      const std::pair<QString, QStringList> expectPreAndNameSearch{kakaPath, fileNames};

      QVERIFY(!m_fsPanel.hasSelection());

      m_fsPanel.m_advanceSearchView->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, jpgInd);
      QCOMPARE(rightDown.row(), psonInd.row());

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 4);
      QCOMPARE(m_fsPanel.getFileNamesRelativeToRootPath(), fileNames);
      QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jpgInd), mTDir.itemPath("FootballOri/Kaka/Kaka.jpg"));
      QCOMPARE(m_fsPanel.getFileFullPathAtIndex(jsonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
      QCOMPARE(m_fsPanel.getFileFullPathAtIndex(psonInd), mTDir.itemPath("FootballOri/Kaka/Kaka.pson"));
      QCOMPARE(m_fsPanel.getFileFullPaths(), fileFullPaths);
      QCOMPARE(m_fsPanel.getModelRootPath(), modelRootPaths);

      const MimeDataHelper::MimeDataMember pathsAndUrls = m_fsPanel.getFileFullPathsAndUrls();
      QCOMPARE(pathsAndUrls.texts, fileFullPaths);
      QCOMPARE(pathsAndUrls.urls, expectUrls);
      QCOMPARE(compareModelIndexList(pathsAndUrls.srcIndexes, selectedRowsIndexes), true);
      QModelIndexList srcIndexes;
      QCOMPARE(m_fsPanel.getRootPathAndRel2Name(srcIndexes), expectPreAndNameSearch);

      m_fsPanel.getFileFullPathsAndUrls();

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 4);
      m_fsPanel.m_advanceSearchView->setCurrentIndex(jpgInd);
      QCOMPARE(m_fsPanel.getCurFileFullPath(), mTDir.itemPath("FootballOri/Kaka/Kaka.jpg"));
      QCOMPARE(m_fsPanel.getCurFileName(), "Kaka.jpg");
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(jpgInd), QFileInfo{mTDir.itemPath("FootballOri/Kaka/Kaka.jpg")});


      // LocateToSpecifiedFile ok (jpgInd, jsonInd, mp4Ind, psonInd)
      {
        m_fsPanel.m_advanceSearchView->setCurrentIndex(jpgInd); // -> 2 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), true);
        QCOMPARE(m_fsPanel.m_advanceSearchView->currentIndex().row(), mp4Ind.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), false); // -> Reach End. Not find!
        QCOMPARE(m_fsPanel.m_advanceSearchView->currentIndex().row(), mp4Ind.row());

        m_fsPanel.m_advanceSearchView->setCurrentIndex(psonInd); // <- 1 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), true);
        QCOMPARE(m_fsPanel.m_advanceSearchView->currentIndex().row(), mp4Ind.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), false);    // <- Reach Begin. Not find!
        QCOMPARE(m_fsPanel.m_advanceSearchView->currentIndex().row(), mp4Ind.row());
      }
    }

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::JSON);
    {
      QCOMPARE(m_fsPanel.m_jsonModel->rowCount(), 1);
      m_fsPanel.m_jsonTableView->sortByColumn(JsonModelField::Name, Qt::SortOrder::AscendingOrder);
      QCOMPARE(m_fsPanel.getRootPath(), kakaPath);

      QCOMPARE(m_fsPanel.onActionAndViewNavigate(footballOriPath, true), true);  // 切换到Kaka的上一级, 现在有两个json
      QCOMPARE(m_fsPanel.m_jsonModel->rowCount(), 2);
      m_fsPanel.m_jsonTableView->sortByColumn(JsonModelField::Name, Qt::SortOrder::AscendingOrder);
      QCOMPARE(m_fsPanel.getRootPath(), footballOriPath);

      const QModelIndex json0Ind{m_fsPanel.m_jsonProxyModel->index(0, JsonModelField::Prepath)};
      const QModelIndex json1Ind{m_fsPanel.m_jsonProxyModel->index(1, JsonModelField::Prepath)};
      QCOMPARE(m_fsPanel.m_jsonProxyModel->index(0, JsonModelField::Name).data(Qt::DisplayRole).toString(), "Kaka");
      QCOMPARE(m_fsPanel.m_jsonProxyModel->index(1, JsonModelField::Name).data(Qt::DisplayRole).toString(), "Neymar");

      const QModelIndexList selectedRowsIndexes{json0Ind, json1Ind};
      const QStringList jsonModelRootPath8{footballOriPath, footballOriPath, footballOriPath, footballOriPath,
                                           footballOriPath, footballOriPath, footballOriPath, footballOriPath};
      const QStringList fileNamesRelatedToJson{"Kaka/Kaka.jpg", "Kaka/Kaka.json", "Kaka/Kaka.mp4", "Kaka/Kaka.pson",
                                               "Neymar/Neymar.json", "Neymar/Neymar.mp4", "Neymar/Neymar.png", "Neymar/Neymar.pson"};
      const QStringList filePathsJson{mTDir.itemPath("FootballOri/Kaka/Kaka.json"), mTDir.itemPath("FootballOri/Neymar/Neymar.json")};
      const std::pair<QString, QStringList> expectPreAndNameJson{footballOriPath, fileNamesRelatedToJson};

      QVERIFY(!m_fsPanel.hasSelection());

      QCOMPARE(m_fsPanel.GetCurView(), m_fsPanel.m_jsonTableView);
      m_fsPanel.m_jsonTableView->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, json0Ind);
      QCOMPARE(rightDown.row(), json1Ind.row());

      QModelIndexList srcIndexes;

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 2);
      QCOMPARE(m_fsPanel.getRootPath(), footballOriPath);
      QCOMPARE(m_fsPanel.getFileNamesRelativeToRootPath(), fileNamesRelatedToJson);
      QCOMPARE(m_fsPanel.getRootPathAndRel2Name(srcIndexes), expectPreAndNameJson);
      QCOMPARE(srcIndexes.size(), 2);

      QCOMPARE(m_fsPanel.getFileFullPathAtIndex(json0Ind), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
      QCOMPARE(m_fsPanel.getFileFullPathAtIndex(json1Ind), mTDir.itemPath("FootballOri/Neymar/Neymar.json"));
      QCOMPARE(m_fsPanel.getFileFullPaths(), filePathsJson);
      QCOMPARE(m_fsPanel.getModelRootPath(), jsonModelRootPath8);

      const MimeDataHelper::MimeDataMember pathsAndUrls = m_fsPanel.getFileFullPathsAndUrls();
      QCOMPARE(pathsAndUrls.isEmpty(), true);

      MimeDataHelper::MimeDataMember mimeDataMemeber = m_fsPanel.getFileFullPathsAndUrls();
      QVERIFY(mimeDataMemeber.isEmpty());

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 2);
      m_fsPanel.m_jsonTableView->setCurrentIndex(json0Ind);
      QCOMPARE(m_fsPanel.getCurFileFullPath(), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
      QCOMPARE(m_fsPanel.getCurFileName(), "Kaka.json");
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(json0Ind), (QFileInfo{mTDir.itemPath("FootballOri/Kaka/Kaka.json")}));
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(json1Ind), (QFileInfo{mTDir.itemPath("FootballOri/Neymar/Neymar.json")}));

      // LocateToSpecifiedFile ok (json0Ind, json1Ind)
      {
        m_fsPanel.m_jsonTableView->setCurrentIndex(json0Ind); // -> 1 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), true);
        QCOMPARE(m_fsPanel.m_jsonTableView->currentIndex().row(), json1Ind.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), false);     // -> Reach End. No more line!
        QCOMPARE(m_fsPanel.m_jsonTableView->currentIndex().row(), json1Ind.row());

        m_fsPanel.m_jsonTableView->setCurrentIndex(json1Ind); // <- 1 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), true);
        QCOMPARE(m_fsPanel.m_jsonTableView->currentIndex().row(), json0Ind.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), false);    // <- Reach Begin. Not find!
        QCOMPARE(m_fsPanel.m_jsonTableView->currentIndex().row(), json0Ind.row());
      }
    }
    QCOMPARE(m_fsPanel.onActionAndViewNavigate(kakaPath, true), true); // 恢复到kaka层级


    // onSwitchByViewType(CAST)后移到准备好库后
    {
      const QString imageHostPath = mTDir.path();
      setConfig(PathKey::PERFORMER_IMAGEHOST_LOCATE, imageHostPath);
      const QString dbName{mTDir.itemPath("SelectionOnCastViewTest.db")};
      const QString connName{"SelectionOnCastViewTestConn"};
      MOCKER(SystemPath::PEFORMERS_DATABASE).expects(exactly(1)).will(returnValue(dbName));
      MOCKER(SystemPath::CastDBConnection).expects(exactly(1)).will(returnValue(connName));
      {  // prepare
        const QString tableName{DB_TABLE::PERFORMERS};
        CastBaseDb castDb{dbName, connName};
        QVERIFY(castDb.IsValid());
        QVERIFY(castDb.CreateTable(tableName, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE));
        QVERIFY(castDb.IsTableExist(tableName));
        QVERIFY(castDb.IsTableEmpty(tableName));
        QCOMPARE(castDb.LoadFromPsonFile(imageHostPath), 2);
      }
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::CAST);
      m_fsPanel.m_castTableView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
      QCOMPARE(m_fsPanel.m_castDbModel->rowCount(), 2);

      const QModelIndex kakaNameInd{m_fsPanel.m_castDbModel->index(0, CastDbModelField::Name)};
      const QModelIndex kakaOriInd{m_fsPanel.m_castDbModel->index(0, CastDbModelField::Ori)};
      const QModelIndex neymarNameInd{m_fsPanel.m_castDbModel->index(1, CastDbModelField::Name)};
      const QModelIndex neymarOriInd{m_fsPanel.m_castDbModel->index(1, CastDbModelField::Ori)};
      {
        QCOMPARE(kakaNameInd.data(Qt::DisplayRole).toString(), "Kaka");
        QCOMPARE(kakaOriInd.data(Qt::DisplayRole).toString(), "FootballOri");
        QCOMPARE(neymarNameInd.data(Qt::DisplayRole).toString(), "Neymar");
        QCOMPARE(neymarOriInd.data(Qt::DisplayRole).toString(), "FootballOri");
      }

      const QModelIndexList selectedRowsIndexes{kakaNameInd, neymarNameInd};

      QVERIFY(!m_fsPanel.hasSelection());
      QCOMPARE(m_fsPanel.GetCurView(), m_fsPanel.m_castTableView);

      m_fsPanel.m_castTableView->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, kakaNameInd);
      QCOMPARE(rightDown.row(), neymarNameInd.row());

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 0); // no suuport
      QCOMPARE(m_fsPanel.getFileFullPathAtIndex(kakaNameInd), kakaPath);
      QCOMPARE(m_fsPanel.getFileFullPathAtIndex(neymarNameInd), neymarPath);
      QCOMPARE(m_fsPanel.getFileFullPaths(), (QStringList{kakaPath, neymarPath}));
      QCOMPARE(m_fsPanel.getModelRootPath(), (QStringList{imageHostPath, imageHostPath}));

      const QStringList foldersRel2RootPath{"FootballOri/Kaka", "FootballOri/Neymar"};
      QCOMPARE(m_fsPanel.getRootPath(), imageHostPath);
      QCOMPARE(m_fsPanel.getFileNamesRelativeToRootPath(), foldersRel2RootPath);
      QModelIndexList srcIndexes;
      QCOMPARE(m_fsPanel.getRootPathAndRel2Name(srcIndexes), (std::pair<QString, QStringList>{imageHostPath, foldersRel2RootPath}));
      QCOMPARE(srcIndexes.size(), 2);

      const MimeDataHelper::MimeDataMember pathsAndUrls = m_fsPanel.getFileFullPathsAndUrls();
      QCOMPARE(pathsAndUrls.isEmpty(), true);

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 2);
      m_fsPanel.m_castTableView->setCurrentIndex(kakaNameInd);
      QCOMPARE(m_fsPanel.getCurFileFullPath(), kakaPath);
      QCOMPARE(m_fsPanel.getCurFileName(), "Kaka");
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(kakaNameInd), (QFileInfo{kakaPath}));

      m_fsPanel.m_castTableView->setCurrentIndex(neymarNameInd);
      QCOMPARE(m_fsPanel.getCurFileFullPath(), neymarPath);
      QCOMPARE(m_fsPanel.getCurFileName(), "Neymar");
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(neymarNameInd), (QFileInfo{neymarPath}));

      // LocateToSpecifiedFile ok (kakaNameInd, neymarNameInd)
      {
        m_fsPanel.m_castTableView->setCurrentIndex(kakaNameInd); // -> 1 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), true);
        QCOMPARE(m_fsPanel.m_castTableView->currentIndex().row(), neymarNameInd.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), false);     // -> Reach End. No more line!
        QCOMPARE(m_fsPanel.m_castTableView->currentIndex().row(), neymarNameInd.row());

        m_fsPanel.m_castTableView->setCurrentIndex(neymarNameInd); // <- 1 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), true);
        QCOMPARE(m_fsPanel.m_castTableView->currentIndex().row(), kakaNameInd.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), false);    // <- Reach Begin. Not find!
        QCOMPARE(m_fsPanel.m_castTableView->currentIndex().row(), kakaNameInd.row());
      }
    }

    {  // Preparation:
      const QString dbName{mTDir.itemPath("SelectionOnMovieViewTest.db")};
      const QString connName{"SelectionOnMovieViewTestConn"};
      const QString tableName{MountPathTableNameMapper::toTableName(footballOriPath)}; // base32 code path
      QVERIFY(tableName != "MOVIE");

      MOCKER(SystemPath::VIDS_DATABASE).expects(exactly(1)).will(returnValue(dbName));
      MOCKER(SystemPath::MovieDBConnection).expects(exactly(1)).will(returnValue(connName));
      {
        FdBasedDb movieDb{dbName, connName};
        QVERIFY(movieDb.IsValid());
        QVERIFY(movieDb.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
        QVERIFY(movieDb.IsTableExist(tableName));
        QVERIFY(movieDb.IsTableEmpty(tableName));
        QCOMPARE(movieDb.ReadADirectoryJson(tableName, footballOriPath), 2);
      }
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::MOVIE);
      QCOMPARE(m_fsPanel.m_movieDbModel->rootPath(), footballOriPath);
      QCOMPARE(m_fsPanel.m_movieDbModel->rowCount(), 2);

      m_fsPanel.m_movieView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
      const QModelIndex movie0Ind = m_fsPanel.m_movieDbModel->index(0, MovieDBModelField::SampleMD5);
      const QModelIndex movie0NameInd = m_fsPanel.m_movieDbModel->index(0, MovieDBModelField::Name);
      const QModelIndex movie1Ind = m_fsPanel.m_movieDbModel->index(1, MovieDBModelField::SampleMD5);
      const QModelIndex movie1NameInd = m_fsPanel.m_movieDbModel->index(1, MovieDBModelField::Name);
      {
        QCOMPARE(movie0NameInd.data(Qt::DisplayRole).toString(), "Kaka.mp4");
        QCOMPARE(movie1NameInd.data(Qt::DisplayRole).toString(), "Neymar.mp4");
      }

      const QModelIndexList selectedRowsMovie{movie0Ind, movie1Ind};
      const QStringList filePathsMovie{mTDir.itemPath("FootballOri/Kaka/Kaka.mp4"),
                                       mTDir.itemPath("FootballOri/Neymar/Neymar.mp4")};
      const QList<QUrl> expectUrls{QUrl::fromLocalFile(mTDir.itemPath("FootballOri/Kaka/Kaka.mp4")),
                                   QUrl::fromLocalFile(mTDir.itemPath("FootballOri/Neymar/Neymar.mp4"))};

      QCOMPARE(m_fsPanel.getRootPath(), footballOriPath);  // rootpath is path from tableName in MOVIE
      QVERIFY(!m_fsPanel.hasSelection());

      QCOMPARE(m_fsPanel.GetCurView(), m_fsPanel.m_movieView);
      m_fsPanel.m_movieView->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, movie0Ind);
      QCOMPARE(rightDown.row(), movie1Ind.row());

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsMovie));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 2);
      QCOMPARE(m_fsPanel.getFileFullPathAtIndex(movie0Ind), mTDir.itemPath("FootballOri/Kaka/Kaka.mp4"));
      QCOMPARE(m_fsPanel.getFileFullPaths(), filePathsMovie);

      // 表名tableName不是MOVIE, 所以root是""
      QCOMPARE(m_fsPanel.getModelRootPath(), (QStringList{footballOriPath, footballOriPath})); // 来自表名
      const QStringList relativePath2RootPath{"Kaka/Kaka.mp4", "Neymar/Neymar.mp4"};
      QCOMPARE(m_fsPanel.getFileNamesRelativeToRootPath(), relativePath2RootPath);
      QModelIndexList srcIndexes;
      QCOMPARE(m_fsPanel.getRootPathAndRel2Name(srcIndexes), (std::pair<QString, QStringList>{footballOriPath, relativePath2RootPath}));
      QCOMPARE(srcIndexes.size(), 2);

      const MimeDataHelper::MimeDataMember pathsAndUrls = m_fsPanel.getFileFullPathsAndUrls();
      QCOMPARE(pathsAndUrls.texts, filePathsMovie);
      QCOMPARE(pathsAndUrls.urls, expectUrls);
      QCOMPARE(pathsAndUrls.srcIndexes, selectedRowsMovie);

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 2);
      m_fsPanel.m_movieView->setCurrentIndex(movie0Ind);
      QCOMPARE(m_fsPanel.getCurFileFullPath(), mTDir.itemPath("FootballOri/Kaka/Kaka.mp4"));
      QCOMPARE(m_fsPanel.getCurFileName(), "Kaka.mp4");
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(movie0Ind), (QFileInfo{mTDir.itemPath("FootballOri/Kaka/Kaka.mp4")}));

      m_fsPanel.m_movieView->setCurrentIndex(movie1Ind);
      QCOMPARE(m_fsPanel.getCurFileFullPath(), mTDir.itemPath("FootballOri/Neymar/Neymar.mp4"));
      QCOMPARE(m_fsPanel.getCurFileName(), "Neymar.mp4");
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(movie1Ind), (QFileInfo{mTDir.itemPath("FootballOri/Neymar/Neymar.mp4")}));

      // LocateToSpecifiedFile ok (movie0Ind, movie1Ind)
      {
        m_fsPanel.m_movieView->setCurrentIndex(movie0Ind); // -> 1 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), true);
        QCOMPARE(m_fsPanel.m_movieView->currentIndex().row(), movie1Ind.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), false);     // -> Reach End. No more line!
        QCOMPARE(m_fsPanel.m_movieView->currentIndex().row(), movie1Ind.row());

        m_fsPanel.m_movieView->setCurrentIndex(movie1Ind); // <- 1 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), true);
        QCOMPARE(m_fsPanel.m_movieView->currentIndex().row(), movie0Ind.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), false);    // <- Reach Begin. Not find!
        QCOMPARE(m_fsPanel.m_movieView->currentIndex().row(), movie0Ind.row());
      }
    }

    // Scene View
    {
      using namespace SceneInfoManager;
      ScnMgr scnMgr1;
      scnMgr1(footballOriPath);
      QCOMPARE(ScnMgr::UpdateScnFiles(footballOriPath), 2);  // 2 scn file[Kaka.scn, Neymar.scn] writed ok
      QVERIFY(mTDir.exists("FootballOri/Kaka/Kaka.scn"));
      QVERIFY(mTDir.exists("FootballOri/Neymar/Neymar.scn"));

      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SCENE);
      QVERIFY(m_fsPanel.onActionAndViewNavigate(footballOriPath, true));
      QCOMPARE(m_fsPanel.m_scenesModel->rowCount(), 2);
      QCOMPARE(m_fsPanel.m_sceneProxyModel->rowCount(), 2);
      m_fsPanel.m_sceneProxyModel->sort((int)SceneInfo::Role::REL_PATH_ROLE, Qt::AscendingOrder);

      QCOMPARE(m_fsPanel.getRootPath(), footballOriPath);
      QVERIFY(!m_fsPanel.hasSelection());

      QCOMPARE(m_fsPanel.GetCurView(), m_fsPanel.m_sceneListView);
      m_fsPanel.m_sceneListView->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      const QModelIndex kakaScnInd{m_fsPanel.m_sceneProxyModel->index(0, 0)};
      const QModelIndex neymarScnInd{m_fsPanel.m_sceneProxyModel->index(1, 0)};
      {
        QCOMPARE(kakaScnInd.data(Qt::DisplayRole).toString(), "Kaka");
        QCOMPARE(neymarScnInd.data(Qt::DisplayRole).toString(), "Neymar");
      }
      const QModelIndexList selectedRowsIndexes{kakaScnInd, neymarScnInd};
      const QStringList modelRootPaths{footballOriPath, footballOriPath, footballOriPath, footballOriPath, footballOriPath,
          footballOriPath, footballOriPath, footballOriPath, footballOriPath, footballOriPath,}; // scn included
      const QStringList relatedFilesRel2Roots{"Kaka/Kaka.jpg", "Kaka/Kaka.json", "Kaka/Kaka.mp4", "Kaka/Kaka.pson", "Kaka/Kaka.scn",
                                              "Neymar/Neymar.json", "Neymar/Neymar.mp4", "Neymar/Neymar.png", "Neymar/Neymar.pson", "Neymar/Neymar.scn"};
      const QStringList fileFullPaths{mTDir.itemPath("FootballOri/Kaka/Kaka.json"),
                                      mTDir.itemPath("FootballOri/Neymar/Neymar.json")};

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, kakaScnInd);
      QCOMPARE(rightDown.row(), neymarScnInd.row());

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 0); // not support
      QCOMPARE(m_fsPanel.getFileFullPathAtIndex(kakaScnInd), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
      QCOMPARE(m_fsPanel.getFileFullPaths(), fileFullPaths);
      QCOMPARE(m_fsPanel.getModelRootPath(), modelRootPaths);
      QCOMPARE(m_fsPanel.getRootPath(), footballOriPath);
      QCOMPARE(m_fsPanel.getFileNamesRelativeToRootPath(), relatedFilesRel2Roots);
      QModelIndexList srcIndexes;
      QCOMPARE(m_fsPanel.getRootPathAndRel2Name(srcIndexes), (std::pair<QString, QStringList>{footballOriPath, relatedFilesRel2Roots}));
      QCOMPARE(srcIndexes.size(), 2);

      const MimeDataHelper::MimeDataMember pathsAndUrls = m_fsPanel.getFileFullPathsAndUrls();
      QVERIFY(pathsAndUrls.isEmpty()); // not support

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 2);
      m_fsPanel.m_sceneListView->setCurrentIndex(kakaScnInd);
      QCOMPARE(m_fsPanel.getCurFileFullPath(), mTDir.itemPath("FootballOri/Kaka/Kaka.json"));
      QCOMPARE(m_fsPanel.getCurFileName(), "Kaka.json");
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(kakaScnInd, SelectionUsage::DEFAULT), (QFileInfo{mTDir.itemPath("FootballOri/Kaka/Kaka.json")}));
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(kakaScnInd, SelectionUsage::RELATED_VIDEO_ONLY), (QFileInfo{mTDir.itemPath("FootballOri/Kaka/Kaka.mp4")}));

      m_fsPanel.m_sceneListView->setCurrentIndex(neymarScnInd);
      QCOMPARE(m_fsPanel.getCurFileFullPath(), mTDir.itemPath("FootballOri/Neymar/Neymar.json"));
      QCOMPARE(m_fsPanel.getCurFileName(), "Neymar.json");
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(neymarScnInd, SelectionUsage::DEFAULT), (QFileInfo{mTDir.itemPath("FootballOri/Neymar/Neymar.json")}));
      QCOMPARE(m_fsPanel.getFileInfoAtIndex(neymarScnInd, SelectionUsage::RELATED_VIDEO_ONLY), (QFileInfo{mTDir.itemPath("FootballOri/Neymar/Neymar.mp4")}));

      // LocateToSpecifiedFile ok (kakaScnInd, neymarScnInd)
      {
        m_fsPanel.m_sceneListView->setCurrentIndex(kakaScnInd); // -> 1 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), true);
        QCOMPARE(m_fsPanel.m_sceneListView->currentIndex().row(), neymarScnInd.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(1), false);     // -> Reach End. No more line!
        QCOMPARE(m_fsPanel.m_sceneListView->currentIndex().row(), neymarScnInd.row());

        m_fsPanel.m_sceneListView->setCurrentIndex(neymarScnInd); // <- 1 time. Find!
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), true);
        QCOMPARE(m_fsPanel.m_sceneListView->currentIndex().row(), kakaScnInd.row());
        QCOMPARE(m_fsPanel.LocateToSpecifiedFile(-1), false);    // <- Reach Begin. Not find!
        QCOMPARE(m_fsPanel.m_sceneListView->currentIndex().row(), kakaScnInd.row());
      }
    }
    QVERIFY(m_fsPanel.onActionAndViewNavigate(kakaPath, true)); // 恢复到Kaka级路径

  }
};

#include "ViewsStackedWidgetTest.moc"
REGISTER_TEST(ViewsStackedWidgetTest, false)
