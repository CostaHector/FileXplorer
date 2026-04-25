#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "MemoryKey.h"
#include "Configuration.h"
#include "BeginToExposePrivateMember.h"
#include "FileXplorer.h"
#include "EndToExposePrivateMember.h"

#include "ViewActions.h"

#include <QFileInfo>

class FileXplorerTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {  //
    Configuration().clear();
  }

  void cleanupTestCase() {  //
    Configuration().clear();
  }

  void GetInitialPathFromArgs_ok() {
    // path specified and valid, use it
    const QString validPath{QFileInfo{__FILE__}.absolutePath()};
    const QString inValidPath{validPath + "/invalid"};
    {  // precondition:
      QCOMPARE(QFileInfo{inValidPath}.absolutePath(), validPath);
    }

    {  // basic expectation
      Configuration().setValue(PathKey::STARTUP_PATH.name, "");
      QStringList specifiedValidPath{"a.exe", validPath};
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(specifiedValidPath), validPath);
    }

    {  // special path:
      Configuration().setValue(PathKey::STARTUP_PATH.name, inValidPath);
      QStringList emptyPath{"a.exe", ""};
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(emptyPath), "");

#ifdef __linux__
      Configuration().setValue(PathKey::STARTUP_PATH.name, inValidPath);
      QStringList slashPath{"a.exe", "/"};
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(slashPath), "/");
#endif
    }

    {  // path specified but invalid
      QStringList specifiedInvalidPath{"a.exe", "invalid/path/to/folder"};
      // when last time path still valid
      Configuration().setValue(PathKey::STARTUP_PATH.name, validPath);
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(specifiedInvalidPath), validPath);
      //  when last time path invalid ->parent
      Configuration().setValue(PathKey::STARTUP_PATH.name, inValidPath);
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(specifiedInvalidPath), validPath);
    }

    {  // no path specified
      const QStringList noPathSpecified{"a.exe"};
      // when last time path is still valid
      Configuration().setValue(PathKey::STARTUP_PATH.name, validPath);
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(noPathSpecified), validPath);
      // when last time path invalid->parent
      Configuration().setValue(PathKey::STARTUP_PATH.name, inValidPath);
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(noPathSpecified), validPath);
    }
  }

  void default_status_ok() {
    Configuration().clear();

    const QString existPath{QFileInfo{__FILE__}.absolutePath()};
    QStringList args{"a.exe", existPath};

    QCOMPARE(Configuration().value(CompoVisKey::SHOW_NAVIGATION_SIDEBAR.name, CompoVisKey::SHOW_NAVIGATION_SIDEBAR.toVariant()).toBool(), true);
    QCOMPARE(Configuration().value(CompoVisKey::SHOW_PREVIEW_DOCKER.name, CompoVisKey::SHOW_PREVIEW_DOCKER.toVariant()).toBool(), true);
    QCOMPARE(Configuration().value(CompoVisKey::FOLDER_PREVIEW_TYPE.name, CompoVisKey::FOLDER_PREVIEW_TYPE.toVariant()).toInt(),
             ((int)PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY));

    auto& viewActInst = g_viewActions();
    viewActInst._TABLE_VIEW->setChecked(true);

    // no configuration
    {
      FileXplorer fe{args, nullptr};
      QVERIFY(fe.m_viewSwitcher != nullptr);
      QVERIFY(fe.m_scenePageControl != nullptr);
      QVERIFY(fe.m_previewHtmlDock != nullptr);
      QVERIFY(fe.m_previewFolder != nullptr);
      QVERIFY(fe.m_previewSwitcher != nullptr);
      QVERIFY(fe.m_fsPanel != nullptr);
      QVERIFY(fe.m_stackedBar != nullptr);
      QVERIFY(fe.m_viewSwitchHelper != nullptr);
      QVERIFY(fe.m_navigationToolBar != nullptr);
      QVERIFY(fe.m_naviSideBarDock != nullptr);
      QVERIFY(fe.m_ribbonMenu != nullptr);
      QVERIFY(fe.m_statusBar != nullptr);

      QVERIFY(fe.m_fsPanel->m_fsModel != nullptr);
      QCOMPARE(fe.m_fsPanel->m_fsModel->rootPath(), existPath);

      fe.showEvent(nullptr);
      fe.closeEvent(nullptr);
      fe.keyPressEvent(nullptr);
      {
        QCOMPARE(viewActInst._NAVIGATION_PANE->isChecked(), true);
        QCOMPARE(fe.m_naviSideBarDock->isHidden(), false);

        QCOMPARE(viewActInst._PREVIEW_PANEL->isChecked(), true);
        QCOMPARE(fe.m_previewHtmlDock->isHidden(), false);
        QCOMPARE(fe.m_previewFolder->isHidden(), false);

        QCOMPARE(viewActInst._TABLE_VIEW->isChecked(), true);
        QCOMPARE(fe.m_fsPanel->GetVt(), ViewTypeTool::ViewType::TABLE);
        QCOMPARE(fe.m_fsPanel->count(), 1);

        QCOMPARE(fe.m_previewHtmlDock->CATEGORY_PRE->isChecked(), true);
        QCOMPARE(fe.m_previewFolder->GetCurrentViewE(), PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY);
      }

      // change status
      viewActInst._NAVIGATION_PANE->toggle();
      QCOMPARE(fe.m_naviSideBarDock->isHidden(), true);

      viewActInst._PREVIEW_PANEL->toggle();
      QCOMPARE(fe.m_previewHtmlDock->isHidden(), true);
      QCOMPARE(fe.m_previewFolder->isHidden(), true);

      viewActInst._LIST_VIEW->setChecked(true);
      emit viewActInst._LIST_VIEW->triggered();
      QCOMPARE(fe.m_fsPanel->GetVt(), ViewTypeTool::ViewType::LIST);
      QCOMPARE(fe.m_fsPanel->count(), 2);

      fe.m_previewHtmlDock->CAROUSEL_PRE->setChecked(true);
      emit fe.m_previewHtmlDock->CAROUSEL_PRE->triggered();
      QCOMPARE(fe.m_previewFolder->GetCurrentViewE(), PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL);

      QShowEvent showEvent;
      fe.showEvent(&showEvent);

      QCloseEvent closeEvent;
      fe.closeEvent(&closeEvent);
    }

    // show navi/preview action checked configuration will saved when static global variable ViewActions get destructed
    // here we change it manually only to see if it work for the new window
    Configuration().setValue(CompoVisKey::SHOW_NAVIGATION_SIDEBAR.name, false);
    Configuration().setValue(CompoVisKey::SHOW_PREVIEW_DOCKER.name, false);

    QCOMPARE(Configuration().value(CompoVisKey::FOLDER_PREVIEW_TYPE.name, CompoVisKey::FOLDER_PREVIEW_TYPE.toVariant()).toInt(),
             ((int)PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL));
    // configuration read and used
    viewActInst._TABLE_VIEW->setChecked(true);
    {
      FileXplorer fe{args, nullptr};
      QCOMPARE(fe.m_naviSideBarDock->isHidden(), true);
      QCOMPARE(fe.m_previewHtmlDock->isHidden(), true);
      QCOMPARE(fe.m_previewFolder->isHidden(), true);
      QCOMPARE(fe.m_fsPanel->GetVt(), ViewTypeTool::ViewType::TABLE);
      QCOMPARE(fe.m_fsPanel->count(), 1);
      QCOMPARE(fe.m_previewFolder->GetCurrentViewE(), PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL);

      {
        // F3: Search Get Focus ok
        QKeyEvent f3Event(QEvent::KeyPress, Qt::Key_F3, Qt::NoModifier);
        fe.keyPressEvent(&f3Event);

        viewActInst._ADVANCE_SEARCH_VIEW->setChecked(true);
        emit viewActInst._ADVANCE_SEARCH_VIEW->triggered();
        QCOMPARE(fe.m_fsPanel->GetVt(), ViewTypeTool::ViewType::SEARCH);
        fe.keyPressEvent(&f3Event);

        viewActInst._MOVIE_VIEW->setChecked(true);
        emit viewActInst._MOVIE_VIEW->triggered();
        QCOMPARE(fe.m_fsPanel->GetVt(), ViewTypeTool::ViewType::MOVIE);
        fe.keyPressEvent(&f3Event);

        viewActInst._CAST_VIEW->setChecked(true);
        emit viewActInst._CAST_VIEW->triggered();
        QCOMPARE(fe.m_fsPanel->GetVt(), ViewTypeTool::ViewType::CAST);
        fe.keyPressEvent(&f3Event);

        // Escape: Panel Get Focus ok
        QKeyEvent escapeEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
        fe.keyPressEvent(&escapeEvent);
      }
    }
  }
};

#include "FileXplorerTest.moc"
REGISTER_TEST(FileXplorerTest, false)
