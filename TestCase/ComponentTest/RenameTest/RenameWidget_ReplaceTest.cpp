#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_Replace.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class RenameWidget_ReplaceTest : public PlainTestSuite {
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

  void replace_ok() {
    const QString replacePath{mTDir.path()};
    const QStringList selectFileNames{"super",  // selection sequence decide rowSelections()
                                      "Henry Cavill.jpg"};

    RenameWidget_Replace pReplace;
    pReplace.init();
    pReplace.setModal(true);

    pReplace.m_nameExtIndependent->setChecked(true);  // file | .txt
    pReplace.m_recursiveCB->setChecked(true);         // including sub

    pReplace.m_oldStrCB->setCurrentText("Chris Evans");
    pReplace.m_newStrCB->setCurrentText("Chris Evans");
    pReplace.m_regexCB->setChecked(false);

    pReplace.InitTextEditContent(replacePath, selectFileNames);
    QCOMPARE(pReplace.mSelectedNames, (QStringList{"super", "Henry Cavill.jpg"}));
    QCOMPARE(pReplace.mRelToNameWithNoRoot, (QStringList{"", "super", "super", ""}));
    QCOMPARE(pReplace.mNames, (QStringList{"super", "Chris Evans", "Chris Evans", "Henry Cavill"}));
    QCOMPARE(pReplace.mExts, (QStringList{"", ".jpg", ".pson", ".jpg"}));

    const QString& sPathLeftNoRoot = pReplace.m_relNameTE->toPlainText();
    const QString& sOldName = pReplace.m_oBaseTE->toPlainText();
    const QString& sOldExt = pReplace.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
    QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
    QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");

    {
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // when new name changed, Chris Evans->Captain Steve
      pReplace.m_newStrCB->setCurrentText("Captain Steve");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nCaptain Steve\nCaptain Steve\nHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // when old name changed, Henry Cavill->Captain Steve
      pReplace.m_oldStrCB->setCurrentText("Henry Cavill");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nChris Evans\nChris Evans\nCaptain Steve");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // regex enabled, (Henry Cavill)|(Chris Evans)->Captain Steve
      pReplace.m_regexCB->setChecked(true);
      pReplace.m_oldStrCB->setCurrentText("(Henry Cavill)|(Chris Evans)");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nCaptain Steve\nCaptain Steve\nCaptain Steve");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(pReplace.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::SAVED);
    }

    {                                             // regex enabled but regex pattern invalid
      pReplace.m_oldStrCB->setCurrentText("([");  // invalid regex
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(pReplace.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::NOT_SAVED);
    }
  }

  void delete_ok() {
    const QString deletePath{mTDir.path()};
    const QStringList selectFileNames{"super",  // selection sequence decide rowSelections()
                                      "Henry Cavill.jpg"};

    RenameWidget_Delete pDelete;
    pDelete.init();
    pDelete.setModal(true);

    pDelete.m_nameExtIndependent->setChecked(true);  // file | .txt
    pDelete.m_recursiveCB->setChecked(true);         // including sub

    pDelete.m_oldStrCB->setCurrentText("Chris Evans");
    // m_newStrCB->setCurrentText("") already set in initExclusiveSetting
    pDelete.m_regexCB->setChecked(false);

    pDelete.InitTextEditContent(deletePath, selectFileNames);
    QCOMPARE(pDelete.mSelectedNames, (QStringList{"super", "Henry Cavill.jpg"}));
    QCOMPARE(pDelete.mRelToNameWithNoRoot, (QStringList{"", "super", "super", ""}));
    QCOMPARE(pDelete.mNames, (QStringList{"super", "Chris Evans", "Chris Evans", "Henry Cavill"}));
    QCOMPARE(pDelete.mExts, (QStringList{"", ".jpg", ".pson", ".jpg"}));
    const QString& sPathLeftNoRoot = pDelete.m_relNameTE->toPlainText();
    const QString& sOldName = pDelete.m_oBaseTE->toPlainText();
    const QString& sOldExt = pDelete.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
    QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
    QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");

    {  // new name is empty by default, Chris Evans->""
      const QString& sNewName = pDelete.m_nBaseTE->toPlainText();
      const QString& sNewExt = pDelete.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\n\n\nHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // "[A-Za-z ]{10,20}" -> ""
      pDelete.m_regexCB->setChecked(true);
      pDelete.m_oldStrCB->setCurrentText("[A-Za-z ]{10,20}");
      const QString& sNewName = pDelete.m_nBaseTE->toPlainText();
      const QString& sNewExt = pDelete.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\n\n\n");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(pDelete.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::SAVED);
    }
  }
};

#include "RenameWidget_ReplaceTest.moc"
REGISTER_TEST(RenameWidget_ReplaceTest, false)
