#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "CastAkasManager.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

class CastAkasManagerTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mDir;
  const QString rootpath{mDir.path()};
  const QString gCastAkaFilePath{mDir.itemPath("cast_aka_list.txt")};
  CastAkasManager* pCastAka{nullptr};
 private slots:
  void initTestCase() {
    QVERIFY(mDir.IsValid());
    QVERIFY(!mDir.exists("cast_aka_list.txt"));  // file not exist at now

    CastAkasManager& castAkaInst = CastAkasManager::getInst();
    {
      castAkaInst.InitializeImpl(gCastAkaFilePath);  //
      QVERIFY(castAkaInst.CastAkaMap().isEmpty());
    }
    QList<FsNodeEntry> gNodeEntries{
        FsNodeEntry{"cast_aka_list.txt", false, "Captain America,Steve\nCristiano Ronaldo,CR7"},  //
    };
    QCOMPARE(mDir.createEntries(gNodeEntries), 1);
    QVERIFY(mDir.exists("cast_aka_list.txt"));  // file not exist at now

    castAkaInst.InitializeImpl(gCastAkaFilePath);
    QVERIFY(castAkaInst.CastAkaMap().size() >= 2);  // 2 elements
    pCastAka = &castAkaInst;
  }

  void contents_ok() {
    QVERIFY(pCastAka != nullptr);
    CAST_AKA_MGR_DATA_T& akaMap = pCastAka->CastAkaMap();
    QVERIFY(akaMap.contains("Captain America"));
    QVERIFY(akaMap.contains("Cristiano Ronaldo"));

    QCOMPARE(pCastAka->ForceReloadImpl(), 0); // unchange
  }
};

#include "CastAkasManagerTest.moc"
REGISTER_TEST(CastAkasManagerTest, false)
