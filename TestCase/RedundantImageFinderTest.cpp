#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "Tools/RedunImgLibs.h"

const QString rootpath = QFileInfo(__FILE__).absolutePath() + "/test/TestEnv_RedundantImageFinderTest";
class RedundantImageFinderTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_ReplaceRename_invalid_regex_failed() {
    // precondition
    const QFile fiA{rootpath + "/a.jpg"};
    const QFile fiB{rootpath + "/b.png"};
    const QFile fiCEmpty{rootpath + "/c.webp"};
    QVERIFY(fiA.exists());
    QVERIFY(fiB.exists());
    QVERIFY(fiCEmpty.exists());
    QCOMPARE(fiA.size(), 3);
    QCOMPARE(fiB.size(), 3);
    QCOMPARE(fiCEmpty.size(), 0);

    // procedure
    RedunImgLibs redunImgLib;
    QCOMPARE(redunImgLib.LearnSizeAndHashFromRedunImgPath(rootpath), 3);  // 3 hash
    QCOMPARE(redunImgLib.m_commonFileHash.size(), 3);
    QCOMPARE(redunImgLib.m_commonFileSizeSet.size(), 2);

    const bool before = PreferenceSettings().value(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v).toBool();
    PreferenceSettings().setValue(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, true);
    const auto& itemsEmptyAlsoRedun = redunImgLib.FindRedunImgs(rootpath);
    QCOMPARE(itemsEmptyAlsoRedun.size(), 3);

    PreferenceSettings().setValue(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, false);
    const auto& itemsEmptyNotRedun = redunImgLib.FindRedunImgs(rootpath);
    QCOMPARE(itemsEmptyNotRedun.size(), 2);
    PreferenceSettings().setValue(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, before);
  }
};

RedundantImageFinderTest g_RedundantImageFinderTest;
#include "RedundantImageFinderTest.moc"
