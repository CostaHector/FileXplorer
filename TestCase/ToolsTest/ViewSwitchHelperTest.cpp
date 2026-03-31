#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ViewSwitchHelper.h"
#include "EndToExposePrivateMember.h"

#include <QFileInfo>

class ViewSwitchHelperTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void default_ok() {
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

    const QString existPath{QFileInfo{__FILE__}.absolutePath()};

    // will not crash
    QVERIFY(m_fsPanel.m_fsListView == nullptr);
    QVERIFY(m_stackedBar.m_addressBar == nullptr);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::LIST);
    QVERIFY(m_stackedBar.m_addressBar != nullptr);
    QVERIFY(m_stackedBar.m_addressBar->m_addressLine != nullptr);
    QVERIFY(m_fsPanel.m_fsListView != nullptr);
    QCOMPARE(m_fsPanel.onActionAndViewNavigate(existPath, true), true);
    QCOMPARE(m_fsPanel.currentWidget(), m_fsPanel.m_fsListView);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::LIST);
    QCOMPARE(m_fsPanel.GetCurViewName(), "LIST");

    QVERIFY(m_fsPanel.m_fsTableView == nullptr);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);
    QVERIFY(m_fsPanel.m_fsTableView != nullptr);
    QCOMPARE(m_fsPanel.currentWidget(), m_fsPanel.m_fsTableView);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TABLE);
    QCOMPARE(m_fsPanel.GetCurViewName(), "TABLE");

    QVERIFY(m_fsPanel.m_fsTreeView == nullptr);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TREE);
    QVERIFY(m_fsPanel.m_fsTreeView != nullptr);
    QCOMPARE(m_fsPanel.currentWidget(), m_fsPanel.m_fsTreeView);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::TREE);
    QCOMPARE(m_fsPanel.GetCurViewName(), "TREE");

    QVERIFY(m_fsPanel.m_advanceSearchView == nullptr);
    QVERIFY(m_fsPanel.m_searchSrcModel == nullptr);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SEARCH);
    QVERIFY(m_fsPanel.m_searchSrcModel != nullptr);
    QVERIFY(m_fsPanel.m_advanceSearchView != nullptr);
    QCOMPARE(m_fsPanel.currentWidget(), m_fsPanel.m_advanceSearchView);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::SEARCH);
    QCOMPARE(m_fsPanel.GetCurViewName(), "SEARCH");

    QVERIFY(m_fsPanel.m_movieView == nullptr);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::MOVIE);
    QVERIFY(m_fsPanel.m_movieView != nullptr);
    QCOMPARE(m_fsPanel.currentWidget(), m_fsPanel.m_movieView);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::MOVIE);
    QCOMPARE(m_fsPanel.GetCurViewName(), "MOVIE");

    QVERIFY(m_fsPanel.m_sceneTableView == nullptr);
    QVERIFY(m_fsPanel.m_scenesModel == nullptr);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SCENE);
    QVERIFY(m_fsPanel.m_scenesModel != nullptr);
    QVERIFY(m_fsPanel.m_sceneTableView != nullptr);
    QCOMPARE(m_fsPanel.currentWidget(), m_fsPanel.m_sceneTableView);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::SCENE);
    QCOMPARE(m_fsPanel.GetCurViewName(), "SCENE");

    QVERIFY(m_fsPanel.m_castTableView == nullptr);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::CAST);
    QVERIFY(m_fsPanel.m_castTableView != nullptr);
    QCOMPARE(m_fsPanel.currentWidget(), m_fsPanel.m_castTableView);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::CAST);
    QCOMPARE(m_fsPanel.GetCurViewName(), "CAST");

    QVERIFY(m_fsPanel.m_jsonTableView == nullptr);
    QVERIFY(m_fsPanel.m_jsonModel == nullptr);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::JSON);
    QVERIFY(m_fsPanel.m_jsonModel != nullptr);
    QVERIFY(m_fsPanel.m_jsonTableView != nullptr);
    QCOMPARE(m_fsPanel.currentWidget(), m_fsPanel.m_jsonTableView);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::JSON);
    QCOMPARE(m_fsPanel.GetCurViewName(), "JSON");

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::VIEW_TYPE_BUTT);
    QCOMPARE(m_fsPanel.GetVt(), ViewTypeTool::ViewType::JSON);

    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::LIST);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TABLE);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::TREE);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SEARCH);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::MOVIE);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::SCENE);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::CAST);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::JSON);
    m_viewSwitchHelper.onSwitchByViewType(ViewTypeTool::ViewType::VIEW_TYPE_BUTT);
  }
};

#include "ViewSwitchHelperTest.moc"
REGISTER_TEST(ViewSwitchHelperTest, false)
