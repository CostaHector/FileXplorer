#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_ArrangeSection.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class RenameWidget_ArrangeSectionTest : public PlainTestSuite {
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

  void ArrangeSection_ok() {
    const QString arrangePath{mTDir.path()};
    const QStringList selectFileNames{"Part A - Part B"};

    RenameWidget_ArrangeSection pArrange;
    pArrange.init();
    pArrange.setModal(true);
    QCOMPARE(pArrange.m_nameExtIndependent->checkState(), Qt::CheckState::Checked);

    pArrange.m_nameExtIndependent->setChecked(true);
    pArrange.m_recursiveCB->setChecked(true);

    pArrange._SWAP_SECTION_AT_2_INDEXES->setChecked(true);
    pArrange.m_swap2Index->setCurrentText("0,1");
    pArrange.m_recordWasted->setChecked(true);

    pArrange.InitTextEditContent(arrangePath, selectFileNames);
    QCOMPARE(pArrange.mSelectedNames, (QStringList{"Part A - Part B"}));
    QCOMPARE(pArrange.mRelToNameWithNoRoot, (QStringList{"", "Part A - Part B", "Part A - Part B"}));
    QCOMPARE(pArrange.mNames, (QStringList{"Part A - Part B", "Img A - Img B - Img C - Img D", "json A - json B - json C - json D"}));
    QCOMPARE(pArrange.mExts, (QStringList{"", ".jpg", ".json"}));

    const QString& sPathLeftNoRoot = pArrange.m_relNameTE->toPlainText();
    const QString& sOldName = pArrange.m_oBaseTE->toPlainText();
    const QString& sOldExt = pArrange.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nPart A - Part B\nPart A - Part B");
    QCOMPARE(sOldName, "Part A - Part B\nImg A - Img B - Img C - Img D\njson A - json B - json C - json D");
    QCOMPARE(sOldExt, "\n.jpg\n.json");

    {  // swap index at 0 and 1
      const QString& sNewName = pArrange.m_nBaseTE->toPlainText();
      const QString& sNewExt = pArrange.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Part B - Part A\nImg B - Img A - Img C - Img D\njson B - json A - json C - json D");
      QCOMPARE(sNewExt, "\n.jpg\n.json");
      QCOMPARE(pArrange.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::SAVED);
    }

    {  // swap index char invalid
      pArrange.m_swap2Index->setCurrentText("a0,1a");
      const QString& sNewName = pArrange.m_nBaseTE->toPlainText();
      const QString& sNewExt = pArrange.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "");
      QCOMPARE(sNewExt, "\n.jpg\n.json");
      QCOMPARE(pArrange.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::NOT_SAVED);
    }

    {  // swap index count invalid
      pArrange.m_swap2Index->setCurrentText("0,1,2");
      const QString& sNewName = pArrange.m_nBaseTE->toPlainText();
      const QString& sNewExt = pArrange.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "");
      QCOMPARE(sNewExt, "\n.jpg\n.json");
      QCOMPARE(pArrange.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::NOT_SAVED);
    }

    {  // rearrange index "2 - 1 - 0" and drop others
      pArrange._SECTIONS_USED_TO_JOIN->setChecked(true);
      pArrange.m_sectionsUsedToJoin->setCurrentText("2,1,0");
      pArrange.m_recordWasted->setChecked(true);
      const QString& sNewName = pArrange.m_nBaseTE->toPlainText();
      const QString& sNewExt = pArrange.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Part B - Part A\nImg C - Img B - Img A\njson C - json B - json A");
      QCOMPARE(sNewExt, "\n.jpg\n.json");
      QCOMPARE(pArrange.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::SAVED);
    }
  }
};

#include "RenameWidget_ArrangeSectionTest.moc"
REGISTER_TEST(RenameWidget_ArrangeSectionTest, false)
