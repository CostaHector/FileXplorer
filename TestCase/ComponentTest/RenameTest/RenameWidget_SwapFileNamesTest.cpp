#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_SwapFileNames.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class RenameWidget_SwapFileNamesTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
 private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    const QList<FsNodeEntry> nodesEntries  //
        {
            {"Henry Cavill 0.jpg", false, ""},
            {"Henry Cavill 1.jpg", false, ""},
        };
    QCOMPARE(mTDir.createEntries(nodesEntries), nodesEntries.size());
  }

  void swap2FilesName_ok() {
    const QString reversePath{mTDir.path()};
    const QStringList selectFileNames{"Henry Cavill 0.jpg", "Henry Cavill 1.jpg"};

    RenameWidget_SwapFileNames pReverse;
    pReverse.init();
    pReverse.setModal(true);
    pReverse.m_nameExtIndependent->setChecked(true);

    pReverse.InitTextEditContent(reversePath, selectFileNames);
    QCOMPARE(pReverse.mSelectedNames, (QStringList{"Henry Cavill 0.jpg", "Henry Cavill 1.jpg"}));
    QCOMPARE(pReverse.mRelToNameWithNoRoot, (QStringList{"", ""}));
    QCOMPARE(pReverse.mNames, (QStringList{"Henry Cavill 0", "Henry Cavill 1"}));
    QCOMPARE(pReverse.mExts, (QStringList{".jpg", ".jpg"}));
    const QString& sPathLeftNoRoot = pReverse.m_relNameTE->toPlainText();
    const QString& sOldName = pReverse.m_oBaseTE->toPlainText();
    const QString& sOldExt = pReverse.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\n");
    QCOMPARE(sOldName, "Henry Cavill 0\nHenry Cavill 1");
    QCOMPARE(sOldExt, ".jpg\n.jpg");

    {
      const QString& sNewName = pReverse.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReverse.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Henry Cavill 1 \nHenry Cavill 0 ");
      QCOMPARE(sNewExt, ".jpg\n.jpg");
    }
  }
};

#include "RenameWidget_SwapFileNamesTest.moc"
REGISTER_TEST(RenameWidget_SwapFileNamesTest, false)
