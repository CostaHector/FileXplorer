#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "RelatedHelper.h"
#include "UserInteractiveMock.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

    using namespace UserInteractiveMock;
using namespace RelatedHelper;

class RelatedHelperTest : public PlainTestSuite {
  Q_OBJECT
public:
private:
  QStringList chrisEvansWithJsonLst{"/Chris Evans.jpg",
                                    "/Chris Evans 1.jpg",
                                    "/Chris Evans 2.png",
                                    "/Chris Evans - 3.png",
                                    "/Chris Evans_tn.jpg",
                                    "/Chris Evans.mp4",
                                    "/Chris Evans.mkv",
                                    "/Chris Evans.json",
                                    };
  QStringList hughJackmanNoJsonList{
      "/Hugh Jackman.jpg",
      "/Hugh Jackman 1.png",
      "/Hugh Jackman 2.png",
      "/Hugh Jackman - 3.png",
      "/Hugh Jackman_tn.jpg",
      "/Hugh Jackman.mp4",
      "/Hugh Jackman.mkv",
  };
  // Chris Evans.json Exist
  // Hugh Jackman.json not Exist
  QStringList allFiles{chrisEvansWithJsonLst + hughJackmanNoJsonList};
private slots:
  void initTestCase() {
    QSet<QString> existsFiles{allFiles.cbegin(), allFiles.cend()};
    QCOMPARE(existsFiles.size(), chrisEvansWithJsonLst.size() + hughJackmanNoJsonList.size());

    GlobalMockObject::reset();
    MOCKER((FILE_EXIST_TYPE) QFile::exists).stubs().will(invoke(invoke_exists));
    existsSet() = existsFiles;
  }

  void cleanupTestCase() {
    existsSet().clear();
    GlobalMockObject::verify();
  }

  void getJsonPathFromFile_ok() {
    QString expectJsonPath = "/Chris Evans.json";
    for (const QString& chrisEvansRelatedFile: chrisEvansWithJsonLst) {
      QString jsonPath;
      QVERIFY(getJsonPathFromFile(chrisEvansRelatedFile, jsonPath));
      QCOMPARE(jsonPath, expectJsonPath);
    }

    for (const QString& hughJackmanRelatedFile: hughJackmanNoJsonList) {
      QString jsonPath;
      QVERIFY(!getJsonPathFromFile(hughJackmanRelatedFile, jsonPath));
      QVERIFY(jsonPath.isEmpty());
    }
  }

  void GetRelatedJsonAbsPaths_ok() {
    QSet<QString> expectJsons{"/Chris Evans.json"};
    QCOMPARE(GetRelatedJsonAbsPaths(chrisEvansWithJsonLst), expectJsons);
    QCOMPARE(GetRelatedJsonAbsPaths(hughJackmanNoJsonList), (QSet<QString>{}));

    QCOMPARE(GetRelatedJsonAbsPaths(allFiles), expectJsons);
  }

  void getBaseNameForImage_ok() {
    QCOMPARE(getBaseNameForImage("Chris Evans 1"), "Chris Evans");
    QCOMPARE(getBaseNameForImage("Chris Evans - 1"), "Chris Evans");
    QCOMPARE(getBaseNameForImage("Chris Evans - 99"), "Chris Evans");
  }
};

#include "RelatedHelperTest.moc"
REGISTER_TEST(RelatedHelperTest, false)