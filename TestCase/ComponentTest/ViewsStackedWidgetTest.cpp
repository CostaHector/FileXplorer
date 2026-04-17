#include <QtTest/QtTest>
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
#include "TableFields.h"
#include "ScenesListModel.h"

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
    viewStkWid.BindLogger(nullptr);

    viewStkWid.disconnectSelectionChanged();
  }

  void onActionAndViewNavigate_ok() {
    QList<FsNodeEntry> nodes{
        {"root/sub/Kaka.jpg", false, ""},
        {"root/emptyFolder", true, ""},
    };
    QCOMPARE(mTDir.createEntries(nodes), 2);
    const QString workPath{mTDir.itemPath("root/sub")};
    const QString emptyWorkPath{mTDir.itemPath("root/emptyFolder")};

    QWidget wid;
    QVBoxLayout lo{&wid};
    CurrentRowPreviewer m_previewFolder{&wid};                               // previewer in docker
    StackedAddressAndSearchToolBar m_stackedBar{"searchToolBarTest", &wid};  // searchToolBar
    ViewsStackedWidget m_fsPanel{&m_previewFolder, &wid};                    // main widget
    ScenePageControl m_scenePageControl{"Pagination display", &wid};
        NavigationToolBar navigationToolbar{"NavigationToolBar", &wid};
    ViewSwitchHelper m_viewSwitchHelper{&m_stackedBar, &m_fsPanel, &m_scenePageControl, &navigationToolbar, &wid};  // view/searchToolBar switcher
    lo.addWidget(&m_previewFolder);
    lo.addWidget(&m_stackedBar);
    lo.addWidget(&m_fsPanel);
    lo.addWidget(&m_scenePageControl);

    QVERIFY(m_fsPanel.m_fsModel != nullptr);

    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TABLE);  // default
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);
    QCOMPARE(m_fsPanel.onActionAndViewNavigate("", true), true);
    QCOMPARE(m_stackedBar.m_addressBar->m_addressLine->pathFromLineEdit(), "");  // 空路径ok
    const QModelIndex emptyPathIndex = m_fsPanel.m_fsModel->GetRootIndex();
    QCOMPARE(m_fsPanel.m_fsTableView->rootIndex(), emptyPathIndex);
    QCOMPARE(m_fsPanel.m_fsModel->rootPath(), "");

    QCOMPARE(m_fsPanel.onActionAndViewNavigate("path/not/exist", true), false);  // 不存在的路径

    QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);  // 存在的路径
    QCOMPARE(m_stackedBar.m_addressBar->m_addressLine->pathFromLineEdit(), workPath);
    QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);
    const QModelIndex rootSubPathIndex = m_fsPanel.m_fsModel->GetRootIndex();
    QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, false), true);  // 例如: 点击undo/redo 回来的路径, 不入撤销/重做栈, 但是进入路径
    emit m_stackedBar.m_addressBar->m_addressLine->pathActionsTriggeredOrLineEditReturnPressed("", true);  // 路径栏Enter信号已连接
    QCOMPARE(m_fsPanel.m_fsModel->rootPath(), "");
    emit m_stackedBar.m_addressBar->m_addressLine->pathActionsTriggeredOrLineEditReturnPressed(workPath, true);
    QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);
    QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);  // 例如: 双击视图, 路径栏Enter, 点击导航图标 进入的路径

    QCOMPARE(m_fsPanel.m_fsTableView->rootIndex(), rootSubPathIndex);
    QCOMPARE(m_fsPanel.m_fsModel->rootPath(), workPath);

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::LIST);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::LIST);
    QCOMPARE(m_fsPanel.m_fsListView->rootIndex(), rootSubPathIndex);

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TREE);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TREE);
    QCOMPARE(m_fsPanel.m_fsTreeView->rootIndex(), rootSubPathIndex);

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::JSON);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::JSON);
    QCOMPARE(m_fsPanel.m_jsonModel->rootPath(), workPath);
    QCOMPARE(m_fsPanel.onActionAndViewNavigate(emptyWorkPath, true), true);
    QCOMPARE(m_fsPanel.m_jsonModel->rootPath(), emptyWorkPath);
    QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);
    QCOMPARE(m_fsPanel.m_jsonModel->rootPath(), workPath);

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SCENE);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::SCENE);

    QCOMPARE(m_fsPanel.m_scenesModel->rootPath(), workPath);
    QCOMPARE(m_fsPanel.onActionAndViewNavigate(emptyWorkPath, true), true);
    QCOMPARE(m_fsPanel.m_scenesModel->rootPath(), emptyWorkPath);
    QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);
    QCOMPARE(m_fsPanel.m_scenesModel->rootPath(), workPath);
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
    ScenePageControl m_scenePageControl{"Pagination display", &wid};
    CustomStatusBar m_statusBar{&wid};
        NavigationToolBar navigationToolbar{"NavigationToolBar", &wid};
    ViewSwitchHelper m_viewSwitchHelper{&m_stackedBar, &m_fsPanel, &m_scenePageControl, &navigationToolbar, &wid};  // view/searchToolBar switcher
    lo.addWidget(&m_previewFolder);
    lo.addWidget(&m_stackedBar);
    lo.addWidget(&m_fsPanel);
    lo.addWidget(&m_scenePageControl);
    lo.addWidget(&m_statusBar);

    QVERIFY(m_fsPanel._logger == nullptr);
    m_fsPanel.BindLogger(&m_statusBar);
    QVERIFY(m_fsPanel._logger != nullptr);

    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TABLE);  // default
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TABLE);

    const QDir::Filters filters{m_fsPanel.m_fsModel->filter()};

    const int expectFilterIntVal{(QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::Drives | QDir::Filter::Hidden | QDir::Filter::NoDotAndDotDot)};
    QCOMPARE(BehaviorKey::DIR_FILTER_ON_SWITCH_ENABLE.v.toInt(), expectFilterIntVal);
    QCOMPARE((int)filters, expectFilterIntVal);
    QCOMPARE(m_fsPanel.m_fsModel->nameFilterDisables(), false);
    {
      QSignalSpy spy(m_fsPanel.m_fsModel, &QFileSystemModel::directoryLoaded);
      QCOMPARE(m_fsPanel.onActionAndViewNavigate(workPath, true), true);  // 存在的路径
      QCOMPARE(m_fsPanel.on_searchTextChanged("Kaka"), true);
      const QModelIndex rootInd{m_fsPanel.m_fsModel->GetRootIndex()};
      // 目录异步加载完成 whenDirectoryLoaded, onAfterDirectoryLoaded
      QVERIFY(spy.wait(1000));
      QVERIFY(spy.count() >= 1);
      QCOMPARE(spy.takeLast(), (QVariantList{workPath}));
      spy.clear();
      const QString textInStatusBar = m_statusBar.GetText();
      QVERIFY(textInStatusBar.contains("Total 2 item(s) |"));  // "Kaka.jpg" "Cristiano Ronaldo & Kaka.jpg"
      QCOMPARE(m_fsPanel.m_fsModel->rowCount(rootInd), 2);
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
    ScenePageControl m_scenePageControl{"Pagination display", &wid};
    CustomStatusBar m_statusBar{&wid};
    ViewSwitchToolBar m_viewSwitcher{"View Switcher", &m_statusBar};
        NavigationToolBar navigationToolbar{"NavigationToolBar", &wid};
    ViewSwitchHelper m_viewSwitchHelper{&m_stackedBar, &m_fsPanel, &m_scenePageControl, &navigationToolbar, &wid};  // view/searchToolBar switcher
    m_statusBar.addPermanentWidget(&m_viewSwitcher);

    lo.addWidget(&m_previewFolder);
    lo.addWidget(&m_stackedBar);
    lo.addWidget(&m_fsPanel);
    lo.addWidget(&m_scenePageControl);
    lo.addWidget(&m_statusBar);

    // 主动模仿FileXplorer连接
    connect(&m_viewSwitcher, &ViewSwitchToolBar::viewTypeChanged, &m_viewSwitchHelper, &ViewSwitchHelper::onSwitchByViewType);
    m_fsPanel.BindLogger(&m_statusBar);

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
      QCOMPARE(spy.takeLast(), (QVariantList{lvl1Path}));
      spy.clear();
      QCOMPARE(wid.windowTitle(), lvl1Path);  // 标题会更新为最新路径
      QCOMPARE(m_fsPanel.m_fsModel->rowCount(lvl1Ind), 6);

      QKeyEvent backspacePathUpEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
      m_fsPanel.keyPressEvent(&backspacePathUpEvent);
      const QModelIndex lvl0Ind{m_fsPanel.m_fsModel->GetRootIndex()};
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), lvl0Path);
      QVERIFY(spy.wait(1000));
      QCOMPARE(spy.count(), 1);
      QCOMPARE(spy.takeLast(), (QVariantList{lvl0Path}));
      QCOMPARE(wid.windowTitle(), lvl0Path);                // 标题会更新为最新路径
      QCOMPARE(m_fsPanel.m_fsModel->rowCount(lvl0Ind), 1);  // folder[1: lvl1]
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
        QSignalSpy harViewSpy{g_viewActions()._HAR_VIEW, &QAction::triggered};
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
      Configuration().setValue(PathKey::PERFORMER_IMAGEHOST_LOCATE.name, imageHostPath);
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
      g_viewActions()._SCENE_VIEW->setChecked(true);
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::CAST);
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::CAST);
      QCOMPARE(m_fsPanel.m_castDbModel->rowCount(), 1);

      QModelIndex kakaNameInCastModel = m_fsPanel.m_castDbModel->index(0, PERFORMER_DB_HEADER_KEY::Name);
      QCOMPARE(kakaNameInCastModel.data(Qt::DisplayRole).toString(), "Kaka");
      QModelIndex kakaRateInCastModel = m_fsPanel.m_castDbModel->index(0, PERFORMER_DB_HEADER_KEY::Rate);
      QCOMPARE(kakaRateInCastModel.data(Qt::DisplayRole).toInt(), 10);

      QSignalSpy tableViewSpy{g_viewActions()._TABLE_VIEW, &QAction::toggled};
      QCOMPARE(m_fsPanel.on_cellDoubleClicked(kakaNameInCastModel), true);
      QCOMPARE(tableViewSpy.count(), 1);
      QCOMPARE(tableViewSpy.takeLast(), (QVariantList{true}));
      QCOMPARE(m_stackedBar.m_addressBar->m_addressLine->pathFromLineEdit(), lvl1Path);
      QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TABLE);
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), lvl1Path);
    }
  }

  void select_related_ok() {
    const QByteArray kakaJson{
        R"({
  "Name": "Kaka",
  "Studio": "",
  "Cast": []
})"};
    const QByteArray kakaPson{CastPsonFileHelper::CastValues2PsonStr("Kaka", 10, "Son of God", "Football", "lvl0")};
    QList<FsNodeEntry> nodes{
        {"lvl0/Kaka/Kaka.jpg", false, ""},         //
        {"lvl0/Kaka/Kaka.json", false, kakaJson},  //
        {"lvl0/Kaka/Kaka.mp4", false, ""},         //
        {"lvl0/Kaka/Kaka.pson", false, kakaPson},  //
    };
    QCOMPARE(mTDir.createEntries(nodes), 4);
    const QString lvl0Path{mTDir.itemPath("lvl0")};
    const QString lvl1Path{mTDir.itemPath("lvl0/Kaka")};
    QWidget wid;
    QVBoxLayout lo{&wid};
    CurrentRowPreviewer m_previewFolder{&wid};                               // previewer in docker
    StackedAddressAndSearchToolBar m_stackedBar{"searchToolBarTest", &wid};  // searchToolBar
    ViewsStackedWidget m_fsPanel{&m_previewFolder, &wid};                    // main widget
    ScenePageControl m_scenePageControl{"Pagination display", &wid};
    CustomStatusBar m_statusBar{&wid};
    ViewSwitchToolBar m_viewSwitcher{"View Switcher", &m_statusBar};
        NavigationToolBar navigationToolbar{"NavigationToolBar", &wid};
    ViewSwitchHelper m_viewSwitchHelper{&m_stackedBar, &m_fsPanel, &m_scenePageControl, &navigationToolbar, &wid};  // view/searchToolBar switcher
    m_statusBar.addPermanentWidget(&m_viewSwitcher);

    lo.addWidget(&m_previewFolder);
    lo.addWidget(&m_stackedBar);
    lo.addWidget(&m_fsPanel);
    lo.addWidget(&m_scenePageControl);
    lo.addWidget(&m_statusBar);

    // 主动模仿FileXplorer连接
    connect(&m_viewSwitcher, &ViewSwitchToolBar::viewTypeChanged, &m_viewSwitchHelper, &ViewSwitchHelper::onSwitchByViewType);
    m_fsPanel.BindLogger(&m_statusBar);
    m_fsPanel.m_fsModel->sort(0, Qt::AscendingOrder);

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);
    {  // prepare
      QSignalSpy spy(m_fsPanel.m_fsModel, &QFileSystemModel::directoryLoaded);
      QCOMPARE(m_fsPanel.onActionAndViewNavigate(lvl1Path, true), true);  // 存在的路径
      QCOMPARE(m_fsPanel.m_fsModel->rootPath(), lvl1Path);
      QVERIFY(spy.wait(1000));
      QVERIFY(spy.count() >= 1);
      QCOMPARE(spy.takeLast(), (QVariantList{lvl1Path}));
      spy.clear();
      QCOMPARE(m_fsPanel.m_fsModel->rowCount(m_fsPanel.m_fsModel->GetRootIndex()), 4);
      m_fsPanel.m_fsModel->sort(0, Qt::AscendingOrder);
    }

    const QString jpgName{"Kaka.jpg"};
    const QString jsonName{"Kaka.json"};
    const QString mp4Name{"Kaka.mp4"};
    const QString psonName{"Kaka.pson"};
    const QString jpgFileAbsPath{mTDir.itemPath("lvl0/Kaka/Kaka.jpg")};
    const QString jsonFileAbsPath{mTDir.itemPath("lvl0/Kaka/Kaka.json")};
    const QString mp4FileAbsPath{mTDir.itemPath("lvl0/Kaka/Kaka.mp4")};
    const QString psonFileAbsPath{mTDir.itemPath("lvl0/Kaka/Kaka.pson")};
    const QFileInfo jpgFileInfo{jpgFileAbsPath};
    const QFileInfo jsonFileInfo{jsonFileAbsPath};
    const QFileInfo mp4FileInfo{mp4FileAbsPath};
    const QFileInfo castFileInfo{lvl1Path};
    const QStringList filePrepaths{lvl1Path, lvl1Path, lvl1Path, lvl1Path};
    const QStringList fileNames{jpgName, jsonName, mp4Name, psonName};
    const QStringList filePaths{jpgFileAbsPath, jsonFileAbsPath, mp4FileAbsPath, psonFileAbsPath};
    const std::pair<QStringList, QStringList> expectPreAndName{filePrepaths, fileNames};
    const QList<QUrl> expectUrls{QUrl::fromLocalFile(jpgFileAbsPath), QUrl::fromLocalFile(jsonFileAbsPath), QUrl::fromLocalFile(mp4FileAbsPath),
                                 QUrl::fromLocalFile(psonFileAbsPath)};

    // LIST/TABLE/TREE
    {
      const QModelIndex rootIndex{m_fsPanel.m_fsModel->index(lvl1Path)};
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
        QCOMPARE(m_fsPanel.getRootPath(), lvl1Path);
        QVERIFY(!m_fsPanel.hasSelection());

        m_fsPanel.GetCurView()->selectAll();
        QVERIFY(m_fsPanel.hasSelection());

        QModelIndex leftTop, rightDown;
        std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
        QCOMPARE(leftTop, jpgInd);
        QCOMPARE(rightDown.row(), psonInd.row());

        QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
        QCOMPARE(m_fsPanel.getFullRecords().size(), 4);
        QCOMPARE(m_fsPanel.getFileNames(), fileNames);
        QCOMPARE(m_fsPanel.getFilePath(jpgInd), jpgFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePath(jsonInd), jsonFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePath(mp4Ind), mp4FileAbsPath);
        QCOMPARE(m_fsPanel.getFilePath(psonInd), psonFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePaths(), filePaths);
        QCOMPARE(m_fsPanel.getFilePrepaths(), filePrepaths);

        const MimeDataHelper::MimeDataMember& pathsAndUrls = m_fsPanel.getFilePathsAndUrls();
        QCOMPARE(pathsAndUrls.texts, filePaths);
        QCOMPARE(pathsAndUrls.urls, expectUrls);
        QCOMPARE(pathsAndUrls.srcIndexes, selectedRowsIndexes);
        QCOMPARE(m_fsPanel.getFilePrepathsAndName(), expectPreAndName);

        m_fsPanel.getFilePathsAndUrls(Qt::DropAction::CopyAction);

        QCOMPARE(m_fsPanel.getSelectedRowsCount(), 4);
        m_fsPanel.GetCurView()->setCurrentIndex(jpgInd);
        QCOMPARE(m_fsPanel.getCurFilePath(), jpgFileAbsPath);
        QCOMPARE(m_fsPanel.getCurFileName(), jpgName);
        QCOMPARE(m_fsPanel.getFileInfo(jpgInd), jpgFileInfo);
      }

      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::LIST);
      {
        QCOMPARE(m_fsPanel.getRootPath(), lvl1Path);
        QVERIFY(!m_fsPanel.hasSelection());

        m_fsPanel.GetCurView()->selectAll();
        QVERIFY(m_fsPanel.hasSelection());

        QModelIndex leftTop, rightDown;
        std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
        QCOMPARE(leftTop, jpgInd);
        QCOMPARE(rightDown.row(), psonInd.row());

        QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
        QCOMPARE(m_fsPanel.getFullRecords().size(), 4);
        QCOMPARE(m_fsPanel.getFileNames(), fileNames);
        QCOMPARE(m_fsPanel.getFilePath(jpgInd), jpgFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePath(jsonInd), jsonFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePath(jsonInd), jsonFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePath(psonInd), psonFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePaths(), filePaths);
        QCOMPARE(m_fsPanel.getFilePrepaths(), filePrepaths);

        const MimeDataHelper::MimeDataMember& pathsAndUrls = m_fsPanel.getFilePathsAndUrls();
        QCOMPARE(pathsAndUrls.texts, filePaths);
        QCOMPARE(pathsAndUrls.urls, expectUrls);
        QCOMPARE(pathsAndUrls.srcIndexes, selectedRowsIndexes);
        QCOMPARE(m_fsPanel.getFilePrepathsAndName(), expectPreAndName);

        m_fsPanel.getFilePathsAndUrls(Qt::DropAction::CopyAction);

        QCOMPARE(m_fsPanel.getSelectedRowsCount(), 4);
        m_fsPanel.GetCurView()->setCurrentIndex(jpgInd);
        QCOMPARE(m_fsPanel.getCurFilePath(), jpgFileAbsPath);
        QCOMPARE(m_fsPanel.getCurFileName(), jpgName);
        QCOMPARE(m_fsPanel.getFileInfo(jpgInd), jpgFileInfo);
      }

      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TREE);
      {
        m_fsPanel.m_fsTreeView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
        QCOMPARE(m_fsPanel.getRootPath(), lvl1Path);
        QVERIFY(!m_fsPanel.hasSelection());

        m_fsPanel.GetCurView()->selectAll();
        QVERIFY(m_fsPanel.hasSelection());

        QModelIndex leftTop, rightDown;
        std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
        QCOMPARE(leftTop, jpgInd);
        QCOMPARE(rightDown.row(), psonInd.row());

        QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
        QCOMPARE(m_fsPanel.getFullRecords().size(), 4);
        QCOMPARE(m_fsPanel.getFileNames(), fileNames);
        QCOMPARE(m_fsPanel.getFilePath(jpgInd), jpgFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePath(jsonInd), jsonFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePath(jsonInd), jsonFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePath(psonInd), psonFileAbsPath);
        QCOMPARE(m_fsPanel.getFilePaths(), filePaths);
        QCOMPARE(m_fsPanel.getFilePrepaths(), filePrepaths);

        const MimeDataHelper::MimeDataMember& pathsAndUrls = m_fsPanel.getFilePathsAndUrls();
        QCOMPARE(pathsAndUrls.texts, filePaths);
        QCOMPARE(pathsAndUrls.urls, expectUrls);
        QCOMPARE(pathsAndUrls.srcIndexes, selectedRowsIndexes);
        QCOMPARE(m_fsPanel.getFilePrepathsAndName(), expectPreAndName);

        m_fsPanel.getFilePathsAndUrls(Qt::DropAction::CopyAction);

        QCOMPARE(m_fsPanel.getSelectedRowsCount(), 4);
        m_fsPanel.GetCurView()->setCurrentIndex(jpgInd);
        QCOMPARE(m_fsPanel.getCurFilePath(), jpgFileAbsPath);
        QCOMPARE(m_fsPanel.getCurFileName(), jpgName);
        QCOMPARE(m_fsPanel.getFileInfo(jpgInd), jpgFileInfo);
      }
    }

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SEARCH);
    {
      QCOMPARE(m_fsPanel.m_searchSrcModel->rowCount(), 4);
      m_fsPanel.m_advanceSearchView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
      QCOMPARE(m_fsPanel.getRootPath(), lvl1Path);

      const QModelIndex jpgInd{m_fsPanel.m_searchProxyModel->index(0, 0)};
      const QModelIndex jsonInd{m_fsPanel.m_searchProxyModel->index(1, 0)};
      const QModelIndex mp4Ind{m_fsPanel.m_searchProxyModel->index(2, 0)};
      const QModelIndex psonInd{m_fsPanel.m_searchProxyModel->index(3, 0)};
      QCOMPARE(jpgInd.data(Qt::DisplayRole).toString(), "Kaka.jpg");
      QCOMPARE(jsonInd.data(Qt::DisplayRole).toString(), "Kaka.json");
      QCOMPARE(mp4Ind.data(Qt::DisplayRole).toString(), "Kaka.mp4");
      QCOMPARE(psonInd.data(Qt::DisplayRole).toString(), "Kaka.pson");
      const QModelIndexList selectedRowsIndexes{jpgInd, jsonInd, mp4Ind, psonInd};
      const QStringList filePrepathsSearch{lvl1Path + '/', lvl1Path + '/', lvl1Path + '/', lvl1Path + '/'};
      const std::pair<QStringList, QStringList> expectPreAndNameSearch{filePrepathsSearch, fileNames};

      QVERIFY(!m_fsPanel.hasSelection());

      m_fsPanel.GetCurView()->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, jpgInd);
      QCOMPARE(rightDown.row(), psonInd.row());

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 4);
      QCOMPARE(m_fsPanel.getFileNames(), fileNames);
      QCOMPARE(m_fsPanel.getFilePath(jpgInd), jpgFileAbsPath);
      QCOMPARE(m_fsPanel.getFilePath(jsonInd), jsonFileAbsPath);
      QCOMPARE(m_fsPanel.getFilePath(psonInd), psonFileAbsPath);
      QCOMPARE(m_fsPanel.getFilePaths(), filePaths);
      QCOMPARE(m_fsPanel.getFilePrepaths(), filePrepathsSearch);

      const MimeDataHelper::MimeDataMember& pathsAndUrls = m_fsPanel.getFilePathsAndUrls();
      QCOMPARE(pathsAndUrls.texts, filePaths);
      QCOMPARE(pathsAndUrls.urls, expectUrls);
      QCOMPARE(compareModelIndexList(pathsAndUrls.srcIndexes, selectedRowsIndexes), true);
      QCOMPARE(m_fsPanel.getFilePrepathsAndName(), expectPreAndNameSearch);

      m_fsPanel.getFilePathsAndUrls(Qt::DropAction::CopyAction);

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 4);
      m_fsPanel.GetCurView()->setCurrentIndex(jpgInd);
      QCOMPARE(m_fsPanel.getCurFilePath(), jpgFileAbsPath);
      QCOMPARE(m_fsPanel.getCurFileName(), jpgName);
      QCOMPARE(m_fsPanel.getFileInfo(jpgInd), jpgFileInfo);
    }

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::JSON);
    {
      QCOMPARE(m_fsPanel.m_jsonModel->rowCount(), 1);
      m_fsPanel.m_jsonTableView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
      QCOMPARE(m_fsPanel.getRootPath(), lvl1Path);

      const QModelIndex jsonInd{m_fsPanel.m_jsonProxyModel->index(0, JsonKey::Prepath)};
      const QModelIndex jsonNameInd{m_fsPanel.m_jsonProxyModel->index(0, JsonKey::Name)};
      QCOMPARE(jsonNameInd.data(Qt::DisplayRole).toString(), "Kaka");
      const QModelIndexList selectedRowsIndexes{jsonInd};
      const QStringList filePrepathsJson{lvl1Path};
      const QStringList fileNamesJson{jsonName};
      const QStringList filePathsJson{jsonFileAbsPath};
      const std::pair<QStringList, QStringList> expectPreAndNameJson{filePrepathsJson, fileNamesJson};

      QVERIFY(!m_fsPanel.hasSelection());

      m_fsPanel.GetCurView()->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, jsonInd);
      QCOMPARE(rightDown.row(), jsonInd.row());

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 1);
      QCOMPARE(m_fsPanel.getFileNames(), fileNamesJson);
      QCOMPARE(m_fsPanel.getFilePath(jsonInd), jsonFileAbsPath);
      QCOMPARE(m_fsPanel.getFilePaths(), filePathsJson);
      QCOMPARE(m_fsPanel.getFilePrepaths(), filePrepathsJson);

      const MimeDataHelper::MimeDataMember& pathsAndUrls = m_fsPanel.getFilePathsAndUrls();
      QCOMPARE(pathsAndUrls.isEmpty(), true);
      QCOMPARE(m_fsPanel.getFilePrepathsAndName(), expectPreAndNameJson);

      m_fsPanel.getFilePathsAndUrls(Qt::DropAction::CopyAction);

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 1);
      m_fsPanel.GetCurView()->setCurrentIndex(jsonInd);
      QCOMPARE(m_fsPanel.getCurFilePath(), jsonFileAbsPath);
      QCOMPARE(m_fsPanel.getCurFileName(), jsonName);
      QCOMPARE(m_fsPanel.getFileInfo(jsonInd), jsonFileInfo);
    }

    // onSwitchByViewType(CAST)后移到准备好库后
    {
      const QString imageHostPath = mTDir.path();
      Configuration().setValue(PathKey::PERFORMER_IMAGEHOST_LOCATE.name, imageHostPath);
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
        QCOMPARE(castDb.LoadFromPsonFile(imageHostPath), 1);
      }
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::CAST);
      m_fsPanel.m_castTableView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
      QCOMPARE(m_fsPanel.m_castDbModel->rowCount(), 1);
      QCOMPARE(m_fsPanel.getRootPath(), imageHostPath);

      const QModelIndex kakaNameInd{m_fsPanel.m_castDbModel->index(0, PERFORMER_DB_HEADER_KEY::Name)};
      const QModelIndex kakaOriInd{m_fsPanel.m_castDbModel->index(0, PERFORMER_DB_HEADER_KEY::Ori)};
      QCOMPARE(kakaNameInd.data(Qt::DisplayRole).toString(), "Kaka");
      QCOMPARE(kakaOriInd.data(Qt::DisplayRole).toString(), "lvl0");
      const QModelIndexList selectedRowsIndexes{kakaNameInd};
      const QString filePathCast{lvl1Path};
      const QStringList filePathsCast{filePathCast};

      QVERIFY(!m_fsPanel.hasSelection());

      m_fsPanel.GetCurView()->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, kakaNameInd);
      QCOMPARE(rightDown.row(), kakaNameInd.row());

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 0);
      QCOMPARE(m_fsPanel.getFileNames(), (QStringList{}));
      QCOMPARE(m_fsPanel.getFilePath(kakaNameInd), filePathCast);
      QCOMPARE(m_fsPanel.getFilePaths(), filePathsCast);
      QCOMPARE(m_fsPanel.getFilePrepaths(), (QStringList{}));

      const MimeDataHelper::MimeDataMember& pathsAndUrls = m_fsPanel.getFilePathsAndUrls();
      QCOMPARE(pathsAndUrls.isEmpty(), true);
      QCOMPARE(m_fsPanel.getFilePrepathsAndName(), (std::pair<QStringList, QStringList>{}));

      m_fsPanel.getFilePathsAndUrls(Qt::DropAction::CopyAction);

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 1);
      m_fsPanel.GetCurView()->setCurrentIndex(kakaNameInd);
      QCOMPARE(m_fsPanel.getCurFilePath(), filePathCast);
      QCOMPARE(m_fsPanel.getCurFileName(), (QString{}));
      QCOMPARE(m_fsPanel.getFileInfo(kakaNameInd), castFileInfo);
    }

    {  // Preparation:
      const QString dbName{mTDir.itemPath("SelectionOnMovieViewTest.db")};
      const QString connName{"SelectionOnMovieViewTestConn"};
      const QString tableName{"ABCDEF12_3456_7890_ABCDEF1234567890"};  // can be converted to guid
      MOCKER(SystemPath::VIDS_DATABASE).expects(exactly(1)).will(returnValue(dbName));
      MOCKER(SystemPath::MovieDBConnection).expects(exactly(1)).will(returnValue(connName));
      {
        FdBasedDb movieDb{dbName, connName};
        QVERIFY(movieDb.IsValid());
        QVERIFY(movieDb.CreateTable(tableName, FdBasedDb::CREATE_TABLE_TEMPLATE));
        QVERIFY(movieDb.IsTableExist(tableName));
        QVERIFY(movieDb.IsTableEmpty(tableName));
        QCOMPARE(movieDb.ReadADirectory(tableName, lvl1Path), 1);
      }
      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::MOVIE);
      QCOMPARE(m_fsPanel.m_movieDbModel->rowCount(), 1);

      m_fsPanel.m_movieView->sortByColumn(0, Qt::SortOrder::AscendingOrder);
      const QModelIndex movieInd = m_fsPanel.m_movieDbModel->index(0, MOVIE_TABLE::Fd);
      const QModelIndex movieNameInd = m_fsPanel.m_movieDbModel->index(0, MOVIE_TABLE::Name);
      QCOMPARE(movieNameInd.data(Qt::DisplayRole).toString(), mp4Name);
      const QStringList fileNamesMovie{mp4Name};
      const QModelIndexList selectedRowsMovie{movieInd};
      const QStringList filePathsMovie{mp4FileAbsPath};
      const QStringList filePrepathsMovie{lvl1Path};
      const QList<QUrl> expectUrls{QUrl::fromLocalFile(mp4FileAbsPath)};
      const std::pair<QStringList, QStringList> expectPreAndNameMovie{filePrepathsMovie, fileNamesMovie};

      QCOMPARE(m_fsPanel.getRootPath(), "");  // no rootpath in MOVIE
      QVERIFY(!m_fsPanel.hasSelection());

      m_fsPanel.GetCurView()->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, movieInd);
      QCOMPARE(rightDown.row(), movieInd.row());

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsMovie));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 1);
      QCOMPARE(m_fsPanel.getFileNames(), fileNamesMovie);
      QCOMPARE(m_fsPanel.getFilePath(movieInd), mp4FileAbsPath);
      QCOMPARE(m_fsPanel.getFilePaths(), filePathsMovie);
      QCOMPARE(m_fsPanel.getFilePrepaths(), filePrepathsMovie);

      const MimeDataHelper::MimeDataMember& pathsAndUrls = m_fsPanel.getFilePathsAndUrls();
      QCOMPARE(pathsAndUrls.texts, filePathsMovie);
      QCOMPARE(pathsAndUrls.urls, expectUrls);
      QCOMPARE(pathsAndUrls.srcIndexes, selectedRowsMovie);
      QCOMPARE(m_fsPanel.getFilePrepathsAndName(), expectPreAndNameMovie);

      m_fsPanel.getFilePathsAndUrls(Qt::DropAction::CopyAction);

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 1);
      m_fsPanel.GetCurView()->setCurrentIndex(movieInd);
      QCOMPARE(m_fsPanel.getCurFilePath(), mp4FileAbsPath);
      QCOMPARE(m_fsPanel.getCurFileName(), mp4Name);
      QCOMPARE(m_fsPanel.getFileInfo(movieInd), mp4FileInfo);
    }

    {
      using namespace SceneInfoManager;
      ScnMgr scnMgr1;
      scnMgr1(lvl1Path);
      QCOMPARE(ScnMgr::UpdateScnFiles(lvl1Path), 1);  // 1 scn file[sub.scn] writed ok
      QVERIFY(mTDir.exists("lvl0/Kaka/Kaka.scn"));

      m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SCENE);
      QCOMPARE(m_fsPanel.m_scenesModel->rowCount(), 1);
      QCOMPARE(m_fsPanel.m_sceneProxyModel->rowCount(), 1);
      m_fsPanel.m_sceneProxyModel->sort((int)SceneInfo::Role::REL_PATH_ROLE, Qt::AscendingOrder);

      QCOMPARE(m_fsPanel.getRootPath(), lvl1Path);
      QVERIFY(!m_fsPanel.hasSelection());

      m_fsPanel.GetCurView()->selectAll();
      QVERIFY(m_fsPanel.hasSelection());

      const QModelIndex kakaScnInd{m_fsPanel.m_sceneProxyModel->index(0, 0)};
      QCOMPARE(kakaScnInd.data(Qt::DisplayRole).toString(), "Kaka");
      const QModelIndexList selectedRowsIndexes{kakaScnInd};
      const QString fileName{"Kaka.mp4"};
      const QStringList fileNames{fileName};
      const QString filePath{mTDir.itemPath("lvl0/Kaka/Kaka.mp4")};
      const QStringList filePaths{filePath};
      const QStringList filePrepaths{lvl1Path + '/'};

      QModelIndex leftTop, rightDown;
      std::tie(leftTop, rightDown) = m_fsPanel.getTopLeftAndRightDownRectangleIndex();
      QCOMPARE(leftTop, kakaScnInd);
      QCOMPARE(rightDown.row(), kakaScnInd.row());

      QVERIFY(compareModelIndexList(m_fsPanel.getSelectedRows(), selectedRowsIndexes));
      QCOMPARE(m_fsPanel.getFullRecords().size(), 0);
      QCOMPARE(m_fsPanel.getFileNames(), fileNames);
      QCOMPARE(m_fsPanel.getFilePath(kakaScnInd), filePath);
      QCOMPARE(m_fsPanel.getFilePaths(), filePaths);
      QCOMPARE(m_fsPanel.getFilePrepaths(), filePrepaths);

      const MimeDataHelper::MimeDataMember& pathsAndUrls = m_fsPanel.getFilePathsAndUrls();
      QVERIFY(pathsAndUrls.isEmpty());
      QCOMPARE(m_fsPanel.getFilePrepathsAndName(), (std::pair<QStringList, QStringList>{}));

      m_fsPanel.getFilePathsAndUrls(Qt::DropAction::CopyAction);

      QCOMPARE(m_fsPanel.getSelectedRowsCount(), 1);
      m_fsPanel.GetCurView()->setCurrentIndex(kakaScnInd);
      QCOMPARE(m_fsPanel.getCurFilePath(), filePath);
      QCOMPARE(m_fsPanel.getCurFileName(), fileName);
      QCOMPARE(m_fsPanel.getFileInfo(kakaScnInd), mp4FileInfo);
    }
  }
};

#include "ViewsStackedWidgetTest.moc"
REGISTER_TEST(ViewsStackedWidgetTest, false)
