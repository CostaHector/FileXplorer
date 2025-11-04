#include <QtTest/QtTest>
#include <QTestEventList>

#include "PlainTestSuite.h"
#include "TDir.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "FileFolderPreviewer.h"
#include "EndToExposePrivateMember.h"
#include "SqlRecordTestHelper.h"

class FileFolderPreviewerTest : public PlainTestSuite {
  Q_OBJECT
public:

private slots:
  void toggle_panel_type_ok() {
    TDir mTDir;
    QDir mDir {mTDir.path()};
    const QList<FsNodeEntry> nodes { {"AnyPath/AnyFile.json", false, ""} };
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());

    FileFolderPreviewer previewer{"TestFileFolderPreviewer"};
    QVERIFY(previewer.mImgVidOtherPane != nullptr);
    QVERIFY(previewer.mDetailsPane != nullptr);

    // by default: Detail Panel
    QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::DETAIL));
    QCOMPARE(previewer.currentWidget(), previewer.mDetailsPane);

    // 1. cast view sqlRecord: Detail Panel
    QSqlRecord aCastRecord;
    previewer(aCastRecord, "");
    QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::DETAIL));
    QCOMPARE(previewer.currentWidget(), previewer.mDetailsPane);


    // 2. file system view: folder->mImgVidOtherPane, file->mDetailsPane
    QString folderAbsPath = mDir.absoluteFilePath("AnyPath");
    previewer(folderAbsPath);
    QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::IMG_VID_OTH));
    QCOMPARE(previewer.currentWidget(), previewer.mImgVidOtherPane);

    QString fileAbsPath = mDir.absoluteFilePath("AnyPath/AnyFile.json");
    previewer(fileAbsPath);
    QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::DETAIL));
    QCOMPARE(previewer.currentWidget(), previewer.mDetailsPane);

    // 3. scene view: folder->mImgVidOtherPane
    const QString name = "AnyFile";
    const QString jsonAbsPath = name + ".json";
    const QStringList imgPthLst {mDir.absoluteFilePath("AnyPath/AnyFile.json")};
    const QStringList vidsLst;
    previewer(name, jsonAbsPath, imgPthLst, vidsLst);
    QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::IMG_VID_OTH));
    QCOMPARE(previewer.currentWidget(), previewer.mImgVidOtherPane);

    // 4. empty file system path no need update. panel unchange
    QString emptyStrPath = "";
    previewer(emptyStrPath);
    QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::IMG_VID_OTH));
    QCOMPARE(previewer.currentWidget(), previewer.mImgVidOtherPane);

    // 5. show cast record in detail view
    const QSqlRecord validRecord = SqlRecordTestHelper::GetACastRecordLine("Michael Fassbender", "X-Man", {});
    const QString imgHostNotExist;
    previewer(validRecord, imgHostNotExist);
    QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::DETAIL));
    QCOMPARE(previewer.currentWidget(), previewer.mDetailsPane);
  }

  void test_configuration_saved() {
    QVERIFY(Configuration().contains("FLOATING_PREVIEW_GEOMETRY"));
    FileFolderPreviewer previewer{"TestFileFolderPreviewer secondtime"};

    QVERIFY(previewer.mImgVidOtherPane != nullptr);
    const QStringList othersLst;
    previewer.UpdateImgs("random images", othersLst); // should not crash down
    previewer.UpdateVids(othersLst); // should not crash down
    previewer.UpdateOthers(othersLst); // should not crash down
  }
};

#include "FileFolderPreviewerTest.moc"
REGISTER_TEST(FileFolderPreviewerTest, false)
