#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_PrependParentFolderName.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class RenameWidget_PrependParentFolderNameTest : public PlainTestSuite {
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

  void prependParentFolderName_ok() {
    const QString prependPath{mTDir.path()};
    const QStringList selectFileNames{"super", "Henry Cavill.jpg"};

    RenameWidget_PrependParentFolderName pPrepend;
    pPrepend.init();
    pPrepend.setModal(true);
    pPrepend.m_nameExtIndependent->setChecked(true);
    // m_recursiveCB->checked(true) already set in initExclusiveSetting
    pPrepend.InitTextEditContent(prependPath, selectFileNames);
    QCOMPARE(pPrepend.mSelectedNames, (QStringList{"super", "Henry Cavill.jpg"}));
    QCOMPARE(pPrepend.mRelToNameWithNoRoot, (QStringList{"", "super", "super", ""}));
    QCOMPARE(pPrepend.mNames, (QStringList{"super", "Chris Evans", "Chris Evans", "Henry Cavill"}));
    QCOMPARE(pPrepend.mExts, (QStringList{"", ".jpg", ".pson", ".jpg"}));
    {
      const QString& sPathLeftNoRoot = pPrepend.m_relNameTE->toPlainText();
      const QString& sOldName = pPrepend.m_oBaseTE->toPlainText();
      const QString& sOldExt = pPrepend.m_oExtTE->toPlainText();
      const QString& sNewName = pPrepend.m_nBaseTE->toPlainText();
      const QString& sNewExt = pPrepend.m_nExtTE->toPlainText();

      QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
      QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(sNewName, "super\nsuper Chris Evans\nsuper Chris Evans\nHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }
  }
};

#include "RenameWidget_PrependParentFolderNameTest.moc"
REGISTER_TEST(RenameWidget_PrependParentFolderNameTest, false)
