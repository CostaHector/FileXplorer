#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pubTestTool/MyTestSuite.h"
// add necessary includes here
#include "Tools/LowResImgsRemover.h"

class LowResImgsRemoverTest : public MyTestSuite {
  Q_OBJECT
 public:
  LowResImgsRemoverTest() : MyTestSuite{false} {}
  LowResImgsRemover m_duplicateImagsRemover;
 private slots:
  void test_NoDuplicateImages() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetLowResImgsToDel({"A.jpg", "B.png"});
    QVERIFY2(toBeDelete.isEmpty(), "should contains nothing");
  }
  void test_onlyOneImage() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetLowResImgsToDel({"A.jpg"});
    QVERIFY2(toBeDelete.isEmpty(), "should contains nothing");
  }
  void test_keep480pImage_no_exist() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetLowResImgsToDel({"A 360p.jpg", "A 480p.jpg"});
    QVERIFY2(!toBeDelete.contains("A 480p.jpg"), "should not contains 480p");
  }

  void test_keep720pImage_no_exist() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetLowResImgsToDel({"A 360p.jpg", "A 480p.jpg", "A 720p.jpg"});
    QVERIFY2(!toBeDelete.contains("A 720p.jpg"), "should not contains 720p");
  }
  void test_keep1080pImage_no_exist() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetLowResImgsToDel({"A 360p.jpg", "A 1080p.jpg", "A 720p.jpg"});
    QVERIFY2(!toBeDelete.contains("A 1080p.jpg"), "should not contains 1080p");
  }
  void test_keep2160pImage_no_exist() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetLowResImgsToDel({"A 360p.jpg", "A 2160p.jpg", "A 720p.jpg"});
    QVERIFY2(!toBeDelete.contains("A 2160.jpg"), "should not contains 2160p");
  }
};

LowResImgsRemoverTest g_LowResImgsRemoverTest;
#include "LowResImgsRemoverTest.moc"
