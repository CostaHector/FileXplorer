#include <QtTest/QtTest>

#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "CurrentRowPreviewer.h"
#include "EndToExposePrivateMember.h"

class CurrentRowPreviewerTest : public PlainTestSuite {
  Q_OBJECT
public:

private slots:
  void toggle_preview_widget_ok() {
    CurrentRowPreviewer previewer;
    QVERIFY(previewer.isTimerDisabled()); // in test case timer is disabled
    const QSize initalSizeHint = previewer.sizeHint();
    QVERIFY(initalSizeHint.isValid());

    QVERIFY(previewer.m_fileFolderPreviewStackedWid == nullptr);
    QVERIFY(previewer.m_imgInFolderBrowser == nullptr);
    QVERIFY(previewer.m_imgInFolderLabels == nullptr);

    // all need init
    QCOMPARE(previewer.NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY), true);
    QCOMPARE(previewer.NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD), true);
    QCOMPARE(previewer.NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL), true);

    // init ok, not nullptr, not need init again
    QCOMPARE(previewer.InitPreviewAndAddView(PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY), true);
    QVERIFY(previewer.m_fileFolderPreviewStackedWid != nullptr);
    QCOMPARE(previewer.NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY), false);

    QCOMPARE(previewer.InitPreviewAndAddView(PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD), true);
    QVERIFY(previewer.m_imgInFolderBrowser != nullptr);
    QCOMPARE(previewer.NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD), false);

    QCOMPARE(previewer.InitPreviewAndAddView(PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL), true);
    QVERIFY(previewer.m_imgInFolderLabels != nullptr);
    QCOMPARE(previewer.NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL), false);

    // toggle preview widget ok
    QCOMPARE(previewer.setCurrentPreviewType(PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY), true);
    QCOMPARE(previewer.currentWidget(), previewer.m_fileFolderPreviewStackedWid);

    QCOMPARE(previewer.setCurrentPreviewType(PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD), true);
    QCOMPARE(previewer.currentWidget(), previewer.m_imgInFolderBrowser);

    QCOMPARE(previewer.setCurrentPreviewType(PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL), true);
    QCOMPARE(previewer.currentWidget(), previewer.m_imgInFolderLabels);
  }

  void toggle_src_from_ok() {
    CurrentRowPreviewer previewer;
    QVERIFY(previewer.isTimerDisabled()); // in test case timer is disabled
    QCOMPARE(previewer.NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY), true);
    QCOMPARE(previewer.InitPreviewAndAddView(PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY), true);
    QCOMPARE(previewer.setCurrentPreviewType(PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY), true);
    QVERIFY(previewer.m_fileFolderPreviewStackedWid != nullptr);

    // 1. FILE_SYSTEM_VIEW
    QString testPath = "/test/path";
    previewer.DisplayFileSystemRecord(testPath);
    QCOMPARE(previewer.mCurrentSrcFrom, CurrentRowPreviewer::SRC_FROM::FILE_SYSTEM_VIEW);
    QCOMPARE(previewer.GetCurPath(), testPath);

    // 2. CAST
    QSqlRecord record;
    QString imageHost = "hostpath";
    previewer.DisplayCastRecord(record, imageHost);
    QCOMPARE(previewer.mCurrentSrcFrom, CurrentRowPreviewer::SRC_FROM::CAST_DB);

    // 3. SCENE
    QString name = "Scene1";
    QString jsonAbsPath = name + ".json";
    QStringList imgPthLst;
    QStringList vidsLst;
    previewer.DisplayJsonRecord(name, jsonAbsPath, imgPthLst, vidsLst);
    QCOMPARE(previewer.mCurrentSrcFrom, CurrentRowPreviewer::SRC_FROM::JSON_OR_SCENE);

    // 4. multi toggle ok
    previewer.DisplayCastRecord(record, "another_host_path");
    QCOMPARE(previewer.mCurrentSrcFrom, CurrentRowPreviewer::SRC_FROM::CAST_DB);

    previewer.DisplayMovieRecord(record);
    QCOMPARE(previewer.mCurrentSrcFrom, CurrentRowPreviewer::SRC_FROM::MOVIE_DB);

    previewer.DisplayJsonRecord("Scene2", jsonAbsPath, {}, {});
    QCOMPARE(previewer.mCurrentSrcFrom, CurrentRowPreviewer::SRC_FROM::JSON_OR_SCENE);

    QCOMPARE(previewer.NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD), true);
    QCOMPARE(previewer.InitPreviewAndAddView(PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD), true);
    QCOMPARE(previewer.setCurrentPreviewType(PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD), true);
    QVERIFY(previewer.m_imgInFolderBrowser != nullptr);
    previewer.DisplayFileSystemRecord("/another/pathProgressiveLoad");
    QCOMPARE(previewer.mCurrentSrcFrom, CurrentRowPreviewer::SRC_FROM::FILE_SYSTEM_VIEW);

    QCOMPARE(previewer.NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL), true);
    QCOMPARE(previewer.InitPreviewAndAddView(PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL), true);
    QCOMPARE(previewer.setCurrentPreviewType(PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL), true);
    QVERIFY(previewer.m_imgInFolderLabels != nullptr);
    previewer.DisplayFileSystemRecord("/another/pathCarousel");
    QCOMPARE(previewer.mCurrentSrcFrom, CurrentRowPreviewer::SRC_FROM::FILE_SYSTEM_VIEW);
  }
};

#include "CurrentRowPreviewerTest.moc"
REGISTER_TEST(CurrentRowPreviewerTest, false)
