#include <QCoreApplication>
#include <QtTest>
#include "MyTestSuite.h"
#include "TDir.h"
#include "BeginToExposePrivateMember.h"
#include "AdvanceRenamer.h"
#include "RenameWidget_LongPath.h"
#include "RenameWidget_ArrangeSection.h"
#include "RenameWidget_ConsecutiveFileNo.h"
#include "RenameWidget_ConvertBoldUnicodeCharset2Ascii.h"
#include "RenameWidget_Case.h"
#include "RenameWidget_Replace.h"
#include "RenameWidget_Numerize.h"
#include "RenameWidget_Insert.h"
#include "RenameWidget_ReverseNames.h"
#include "RenameWidget_PrependParentFolderName.h"
#include "EndToExposePrivateMember.h"
#include "RenameActions.h"

class AdvanceRenamerTest : public MyTestSuite {
  Q_OBJECT
public:
  AdvanceRenamerTest() : MyTestSuite{false} {}
  TDir mTDir;
  QString mPath{mTDir.path()};
  QDir mDir{mPath};
private slots:
  void initTestCase() {
    const QList<FsNodeEntry> nodesEntries //
        {
         {"numerizer/super/Chris Evans.jpg", false, ""},
         {"numerizer/super/Chris Evans.pson", false, ""},
         {"numerizer/Henry Cavill.jpg", false, ""},

         {"reverser/Henry Cavill 0.jpg", false, ""},
         {"reverser/Henry Cavill 1.jpg", false, ""},
         };
    QCOMPARE(mTDir.createEntries(nodesEntries), 3 + 2);
  }
  void test_Numerize() {
    const QString numerizerPath{mDir.absoluteFilePath("numerizer")};
    const QStringList selectFileNames {"super", // selection sequence decide rowSelections()
                                      "Henry Cavill.jpg"};

    RenameWidget_Numerize pNumerize;
    pNumerize.init();
    pNumerize.setModal(true);

    pNumerize.m_nameExtIndependent->setChecked(true); // file | .txt
    pNumerize.m_recursiveCB->setChecked(true); // including sub

    pNumerize.m_completeBaseName->setText("super");
    pNumerize.m_startNo->setText("1");
    pNumerize.m_isUniqueCounterPerExtension->setChecked(false);
    pNumerize.m_numberPattern->setCurrentText(" (%1)");

    pNumerize.InitTextEditContent(numerizerPath,
                                  selectFileNames);


    QCOMPARE(pNumerize.mSelectedNames,
             (QStringList{"super",
                          "Henry Cavill.jpg"}));
    QCOMPARE(pNumerize.mRelToNameWithNoRoot,
             (QStringList{"",
                          "super",
                          "super",
                          ""}));
    QCOMPARE(pNumerize.mNames,
             (QStringList{"super",
                          "Chris Evans",
                          "Chris Evans",
                          "Henry Cavill"}));
    QCOMPARE(pNumerize.mExts,
             (QStringList{"",
                          ".jpg",
                          ".pson",
                          ".jpg"}));
    {
      const QString& sPathLeftNoRoot = pNumerize.m_relNameTE->toPlainText();
      const QString& sOldName = pNumerize.m_oBaseTE->toPlainText();
      const QString& sOldExt = pNumerize.m_oExtTE->toPlainText();
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pNumerize.m_nExtTE->toPlainText();

      QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
      QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(sNewName, "super (1)\nsuper (2)\nsuper (3)\nsuper (4)");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // when base name changed
      pNumerize.m_completeBaseName->setText("Captain CE");
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE (1)\nCaptain CE (2)\nCaptain CE (3)\nCaptain CE (4)");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // when number pattern changed
      pNumerize.m_numberPattern->setCurrentText(" [%1]");
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE [1]\nCaptain CE [2]\nCaptain CE [3]\nCaptain CE [4]");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // when number start index changed
      pNumerize.m_startNo->setText("99");
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE [099]\nCaptain CE [100]\nCaptain CE [101]\nCaptain CE [102]");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // when unique counter per ext changed
      pNumerize.m_isUniqueCounterPerExtension->setChecked(true);
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE\nCaptain CE [099]\nCaptain CE\nCaptain CE [100]"); // the folder and the pson has only itself, no need add number
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }
  }

  void test_Replace_or_Delete() {
    const QString replacePath{mDir.absoluteFilePath("numerizer")};
    const QStringList selectFileNames {"super", // selection sequence decide rowSelections()
                                      "Henry Cavill.jpg"};

    RenameWidget_Replace pReplace;
    pReplace.init();
    pReplace.setModal(true);

    pReplace.m_nameExtIndependent->setChecked(true); // file | .txt
    pReplace.m_recursiveCB->setChecked(true); // including sub

    pReplace.m_oldStrCB->setCurrentText("Chris Evans");
    pReplace.m_newStrCB->setCurrentText("Chris Evans");
    pReplace.m_regexCB->setChecked(false);

    pReplace.InitTextEditContent(replacePath,
                                 selectFileNames);


    QCOMPARE(pReplace.mSelectedNames,
             (QStringList{"super",
                          "Henry Cavill.jpg"}));
    QCOMPARE(pReplace.mRelToNameWithNoRoot,
             (QStringList{"",
                          "super",
                          "super",
                          ""}));
    QCOMPARE(pReplace.mNames,
             (QStringList{"super",
                          "Chris Evans",
                          "Chris Evans",
                          "Henry Cavill"}));
    QCOMPARE(pReplace.mExts,
             (QStringList{"",
                          ".jpg",
                          ".pson",
                          ".jpg"}));
    {
      const QString& sPathLeftNoRoot = pReplace.m_relNameTE->toPlainText();
      const QString& sOldName = pReplace.m_oBaseTE->toPlainText();
      const QString& sOldExt = pReplace.m_oExtTE->toPlainText();
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pReplace.m_nExtTE->toPlainText();

      QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
      QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(sNewName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // when new name changed to empty, Chris Evans->""
      pReplace.m_newStrCB->setCurrentText("");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\n\n\nHenry Cavill");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // when new name changed, Chris Evans->Captain Steve
      pReplace.m_newStrCB->setCurrentText("Captain Steve");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nCaptain Steve\nCaptain Steve\nHenry Cavill");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // when old name changed, Henry Cavill->Captain Steve
      pReplace.m_oldStrCB->setCurrentText("Henry Cavill");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nChris Evans\nChris Evans\nCaptain Steve");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // regex enabled, (Henry Cavill)|(Chris Evans)->Captain Steve
      pReplace.m_regexCB->setChecked(true);
      pReplace.m_oldStrCB->setCurrentText("(Henry Cavill)|(Chris Evans)");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nCaptain Steve\nCaptain Steve\nCaptain Steve");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(pReplace.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::SAVED);
    }

    { // regex enabled but regex pattern invalid
      pReplace.m_oldStrCB->setCurrentText("(["); // invalid regex
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(pReplace.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::NOT_SAVED);
    }
  }

  void test_Insert() {
    const QString insertPath{mDir.absoluteFilePath("numerizer")};
    const QStringList selectFileNames {"super", // selection sequence decide rowSelections()
                                      "Henry Cavill.jpg"};

    RenameWidget_Insert pInsert;
    pInsert.init();
    pInsert.setModal(true);

    pInsert.m_nameExtIndependent->setChecked(true); // file | .txt
    pInsert.m_recursiveCB->setChecked(true); // including sub

    pInsert.insertStrCB->setCurrentText(""); // insert nothing
    pInsert.insertAtCB->setCurrentText("256");

    pInsert.InitTextEditContent(insertPath,
                                selectFileNames);

    QCOMPARE(pInsert.mSelectedNames,
             (QStringList{"super",
                          "Henry Cavill.jpg"}));
    QCOMPARE(pInsert.mRelToNameWithNoRoot,
             (QStringList{"",
                          "super",
                          "super",
                          ""}));
    QCOMPARE(pInsert.mNames,
             (QStringList{"super",
                          "Chris Evans",
                          "Chris Evans",
                          "Henry Cavill"}));
    QCOMPARE(pInsert.mExts,
             (QStringList{"",
                          ".jpg",
                          ".pson",
                          ".jpg"}));
    {
      const QString& sPathLeftNoRoot = pInsert.m_relNameTE->toPlainText();
      const QString& sOldName = pInsert.m_oBaseTE->toPlainText();
      const QString& sOldExt = pInsert.m_oExtTE->toPlainText();
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pInsert.m_nExtTE->toPlainText();

      QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
      QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(sNewName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // when insert str changed to " - Captain Steve" insert at 256
      pInsert.insertStrCB->setCurrentText(" - Captain Steve");
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super - Captain Steve\nChris Evans - Captain Steve\nChris Evans - Captain Steve\nHenry Cavill - Captain Steve");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // when insert at index changed to 0. " - Captain Steve" insert at 0
      pInsert.insertAtCB->setCurrentText("0");
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, " - Captain Stevesuper\n - Captain SteveChris Evans\n - Captain SteveChris Evans\n - Captain SteveHenry Cavill");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    // common behavior 1: when recursiveCB changed to false. " - Captain Steve" insert at 0
    {
      pInsert.m_recursiveCB->setChecked(false);
      const QString& sOldName = pInsert.m_oBaseTE->toPlainText();
      const QString& sOldExt = pInsert.m_oExtTE->toPlainText();
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sOldName, "super\nHenry Cavill");
      QCOMPARE(sOldExt, "\n.jpg");
      QCOMPARE(sNewName, " - Captain Stevesuper\n - Captain SteveHenry Cavill");
      QCOMPARE(sNewExtT, "\n.jpg");
    }

    // common behavior 2: when nameExtIndependent changed to false " - Captain Steve" insert at 256
    {
      pInsert.insertAtCB->setCurrentText("256");
      pInsert.m_nameExtIndependent->setChecked(false);
      const QString& sOldName = pInsert.m_oBaseTE->toPlainText();
      const QString& sOldExt = pInsert.m_oExtTE->toPlainText();
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sOldName, "super\nHenry Cavill.jpg");
      QCOMPARE(sOldExt, "\n");
      QCOMPARE(sNewName, "super - Captain Steve\nHenry Cavill.jpg - Captain Steve");
      QCOMPARE(sNewExtT, "\n");
    }
  }

  void test_ReverseNames() {
    const QString reversePath{mDir.absoluteFilePath("reverser")};
    const QStringList selectFileNames {"Henry Cavill 0.jpg",
                                      "Henry Cavill 1.jpg"};

    RenameWidget_ReverseNames pReverse;
    pReverse.init();
    pReverse.setModal(true);
    pReverse.m_nameExtIndependent->setChecked(true);
    pReverse.InitTextEditContent(reversePath,
                                 selectFileNames);

    QCOMPARE(pReverse.mSelectedNames,
             (QStringList{"Henry Cavill 0.jpg",
                          "Henry Cavill 1.jpg"}));
    QCOMPARE(pReverse.mRelToNameWithNoRoot,
             (QStringList{"",
                          ""}));
    QCOMPARE(pReverse.mNames,
             (QStringList{"Henry Cavill 0",
                          "Henry Cavill 1"}));
    QCOMPARE(pReverse.mExts,
             (QStringList{".jpg",
                          ".jpg"}));
    {
      const QString& sPathLeftNoRoot = pReverse.m_relNameTE->toPlainText();
      const QString& sOldName = pReverse.m_oBaseTE->toPlainText();
      const QString& sOldExt = pReverse.m_oExtTE->toPlainText();
      const QString& sNewName = pReverse.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pReverse.m_nExtTE->toPlainText();

      QCOMPARE(sPathLeftNoRoot, "\n");
      QCOMPARE(sOldName, "Henry Cavill 0\nHenry Cavill 1");
      QCOMPARE(sOldExt, ".jpg\n.jpg");
      QCOMPARE(sNewName, "Henry Cavill 1 \nHenry Cavill 0 ");
      QCOMPARE(sNewExtT, ".jpg\n.jpg");
    }
  }

  void test_PrependParentFolderName() {
    const QString prependPath{mDir.absoluteFilePath("numerizer")};
    const QStringList selectFileNames {"super",
                                      "Henry Cavill.jpg"};

    RenameWidget_PrependParentFolderName pPrepend;
    pPrepend.init();
    pPrepend.setModal(true);
    pPrepend.m_nameExtIndependent->setChecked(true);
    pPrepend.m_recursiveCB->setChecked(true);
    pPrepend.InitTextEditContent(prependPath,
                                 selectFileNames);

    QCOMPARE(pPrepend.mSelectedNames,
             (QStringList{"super",
                          "Henry Cavill.jpg"}));
    QCOMPARE(pPrepend.mRelToNameWithNoRoot,
             (QStringList{"",
                          "super",
                          "super",
                          ""}));
    QCOMPARE(pPrepend.mNames,
             (QStringList{"super",
                          "Chris Evans",
                          "Chris Evans",
                          "Henry Cavill"}));
    QCOMPARE(pPrepend.mExts,
             (QStringList{"",
                          ".jpg",
                          ".pson",
                          ".jpg"}));
    {
      const QString& sPathLeftNoRoot = pPrepend.m_relNameTE->toPlainText();
      const QString& sOldName = pPrepend.m_oBaseTE->toPlainText();
      const QString& sOldExt = pPrepend.m_oExtTE->toPlainText();
      const QString& sNewName = pPrepend.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pPrepend.m_nExtTE->toPlainText();

      QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
      QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(sNewName, "super\nsuper Chris Evans\nsuper Chris Evans\nHenry Cavill");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }
  }
  void test_Case() {
    const QString casePath{mDir.absoluteFilePath("numerizer")};
    const QStringList selectFileNames {"super",
                                      "Henry Cavill.jpg"};

    RenameWidget_Case pCase;
    pCase.init();
    pCase.setModal(true);
    pCase.m_nameExtIndependent->setChecked(true);
    pCase.m_recursiveCB->setChecked(true);
    g_renameAg()._UPPER_CASE->setChecked(true);

    pCase.InitTextEditContent(casePath,
                              selectFileNames);

    QCOMPARE(pCase.mSelectedNames,
             (QStringList{"super",
                          "Henry Cavill.jpg"}));
    QCOMPARE(pCase.mRelToNameWithNoRoot,
             (QStringList{"",
                          "super",
                          "super",
                          ""}));
    QCOMPARE(pCase.mNames,
             (QStringList{"super",
                          "Chris Evans",
                          "Chris Evans",
                          "Henry Cavill"}));
    QCOMPARE(pCase.mExts,
             (QStringList{"",
                          ".jpg",
                          ".pson",
                          ".jpg"}));

    const QString& sPathLeftNoRoot = pCase.m_relNameTE->toPlainText();
    const QString& sOldName = pCase.m_oBaseTE->toPlainText();
    const QString& sOldExt = pCase.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
    QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
    QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");

    { // upper case
      const QString& sNewName = pCase.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pCase.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "SUPER\nCHRIS EVANS\nCHRIS EVANS\nHENRY CAVILL");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }

    { // lower case
      g_renameAg()._LOWER_CASE->setChecked(true);
      emit g_renameAg().NAME_CASE->triggered(g_renameAg()._LOWER_CASE);

      const QString& sNewName = pCase.m_nBaseTE->toPlainText();
      const QString& sNewExtT = pCase.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nchris evans\nchris evans\nhenry cavill");
      QCOMPARE(sNewExtT, "\n.jpg\n.pson\n.jpg");
    }
  }
};

#include "AdvanceRenamerTest.moc"
AdvanceRenamerTest g_AdvanceRenamerTest;
