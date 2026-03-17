#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_Insert.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class RenameWidget_InsertTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
 private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    const QList<FsNodeEntry> nodesEntries  //
        {
            {"super/Chris Evans.jpg", false, ""},
            {"super/Chris Evans.pson", false, ""},
            {"Henry Cavill.jpg", false, ""},
        };
    QCOMPARE(mTDir.createEntries(nodesEntries), nodesEntries.size());
  }

  void insert_ok() {
    const QString insertPath{mTDir.path()};
    const QStringList selectFileNames{"super",  // selection sequence decide rowSelections()
                                      "Henry Cavill.jpg"};

    RenameWidget_Insert pInsert;
    pInsert.init();
    pInsert.setModal(true);

    pInsert.m_nameExtIndependent->setChecked(true);  // file | .txt
    pInsert.m_recursiveCB->setChecked(true);         // including sub

    pInsert.insertStrCB->setCurrentText("");  // insert nothing
    pInsert.insertAtCB->setCurrentText("256");

    pInsert.InitTextEditContent(insertPath, selectFileNames);
    QCOMPARE(pInsert.mSelectedNames, (QStringList{"super", "Henry Cavill.jpg"}));
    QCOMPARE(pInsert.mRelToNameWithNoRoot, (QStringList{"", "super", "super", ""}));
    QCOMPARE(pInsert.mNames, (QStringList{"super", "Chris Evans", "Chris Evans", "Henry Cavill"}));
    QCOMPARE(pInsert.mExts, (QStringList{"", ".jpg", ".pson", ".jpg"}));
    const QString& sPathLeftNoRoot = pInsert.m_relNameTE->toPlainText();
    const QString& sOldName = pInsert.m_oBaseTE->toPlainText();
    const QString& sOldExt = pInsert.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
    QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
    QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");

    {
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExt = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // when insert str changed to " - Captain Steve" insert at 256
      pInsert.insertStrCB->setCurrentText(" - Captain Steve");
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExt = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super - Captain Steve\nChris Evans - Captain Steve\nChris Evans - Captain Steve\nHenry Cavill - Captain Steve");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // when insert at index changed to 0. " - Captain Steve" insert at 0
      pInsert.insertAtCB->setCurrentText("0");
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExt = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, " - Captain Stevesuper\n - Captain SteveChris Evans\n - Captain SteveChris Evans\n - Captain SteveHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    // common behavior 1: when recursiveCB changed to false. " - Captain Steve" insert at 0
    {
      pInsert.m_recursiveCB->setChecked(false);
      const QString& sOldName = pInsert.m_oBaseTE->toPlainText();
      const QString& sOldExt = pInsert.m_oExtTE->toPlainText();
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExt = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sOldName, "super\nHenry Cavill");
      QCOMPARE(sOldExt, "\n.jpg");
      QCOMPARE(sNewName, " - Captain Stevesuper\n - Captain SteveHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg");
    }

    // common behavior 2: when nameExtIndependent changed to false " - Captain Steve" insert at 256
    {
      pInsert.insertAtCB->setCurrentText("256");
      pInsert.m_nameExtIndependent->setChecked(false);
      const QString& sOldName = pInsert.m_oBaseTE->toPlainText();
      const QString& sOldExt = pInsert.m_oExtTE->toPlainText();
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExt = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sOldName, "super\nHenry Cavill.jpg");
      QCOMPARE(sOldExt, "\n");
      QCOMPARE(sNewName, "super - Captain Steve\nHenry Cavill.jpg - Captain Steve");
      QCOMPARE(sNewExt, "\n");
    }
  }
};

#include "RenameWidget_InsertTest.moc"
REGISTER_TEST(RenameWidget_InsertTest, false)
