#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "JsonUpdater.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "FileTool.h"
#include "PathTool.h"
#include "JsonFieldBoundary.h"
#include "UserInteractiveMock.h"

#include <QFile>
#include <QIODevice>
#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

// fileAbsPath -> filesize: qint64
// "file 1024.mp4" -> 1024
// "file 512.dvd" -> 512
qint64 invokeGetFileSize(const QString& fileAbsPath) {
  QString baseName = PathTool::GetBaseName(fileAbsPath);
  const int sizeStartIndex = baseName.lastIndexOf(' ');

  qint64 filesize = -1;
  if (sizeStartIndex == -1) {
    return filesize;
  }
  bool bOk = false;
  filesize = baseName.mid(sizeStartIndex + 1).toLongLong(&bOk);
  if (!bOk) {
    return filesize;
  }
  return filesize;
}

class JsonUpdaterTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
private slots:
  void initTestCase() {
    GlobalMockObject::reset();
    MOCKER(FileTool::GetFileSize).stubs().will(invoke(invokeGetFileSize));
    QCOMPARE(invokeGetFileSize("file 1024.mp4"), 1024);
    QCOMPARE(invokeGetFileSize("file 512.dvd"), 512);
    QCOMPARE(FileTool::GetFileSize("file 1024.mp4"), 1024);
    QCOMPARE(FileTool::GetFileSize("file 512.dvd"), 512);
  }

  void cleanupTestCase() {
    GlobalMockObject::verify();
  }

  void MergeTextContentsIntoDetailAndRecycleTxt_ok() {
    mTDir.ClearAll();
    QList<FsNodeEntry> nodes {
        {"A random video 1024.txt", false, R"(contents 1024.txt)"},
        {"too large exceed 10240000.txt", false, R"(contents 10240000.txt)"}
    };
    QCOMPARE(mTDir.createEntries(nodes), 2);

    QString detail;
    // detail is empty at first. no need append \n at first
    QVERIFY(mTDir.exists("A random video 1024.txt"));
    QVERIFY(JsonUpdater::MergeTextContentsIntoDetailAndRecycleTxt(mTDir.itemPath("A random video 1024.txt"), detail));
    QCOMPARE(detail, R"(contents 1024.txt)");
    QVERIFY(!mTDir.exists("A random video 1024.txt"));

    // detail already contains text, not append again
    mTDir.createEntries(nodes); // create again
    QVERIFY(!JsonUpdater::MergeTextContentsIntoDetailAndRecycleTxt(mTDir.itemPath("A random video 1024.txt"), detail));
    QCOMPARE(detail, R"(contents 1024.txt)");

    // new contents. append back '\n' and contents
    QVERIFY(FileTool::StringTextWriter(mTDir.itemPath("A random video 1024.txt"), "new contents", QIODevice::OpenModeFlag::WriteOnly));
    QVERIFY(JsonUpdater::MergeTextContentsIntoDetailAndRecycleTxt(mTDir.itemPath("A random video 1024.txt"), detail));
    QCOMPARE(detail, "contents 1024.txt\nnew contents");

    // too large will not read
    detail.clear();
    QVERIFY(!JsonUpdater::MergeTextContentsIntoDetailAndRecycleTxt(mTDir.itemPath("too large exceed 10240000.txt"), detail));
    QVERIFY(detail.isEmpty());
  }

  void UpdateJsonKeyValuePair_ok() {
    auto touchOrDeleteMP4File = [] (TDir& mTDir, bool bCreate) -> bool {
      if (bCreate) {
        return mTDir.touch("path/to/Lewandowski 1024.mp4", "Contents in mp4");
      } else {
        return QFile::remove(mTDir.itemPath("path/to/Lewandowski 1024.mp4"));
      }
    };

    mTDir.ClearAll();
    QVERIFY(touchOrDeleteMP4File(mTDir, true));
    ScenesMixed sMixed;
    sMixed.m_img2Name["Lewandowski 1024"] = QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"};
    sMixed.m_vid2Name["Lewandowski 1024"] = QStringList{"Lewandowski 1024.mp4"};
    sMixed.m_json2Name = QSet<QString>{"Lewandowski 1024"}; // Lewandowski 1024.json

    const QString parentPath{mTDir.itemPath("path/to")};
    const QString jsonFileBaseName{"Lewandowski 1024"};
    QVERIFY(mTDir.exists("path/to/Lewandowski 1024.mp4"));

    // Check m_Name, m_ImgName, m_VidName, m_Size, m_Rate

    { // no update
      QVariantHash emptyJson;
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, emptyJson, parentPath, jsonFileBaseName), (JsonOp::Counter(0, 0, 0, 0)));

      JsonPr nonLocalJsonPr;
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, nonLocalJsonPr), (JsonOp::Counter(0, 0, 0, 0)));
    }

    { // no update
      QVariantHash noNameJson{{"Size", 1024}, {"Rate", 10}, {"ImgName", QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"}}, {"VidName", "Lewandowski 1024.mp4"}};
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noNameJson, parentPath, jsonFileBaseName), (JsonOp::Counter(0, 0, 0, 0)));
    }

    {
      // Name value updated
      QVariantHash nameNoMatchJson{{"Name", "Lewan 1024"}, {"Size", 1024}, {"Rate", 10}, {"ImgName", QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"}}, {"VidName", "Lewandowski 1024.mp4"}};
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, nameNoMatchJson, parentPath, jsonFileBaseName), (JsonOp::Counter(1, 1, 0, 0)));
      QCOMPARE(nameNoMatchJson["Name"].toString(), "Lewandowski 1024");
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, nameNoMatchJson, parentPath, jsonFileBaseName), (JsonOp::Counter(0, 1, 0, 0))); // update again. no field updated

      JsonPr nameNoMatchJsonPr;
      {
        nameNoMatchJsonPr.m_Prepath = parentPath;
        nameNoMatchJsonPr.jsonFileName = jsonFileBaseName + ".json";
        nameNoMatchJsonPr.m_Name = "Lewan 1024";
        nameNoMatchJsonPr.m_Size = 1024;
        nameNoMatchJsonPr.m_Rate = 10;
        nameNoMatchJsonPr.m_ImgName = QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"};
        nameNoMatchJsonPr.m_VidName = "Lewandowski 1024.mp4";
      }
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, nameNoMatchJsonPr), (JsonOp::Counter(1, 1, 0, 0)));
      QCOMPARE(nameNoMatchJsonPr.m_Name, "Lewandowski 1024");
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, nameNoMatchJsonPr), (JsonOp::Counter(0, 1, 0, 0))); // update again. no field updated
    }

    {
      // ImgName inserted
      QVariantHash noImgNameMatchJson{{"Name", "Lewandowski 1024"}, {"Size", 1024}, {"Rate", 10}, {"VidName", "Lewandowski 1024.mp4"}};
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noImgNameMatchJson, parentPath, jsonFileBaseName), (JsonOp::Counter(1, 1, 1, 0)));
      QStringList beforeImages = noImgNameMatchJson["ImgName"].toStringList();
      QCOMPARE(beforeImages, (QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"}));
      // ImgName updated, changed to wrong one, update will correct it
      noImgNameMatchJson["ImgName"] = QStringList{"wrong 1.jpg", "wrong 2.jpg", "wrong 3.jpg"};
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noImgNameMatchJson, parentPath, jsonFileBaseName), (JsonOp::Counter(1, 1, 1, 0)));
      QStringList afterImages = noImgNameMatchJson["ImgName"].toStringList();
      QCOMPARE(afterImages, beforeImages);
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noImgNameMatchJson, parentPath, jsonFileBaseName), (JsonOp::Counter(0, 1, 0, 0))); // update again. no field updated


      JsonPr noImgNameJsonPr;
      {
        noImgNameJsonPr.m_Prepath = parentPath;
        noImgNameJsonPr.jsonFileName = jsonFileBaseName + ".json";
        noImgNameJsonPr.m_Name = "Lewandowski 1024";
        noImgNameJsonPr.m_Size = 1024;
        noImgNameJsonPr.m_Rate = 10;
        noImgNameJsonPr.m_ImgName = QStringList{};
        noImgNameJsonPr.m_VidName = "Lewandowski 1024.mp4";
      }
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noImgNameJsonPr), (JsonOp::Counter(1, 1, 1, 0)));
      QCOMPARE(noImgNameJsonPr.m_ImgName, (QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"}));
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noImgNameJsonPr), (JsonOp::Counter(0, 1, 0, 0)));
    }

    {
      // VidName inserted
      QVariantHash noVidNameMatchJson{{"Name", "Lewandowski 1024"}, {"Size", 1024}, {"Rate", 10}, {"ImgName", QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"}}};
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noVidNameMatchJson, parentPath, jsonFileBaseName), (JsonOp::Counter(1, 1, 0, 1)));
      // VidName updated
      QString beforeVidName = noVidNameMatchJson["VidName"].toString();
      QCOMPARE(beforeVidName, (QString{"Lewandowski 1024.mp4"}));
      // VidName updated, changed to wrong one, update will correct it
      noVidNameMatchJson["VidName"] = QString{"wrong.mp4"};
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noVidNameMatchJson, parentPath, jsonFileBaseName), (JsonOp::Counter(1, 1, 0, 1)));
      QString afterVidName = noVidNameMatchJson["VidName"].toString();
      QCOMPARE(afterVidName, beforeVidName);
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noVidNameMatchJson, parentPath, jsonFileBaseName), (JsonOp::Counter(0, 1, 0, 0))); // update again. no field updated

      JsonPr noVidNameJsonPr;
      {
        noVidNameJsonPr.m_Prepath = parentPath;
        noVidNameJsonPr.jsonFileName = jsonFileBaseName + ".json";
        noVidNameJsonPr.m_Name = "Lewandowski 1024";
        noVidNameJsonPr.m_Size = 1024;
        noVidNameJsonPr.m_Rate = 10;
        noVidNameJsonPr.m_ImgName = QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"};
        noVidNameJsonPr.m_VidName = "";
      }
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noVidNameJsonPr), (JsonOp::Counter(1, 1, 0, 1)));
      QCOMPARE(noVidNameJsonPr.m_VidName, (QString{"Lewandowski 1024.mp4"}));
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noVidNameJsonPr), (JsonOp::Counter(0, 1, 0, 0)));

      // Videos no longer exist. will not update VidName
      QVERIFY(touchOrDeleteMP4File(mTDir, false));
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noVidNameMatchJson, parentPath, jsonFileBaseName), (JsonOp::Counter(0, 1, 0, 0))); // update again. no field updated
      QCOMPARE(noVidNameMatchJson["VidName"].toString(), (QString{"Lewandowski 1024.mp4"}));

      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noVidNameJsonPr), (JsonOp::Counter(0, 1, 0, 0)));
      QCOMPARE(noVidNameJsonPr.m_VidName, (QString{"Lewandowski 1024.mp4"}));
    }

    QVERIFY(touchOrDeleteMP4File(mTDir, true));

    {
      // Size inserted
      QVariantHash noSizeJson{{"Name", "Lewandowski 1024"}, {"Rate", 10}, {"ImgName", QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"}}, {"VidName", "Lewandowski 1024.mp4"}};
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noSizeJson, parentPath, jsonFileBaseName), (JsonOp::Counter(1, 1, 0, 0)));
      QCOMPARE(noSizeJson["Size"].toLongLong(), 1024);
      // Size updated. changed to another wrong value
      noSizeJson["Size"] = 1024 * 1000;
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noSizeJson, parentPath, jsonFileBaseName), (JsonOp::Counter(1, 1, 0, 0)));
      QCOMPARE(noSizeJson["Size"].toLongLong(), 1024);
      // update again. no field updated
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noSizeJson, parentPath, jsonFileBaseName), (JsonOp::Counter(0, 1, 0, 0)));

      JsonPr noSizeJsonPr;
      {
        noSizeJsonPr.m_Prepath = parentPath;
        noSizeJsonPr.jsonFileName = jsonFileBaseName + ".json";
        noSizeJsonPr.m_Name = "Lewandowski 1024";
        noSizeJsonPr.m_Size = 0;
        noSizeJsonPr.m_Rate = 10;
        noSizeJsonPr.m_ImgName = QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"};
        noSizeJsonPr.m_VidName = "Lewandowski 1024.mp4";
      }
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noSizeJsonPr), (JsonOp::Counter(1, 1, 0, 0)));
      QCOMPARE(noSizeJsonPr.m_Size, 1024);
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noSizeJsonPr), (JsonOp::Counter(0, 1, 0, 0)));


      // Videos no longer exist. will not update Size
      QVERIFY(touchOrDeleteMP4File(mTDir, false));
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noSizeJson, parentPath, jsonFileBaseName), (JsonOp::Counter(0, 1, 0, 0)));
      QCOMPARE(noSizeJson["Size"].toLongLong(), 1024);

      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noSizeJsonPr), (JsonOp::Counter(0, 1, 0, 0)));
      QCOMPARE(noSizeJsonPr.m_Size, 1024);
    }

    QVERIFY(touchOrDeleteMP4File(mTDir, true));

    {
      // Rate inserted
      QVariantHash noRateJson{{"Name", "Lewandowski 1024"}, {"Size", 1024}, {"ImgName", QStringList{"Lewandowski 1024 1.jpg", "Lewandowski 1024 2.jpg"}}, {"VidName", "Lewandowski 1024.mp4"}};
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noRateJson, parentPath, jsonFileBaseName), (JsonOp::Counter(1, 1, 0, 0)));
      bool bRateValid{false};
      QCOMPARE(noRateJson["Rate"].toInt(&bRateValid), JsonFieldBoundary::RATE_MIN_UNINITIALIZED_V);
      QVERIFY(bRateValid);

      // update again. no field updated
      QCOMPARE(JsonUpdater::UpdateJsonKeyValuePair(sMixed, noRateJson, parentPath, jsonFileBaseName), (JsonOp::Counter(0, 1, 0, 0)));
    }
  }
};

#include "JsonUpdaterTest.moc"
REGISTER_TEST(JsonUpdaterTest, false)