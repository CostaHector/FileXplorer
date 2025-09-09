#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
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

class AdvanceRenamerTest : public PlainTestSuite {
  Q_OBJECT
public:
  AdvanceRenamerTest() : PlainTestSuite{} {}
  TDir mTDir;
  QString mPath{mTDir.path()};
  QDir mDir{mPath};
private slots:
  void initTestCase() {
    const QList<FsNodeEntry> nodesEntries //
        {{"numerizer/super/Chris Evans.jpg", false, ""},
         {"numerizer/super/Chris Evans.pson", false, ""},
         {"numerizer/Henry Cavill.jpg", false, ""},

         {"reverser/Henry Cavill 0.jpg", false, ""},
         {"reverser/Henry Cavill 1.jpg", false, ""},

         {"arrange/Part A - Part B/Img A - Img B - Img C - Img D.jpg", false, ""},
         {"arrange/Part A - Part B/json A - json B - json C - json D.json", false, ""}};
    QCOMPARE(mTDir.createEntries(nodesEntries), 3 + 2 + 2);
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

    pNumerize.InitTextEditContent(numerizerPath, selectFileNames);
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

    const QString& sPathLeftNoRoot = pNumerize.m_relNameTE->toPlainText();
    const QString& sOldName = pNumerize.m_oBaseTE->toPlainText();
    const QString& sOldExt = pNumerize.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
    QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
    QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");

    {
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExt = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super (1)\nsuper (2)\nsuper (3)\nsuper (4)");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    { // when base name changed
      pNumerize.m_completeBaseName->setText("Captain CE");
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExt = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE (1)\nCaptain CE (2)\nCaptain CE (3)\nCaptain CE (4)");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    { // when number pattern changed
      pNumerize.m_numberPattern->setCurrentText(" [%1]");
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExt = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE [1]\nCaptain CE [2]\nCaptain CE [3]\nCaptain CE [4]");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    { // when number start index changed
      pNumerize.m_startNo->setText("99");
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExt = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE [099]\nCaptain CE [100]\nCaptain CE [101]\nCaptain CE [102]");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    { // when unique counter per ext changed
      pNumerize.m_isUniqueCounterPerExtension->setChecked(true);
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExt = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE\nCaptain CE [099]\nCaptain CE\nCaptain CE [100]"); // the folder and the pson has only itself, no need add number
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }
  }

  void test_Replace() {
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

    pReplace.InitTextEditContent(replacePath, selectFileNames);
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

    { // when new name changed, Chris Evans->Captain Steve
      pReplace.m_newStrCB->setCurrentText("Captain Steve");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nCaptain Steve\nCaptain Steve\nHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    { // when old name changed, Henry Cavill->Captain Steve
      pReplace.m_oldStrCB->setCurrentText("Henry Cavill");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nChris Evans\nChris Evans\nCaptain Steve");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    { // regex enabled, (Henry Cavill)|(Chris Evans)->Captain Steve
      pReplace.m_regexCB->setChecked(true);
      pReplace.m_oldStrCB->setCurrentText("(Henry Cavill)|(Chris Evans)");
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nCaptain Steve\nCaptain Steve\nCaptain Steve");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(pReplace.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::SAVED);
    }

    { // regex enabled but regex pattern invalid
      pReplace.m_oldStrCB->setCurrentText("(["); // invalid regex
      const QString& sNewName = pReplace.m_nBaseTE->toPlainText();
      const QString& sNewExt = pReplace.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(pReplace.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::NOT_SAVED);
    }
  }

  void test_Delete() {
    const QString deletePath{mDir.absoluteFilePath("numerizer")};
    const QStringList selectFileNames {"super", // selection sequence decide rowSelections()
                                      "Henry Cavill.jpg"};

    RenameWidget_Delete pDelete;
    pDelete.init();
    pDelete.setModal(true);

    pDelete.m_nameExtIndependent->setChecked(true); // file | .txt
    pDelete.m_recursiveCB->setChecked(true); // including sub

    pDelete.m_oldStrCB->setCurrentText("Chris Evans");
    // m_newStrCB->setCurrentText("") already set in initExclusiveSetting
    pDelete.m_regexCB->setChecked(false);

    pDelete.InitTextEditContent(deletePath, selectFileNames);
    QCOMPARE(pDelete.mSelectedNames,
             (QStringList{"super",
                          "Henry Cavill.jpg"}));
    QCOMPARE(pDelete.mRelToNameWithNoRoot,
             (QStringList{"",
                          "super",
                          "super",
                          ""}));
    QCOMPARE(pDelete.mNames,
             (QStringList{"super",
                          "Chris Evans",
                          "Chris Evans",
                          "Henry Cavill"}));
    QCOMPARE(pDelete.mExts,
             (QStringList{"",
                          ".jpg",
                          ".pson",
                          ".jpg"}));
    const QString& sPathLeftNoRoot = pDelete.m_relNameTE->toPlainText();
    const QString& sOldName = pDelete.m_oBaseTE->toPlainText();
    const QString& sOldExt = pDelete.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
    QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
    QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");

    { // new name is empty by default, Chris Evans->""
      const QString& sNewName = pDelete.m_nBaseTE->toPlainText();
      const QString& sNewExt = pDelete.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\n\n\nHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    { // "[A-Za-z ]{10,20}" -> ""
      pDelete.m_regexCB->setChecked(true);
      pDelete.m_oldStrCB->setCurrentText("[A-Za-z ]{10,20}");
      const QString& sNewName = pDelete.m_nBaseTE->toPlainText();
      const QString& sNewExt = pDelete.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\n\n\n");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(pDelete.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::SAVED);
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

    pInsert.InitTextEditContent(insertPath, selectFileNames);
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

    { // when insert str changed to " - Captain Steve" insert at 256
      pInsert.insertStrCB->setCurrentText(" - Captain Steve");
      const QString& sNewName = pInsert.m_nBaseTE->toPlainText();
      const QString& sNewExt = pInsert.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super - Captain Steve\nChris Evans - Captain Steve\nChris Evans - Captain Steve\nHenry Cavill - Captain Steve");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    { // when insert at index changed to 0. " - Captain Steve" insert at 0
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

  void test_ReverseNames() {
    const QString reversePath{mDir.absoluteFilePath("reverser")};
    const QStringList selectFileNames {"Henry Cavill 0.jpg",
                                      "Henry Cavill 1.jpg"};

    RenameWidget_ReverseNames pReverse;
    pReverse.init();
    pReverse.setModal(true);
    pReverse.m_nameExtIndependent->setChecked(true);

    pReverse.InitTextEditContent(reversePath, selectFileNames);
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

  void test_PrependParentFolderName() {
    const QString prependPath{mDir.absoluteFilePath("numerizer")};
    const QStringList selectFileNames {"super",
                                      "Henry Cavill.jpg"};

    RenameWidget_PrependParentFolderName pPrepend;
    pPrepend.init();
    pPrepend.setModal(true);
    pPrepend.m_nameExtIndependent->setChecked(true);
    // m_recursiveCB->checked(true) already set in initExclusiveSetting
    pPrepend.InitTextEditContent(prependPath, selectFileNames);
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
      const QString& sNewExt = pPrepend.m_nExtTE->toPlainText();

      QCOMPARE(sPathLeftNoRoot, "\nsuper\nsuper\n");
      QCOMPARE(sOldName, "super\nChris Evans\nChris Evans\nHenry Cavill");
      QCOMPARE(sOldExt, "\n.jpg\n.pson\n.jpg");
      QCOMPARE(sNewName, "super\nsuper Chris Evans\nsuper Chris Evans\nHenry Cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
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

    pCase.InitTextEditContent(casePath, selectFileNames);
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
      const QString& sNewExt = pCase.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "SUPER\nCHRIS EVANS\nCHRIS EVANS\nHENRY CAVILL");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    { // lower case
      g_renameAg()._LOWER_CASE->setChecked(true);
      emit g_renameAg().NAME_CASE->triggered(g_renameAg()._LOWER_CASE);

      const QString& sNewName = pCase.m_nBaseTE->toPlainText();
      const QString& sNewExt = pCase.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "super\nchris evans\nchris evans\nhenry cavill");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }
  }

  void test_ArrangeSection() {
    const QString arrangePath{mDir.absoluteFilePath("arrange")};
    const QStringList selectFileNames {"Part A - Part B"};

    RenameWidget_ArrangeSection pArrange;
    pArrange.init();
    pArrange.setModal(true);

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
    const QString& sOldName        = pArrange.m_oBaseTE->toPlainText();
    const QString& sOldExt         = pArrange.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nPart A - Part B\nPart A - Part B");
    QCOMPARE(sOldName,        "Part A - Part B\nImg A - Img B - Img C - Img D\njson A - json B - json C - json D");
    QCOMPARE(sOldExt,         "\n.jpg\n.json");

    { // swap index at 0 and 1
      const QString& sNewName = pArrange.m_nBaseTE->toPlainText();
      const QString& sNewExt  = pArrange.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Part B - Part A\nImg B - Img A - Img C - Img D\njson B - json A - json C - json D");
      QCOMPARE(sNewExt,  "\n.jpg\n.json");
      QCOMPARE(pArrange.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::SAVED);
    }

    { // swap index char invalid
      pArrange.m_swap2Index->setCurrentText("a0,1a");
      const QString& sNewName = pArrange.m_nBaseTE->toPlainText();
      const QString& sNewExt  = pArrange.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "");
      QCOMPARE(sNewExt,  "\n.jpg\n.json");
      QCOMPARE(pArrange.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::NOT_SAVED);
    }

    { // swap index count invalid
      pArrange.m_swap2Index->setCurrentText("0,1,2");
      const QString& sNewName = pArrange.m_nBaseTE->toPlainText();
      const QString& sNewExt  = pArrange.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "");
      QCOMPARE(sNewExt,  "\n.jpg\n.json");
      QCOMPARE(pArrange.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::NOT_SAVED);
    }

    { // rearrange index "2 - 1 - 0" and drop others
      pArrange._SECTIONS_USED_TO_JOIN->setChecked(true);
      pArrange.m_sectionsUsedToJoin->setCurrentText("2,1,0");
      pArrange.m_recordWasted->setChecked(true);
      const QString& sNewName = pArrange.m_nBaseTE->toPlainText();
      const QString& sNewExt  = pArrange.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Part B - Part A\nImg C - Img B - Img A\njson C - json B - json A");
      QCOMPARE(sNewExt,  "\n.jpg\n.json");
      QCOMPARE(pArrange.regexValidLabel->state(), StateLabel::LABEL_STATUS_E::SAVED);
    }
  }

  void test_LongPath() {
    const QString longPath{mDir.absoluteFilePath("arrange")};
    const QStringList selectFileNames {"Part A - Part B"};

    RenameWidget_LongPath pLongPath;
    pLongPath.init();
    pLongPath.setModal(true);

    pLongPath.m_dropSectionLE->setText("1"); // drop the 1st section
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
    const QString& sOldName        = pLongPath.m_oBaseTE->toPlainText();
    const QString& sOldExt         = pLongPath.m_oExtTE->toPlainText();
    QCOMPARE(sPathLeftNoRoot, "\nPart A - Part B\nPart A - Part B");
    QCOMPARE(sOldName,        "Part A - Part B\nImg A - Img B - Img C - Img D\njson A - json B - json C - json D");
    QCOMPARE(sOldExt,         "\n.jpg\n.json");

    { // remove name first section
      const QString& sNewName = pLongPath.m_nBaseTE->toPlainText();
      const QString& sNewExt  = pLongPath.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Part B\nImg B - Img C - Img D\njson B - json C - json D");
      QCOMPARE(sNewExt,  "\n.jpg\n.json");
    }

    { // long enough path. remove nothing
      pLongPath.m_maxPathLengthLE->setText("260");
      pLongPath.DropSectionChanged("260");

      const QString& sNewName = pLongPath.m_nBaseTE->toPlainText();
      const QString& sNewExt  = pLongPath.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Part A - Part B\nImg A - Img B - Img C - Img D\njson A - json B - json C - json D");
      QCOMPARE(sNewExt,  "\n.jpg\n.json");
    }
  }
};

#include "AdvanceRenamerTest.moc"
REGISTER_TEST(AdvanceRenamerTest, false)
