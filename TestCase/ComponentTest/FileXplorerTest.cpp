#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QSignalSpy>

#include "MemoryKey.h"
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
      Configuration().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, "");
      QStringList specifiedValidPath{"a.exe", validPath};
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(specifiedValidPath), validPath);
    }

    {  // special path:
      Configuration().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, inValidPath);
      QStringList emptyPath{"a.exe", ""};
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(emptyPath), "");

#ifdef __linux__
      Configuration().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, inValidPath);
      QStringList slashPath{"a.exe", "/"};
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(slashPath), "/");
#endif
    }

    {  // path specified but invalid
      QStringList specifiedInvalidPath{"a.exe", "invalid/path/to/folder"};
      // when last time path still valid
      Configuration().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, validPath);
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(specifiedInvalidPath), validPath);
      //  when last time path invalid ->parent
      Configuration().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, inValidPath);
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(specifiedInvalidPath), validPath);
    }

    {  // no path specified
      const QStringList noPathSpecified{"a.exe"};
      // when last time path is still valid
      Configuration().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, validPath);
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(noPathSpecified), validPath);
      // when last time path invalid->parent
      Configuration().setValue(MemoryKey::DEFAULT_OPEN_PATH.name, inValidPath);
      QCOMPARE(FileXplorer::GetInitialPathFromArgs(noPathSpecified), validPath);
    }
  }

  void default_status_ok() {
    Configuration().clear();

    const QString existPath{QFileInfo{__FILE__}.absolutePath()};
    QStringList args{"a.exe", existPath};

    QCOMPARE(Configuration().value(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.v).toBool(), true);
    QCOMPARE(Configuration().value(MemoryKey::SHOW_FLOATING_PREVIEW.name, MemoryKey::SHOW_FLOATING_PREVIEW.v).toBool(), true);
    QCOMPARE(Configuration().value(MemoryKey::FOLDER_PREVIEW_TYPE.name, MemoryKey::FOLDER_PREVIEW_TYPE.v).toInt(),
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
      QVERIFY(fe.m_ribbonMenu != nullptr);
      QVERIFY(fe.m_statusBar != nullptr);

      QVERIFY(fe.m_fsPanel->m_fsModel != nullptr);
      QCOMPARE(fe.m_fsPanel->m_fsModel->rootPath(), existPath);

      fe.showEvent(nullptr);
      fe.closeEvent(nullptr);
      fe.keyPressEvent(nullptr);
      {
        QCOMPARE(viewActInst._NAVIGATION_PANE->isChecked(), true);
        QCOMPARE(fe.m_navigationToolBar->isHidden(), false);

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
      QCOMPARE(fe.m_navigationToolBar->isHidden(), true);

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
    Configuration().setValue(MemoryKey::SHOW_QUICK_NAVIGATION_TOOL_BAR.name, false);
    Configuration().setValue(MemoryKey::SHOW_FLOATING_PREVIEW.name, false);

    QCOMPARE(Configuration().value(MemoryKey::FOLDER_PREVIEW_TYPE.name, MemoryKey::FOLDER_PREVIEW_TYPE.v).toInt(),
             ((int)PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL));
    // configuration read and used
    viewActInst._TABLE_VIEW->setChecked(true);
    {
      FileXplorer fe{args, nullptr};
      QCOMPARE(fe.m_navigationToolBar->isHidden(), true);
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
