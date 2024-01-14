#include <QCoreApplication>
#include <QtTest>

// add necessary includes here
#include "Tools/DuplicateImagesRemover.h"

class DuplicateImagesRemoverTest : public QObject {
  Q_OBJECT
 public:
  DuplicateImagesRemover m_duplicateImagsRemover;
 private slots:
  void test_NoDuplicateImages() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetDuplicateImagesFiles({"A.jpg", "B.png"});
    QVERIFY2(toBeDelete.isEmpty(), "should contains nothing");
  }
  void test_onlyOneImage() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetDuplicateImagesFiles({"A.jpg"});
    QVERIFY2(toBeDelete.isEmpty(), "should contains nothing");
  }
  void test_keep480pImage() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetDuplicateImagesFiles({"A 360p.jpg", "A 480p.jpg"});
    QVERIFY2(not toBeDelete.contains("A 480p.jpg"), "should not contains 480p");
  }

  void test_keep720pImage() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetDuplicateImagesFiles({"A 360p.jpg", "A 480p.jpg", "A 720p.jpg"});
    QVERIFY2(not toBeDelete.contains("A 720p.jpg"), "should not contains 720p");
  }
  void test_keep1080pImage() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetDuplicateImagesFiles({"A 360p.jpg", "A 1080p.jpg", "A 720p.jpg"});
    QVERIFY2(not toBeDelete.contains("A 1080p.jpg"), "should not contains 1080p");
  }
  void test_keep2160pImage() {
    const auto& toBeDelete = m_duplicateImagsRemover.GetDuplicateImagesFiles({"A 360p.jpg", "A 2160p.jpg", "A 720p.jpg"});
    QVERIFY2(not toBeDelete.contains("A 2160.jpg"), "should not contains 2160p");
  }
};

QTEST_MAIN(DuplicateImagesRemoverTest)

#include "DuplicateImagesRemoverTest.moc"
