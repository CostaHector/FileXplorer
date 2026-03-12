#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "FolderPreviewSwitcher.h"
#include "EndToExposePrivateMember.h"

class FolderPreviewSwitcherTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void default_ok() {
    FolderPreviewSwitcher sw{nullptr, nullptr};
    QVERIFY(!sw.onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E::NONE));
    QVERIFY(!sw.onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY));
    QVERIFY(!sw.onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD));
    QVERIFY(!sw.onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL));
  }

  void switch_by_viewtype_ok() {
    CurrentRowPreviewer previewer;
    FolderPreviewSwitcher sw{&previewer, nullptr};
    QVERIFY(!sw.onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E::NONE));
    QVERIFY(sw.onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY));
    QVERIFY(sw.onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD));
    QVERIFY(sw.onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL));
  }
};

#include "FolderPreviewSwitcherTest.moc"
REGISTER_TEST(FolderPreviewSwitcherTest, false)
