#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "FileFolderPreviewer.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"
#include "Configuration.h"
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

    // 1. cast view(sqlRecord, qstring)/movie db view(sqlRecord). using Detail Panel
    {
      QSqlRecord aCastRecord = SqlRecordTestHelper::GetACastRecordLine("Michael Fassbender", "X-Man", {});;
      QVERIFY(previewer.DisplayCastInformation(aCastRecord, ""));
      QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::DETAIL));
      QCOMPARE(previewer.currentWidget(), previewer.mDetailsPane);
    }

    // 2. file system view: folder->mImgVidOtherPane, file->mDetailsPane
    {
      QString folderAbsPath = mDir.absoluteFilePath("AnyPath");
      QVERIFY(previewer.DisplayFileInformation(folderAbsPath));
      QCOMPARE(previewer.mLastName, folderAbsPath);
      QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::IMG_VID_OTH));
      QCOMPARE(previewer.currentWidget(), previewer.mImgVidOtherPane);

      QString fileAbsPath = mDir.absoluteFilePath("AnyPath/AnyFile.json");
      QVERIFY(previewer.DisplayFileInformation(fileAbsPath));
      QCOMPARE(previewer.mLastName, fileAbsPath);
      QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::DETAIL));
      QCOMPARE(previewer.currentWidget(), previewer.mDetailsPane);
    }

    // 3. json/scene view: folder->mImgVidOtherPane
    {
      const QString name = "AnyFile";
      const QString jsonAbsPath = name + ".json";
      const QStringList imgPthLst {mDir.absoluteFilePath("AnyPath/AnyFile.json")};
      const QStringList vidsLst;
      QVERIFY(previewer.DisplayJsonInformation(name, jsonAbsPath, imgPthLst, vidsLst));
      QCOMPARE(previewer.mLastName, name);
      QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::IMG_VID_OTH));
      QCOMPARE(previewer.currentWidget(), previewer.mImgVidOtherPane);
    }

    // 4. empty file system path no need update. panel unchange
    {
      QString emptyStrPath = "";
      QVERIFY(!previewer.DisplayFileInformation(emptyStrPath));
      QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::IMG_VID_OTH));
      QCOMPARE(previewer.currentWidget(), previewer.mImgVidOtherPane);
    }

    // 5. show cast record in detail view
    {
      const QString imgHostNotExist;
      const QSqlRecord michaelFassbenderRecord = SqlRecordTestHelper::GetACastRecordLine("Michael Fassbender", "X-Man", {});
      QVERIFY(previewer.mLastName != "Michael Fassbender");
      QVERIFY(previewer.DisplayCastInformation(michaelFassbenderRecord, imgHostNotExist)); // Name changed
      QCOMPARE(previewer.mLastName, "Michael Fassbender");
      QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::DETAIL));
      QCOMPARE(previewer.currentWidget(), previewer.mDetailsPane);
      QVERIFY(!previewer.DisplayCastInformation(michaelFassbenderRecord, imgHostNotExist)); // Name still Michael Fassbender. unchange

      const QSqlRecord kakaRecord = SqlRecordTestHelper::GetACastRecordLine("Kaka", "Real Madrid", {});
      QVERIFY(previewer.DisplayCastInformation(kakaRecord, imgHostNotExist)); // Name changed
      QCOMPARE(previewer.mLastName, "Kaka");
    }

    // 6. show movie record in detail view
    {
      const QString imgHostNotExist;
      const QSqlRecord cMichaelFassbenderRecord = SqlRecordTestHelper::GetAMovieRecordFromDb("01234567890123456789012345678901", "", "C:/X-MEN/",  "Michael Fassbender.mp4", 1024);
      QVERIFY(previewer.DisplayMovieInformation(cMichaelFassbenderRecord)); // Name+pathHash changed
      QCOMPARE(previewer.currentIndex(), static_cast<int>(FileFolderPreviewer::PANE_TYPE::DETAIL));
      QCOMPARE(previewer.currentWidget(), previewer.mDetailsPane);
      QVERIFY(!previewer.DisplayMovieInformation(cMichaelFassbenderRecord)); // Name still Michael Fassbender. unchange

      const QSqlRecord dMichaelFassbenderRecord = SqlRecordTestHelper::GetAMovieRecordFromDb("01234567890123456789012345678901", "", "D:/X-MEN/",  "Michael Fassbender.mp4", 1024);
      QVERIFY(previewer.DisplayMovieInformation(dMichaelFassbenderRecord)); // Name+pathHash changed
    }
  }

  void configuration_will_saved() {
    QVERIFY(Configuration().contains("Geometry/FLOATING_PREVIEW"));
    FileFolderPreviewer previewer{"TestFileFolderPreviewer secondtime"};

    QVERIFY(previewer.mImgVidOtherPane != nullptr);
    const QStringList othersLst;
    previewer.UpdateImgs("random images", othersLst); // should not crash down
    previewer.UpdateVids("", othersLst); // should not crash down
    previewer.UpdateOthers(othersLst); // should not crash down
  }
};

#include "FileFolderPreviewerTest.moc"
REGISTER_TEST(FileFolderPreviewerTest, false)
