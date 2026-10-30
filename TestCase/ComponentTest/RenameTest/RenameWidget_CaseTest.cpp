#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_Case.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

class RenameWidget_CaseTest : public PlainTestSuite {
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

 private slots:
  void case_change_ok() {
    const QString casePath{mTDir.path()};
    const QStringList selectFileNames{"super", "Henry Cavill.jpg"};

    RenameWidget_Case pCase;
    pCase.init();
    pCase.setModal(true);
    pCase.m_nameExtIndependent->setChecked(true);
    pCase.m_recursiveCB->setChecked(true);
    pCase.m_upperCase->setChecked(true);

    pCase.InitTextEditContent(casePath, selectFileNames);
    QCOMPARE(pCase.mSelectedNames, (QStringList{"super", "Henry Cavill.jpg"}));
    QCOMPARE(pCase.mRelToNameWithNoRoot, (QStringList{"/", "/super/", "/super/", "/"}));
    QCOMPARE(pCase.mNames, (QStringList{"super", "Chris Evans", "Chris Evans", "Henry Cavill"}));
    QCOMPARE(pCase.mExts, (QStringList{"", ".jpg", ".pson", ".jpg"}));

    const QString& sPathLeftNoRoot = pCase.m_relNameTE->toPlainText();
    const QString& sOldName = pCase.m_oBaseTE->toPlainText();
    const QString& sOldExt = pCase.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "/\n/super/\n/super/\n/");
    QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
    QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");

    {  // upper case
      const QString& sNewName = pCase.m_nBaseTE->toPlainText();
      const QString& sNewExt = pCase.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "SUPER\nCHRIS EVANS\nCHRIS EVANS\nHENRY CAVILL");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // lower case
      pCase.m_lowerCase->setChecked(true);
      emit pCase.m_nameCaseAg->triggered(pCase.m_lowerCase);

      const QString& sNewName = pCase.m_nBaseTE->toPlainText();
      const QString& sNewExt = pCase.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nchris evans\nchris evans\nhenry cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }
  }
};

#include "RenameWidget_CaseTest.moc"
REGISTER_TEST(RenameWidget_CaseTest, false)
