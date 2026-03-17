#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_LongPath.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class RenameWidget_LongPathTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
 private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    const QList<FsNodeEntry> nodesEntries  //
        {
            {"Part A - Part B/Img A - Img B - Img C - Img D.jpg", false, ""},
            {"Part A - Part B/json A - json B - json C - json D.json", false, ""},
        };
    QCOMPARE(mTDir.createEntries(nodesEntries), nodesEntries.size());
  }

  void longPath_ok() {
    const QString longPath{mTDir.path()};
    const QStringList selectFileNames{"Part A - Part B"};

    RenameWidget_LongPath pLongPath;
    pLongPath.init();
    pLongPath.setModal(true);

    pLongPath.m_dropSectionLE->setText("1");  // drop the 1st section
    emit pLongPath.m_dropSectionLE->textEdited("1");
    pLongPath.m_maxPathLengthLE->setText("10");

    pLongPath.m_nameExtIndependent->setChecked(true);
    pLongPath.m_recursiveCB->setChecked(true);

    pLongPath.InitTextEditContent(longPath, selectFileNames);
    QCOMPARE(pLongPath.mSelectedNames, (QStringList{"Part A - Part B"}));
    QCOMPARE(pLongPath.mRelToNameWithNoRoot, (QStringList{"", "Part A - Part B", "Part A - Part B"}));
    QCOMPARE(pLongPath.mNames, (QStringList{"Part A - Part B", "Img A - Img B - Img C - Img D", "json A - json B - json C - json D"}));
    QCOMPARE(pLongPath.mExts, (QStringList{"", ".jpg", ".json"}));

    const QString& sPathLeftNoRoot = pLongPath.m_relNameTE->toPlainText();
    const QString& sOldName = pLongPath.m_oBaseTE->toPlainText();
    const QString& sOldExt = pLongPath.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nPart A - Part B\nPart A - Part B");
    QCOMPARE(sOldName, "Part A - Part B\nImg A - Img B - Img C - Img D\njson A - json B - json C - json D");
    QCOMPARE(sOldExt, "\n.jpg\n.json");

    {  // remove name first section
      const QString& sNewName = pLongPath.m_nBaseTE->toPlainText();
      const QString& sNewExt = pLongPath.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Part B\nImg B - Img C - Img D\njson B - json C - json D");
      QCOMPARE(sNewExt, "\n.jpg\n.json");
    }

    {  // long enough path. remove nothing
      pLongPath.m_maxPathLengthLE->setText("260");
      pLongPath.DropSectionChanged("260");

      const QString& sNewName = pLongPath.m_nBaseTE->toPlainText();
      const QString& sNewExt = pLongPath.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Part A - Part B\nImg A - Img B - Img C - Img D\njson A - json B - json C - json D");
      QCOMPARE(sNewExt, "\n.jpg\n.json");
    }
  }
};

#include "RenameWidget_LongPathTest.moc"
REGISTER_TEST(RenameWidget_LongPathTest, false)
