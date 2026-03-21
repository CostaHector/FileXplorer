#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_Numerize.h"
#include "EndToExposePrivateMember.h"

#include "ImgReorderDialog.h"
#include "TDir.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class RenameWidget_NumerizeTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
  const QString numerizerPath{mTDir.path()};
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

  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void numerize_ok() {
    const QStringList expectsFullNames{
        mTDir.itemPath("super"),
        mTDir.itemPath("super/Chris Evans.jpg"),
        mTDir.itemPath("super/Chris Evans.pson"),
        mTDir.itemPath("Henry Cavill.jpg"),
    };

    RenameWidget_Numerize pNumerize;
    pNumerize.init();
    pNumerize.setModal(true);
    QCOMPARE(pNumerize.GetSelectedFilesFullPath().isEmpty(), true);

    pNumerize.m_nameExtIndependent->setChecked(true);  // file | .txt
    pNumerize.m_recursiveCB->setChecked(true);         // including sub

    pNumerize.m_completeBaseName->setText("super");
    pNumerize.m_startNo->setCurrentText("1");
    pNumerize.m_isUniqueCounterPerExtension->setChecked(false);
    pNumerize.m_numberPattern->setCurrentText(" (%1)");

    const QStringList selectFileNames{"super", "Henry Cavill.jpg"};
    pNumerize.InitTextEditContent(numerizerPath, selectFileNames);
    QCOMPARE(pNumerize.GetSelectedFilesFullPath(), expectsFullNames);

    QCOMPARE(pNumerize.mSelectedNames, (QStringList{"super", "Henry Cavill.jpg"}));
    QCOMPARE(pNumerize.mRelToNameWithNoRoot, (QStringList{"", "super", "super", ""}));
    QCOMPARE(pNumerize.mNames, (QStringList{"super", "Chris Evans", "Chris Evans", "Henry Cavill"}));
    QCOMPARE(pNumerize.mExts, (QStringList{"", ".jpg", ".pson", ".jpg"}));

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

    {  // when base name changed
      pNumerize.m_completeBaseName->setText("Captain CE");
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExt = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE (1)\nCaptain CE (2)\nCaptain CE (3)\nCaptain CE (4)");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // when number pattern changed
      pNumerize.m_numberPattern->setCurrentText(" [%1]");
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExt = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE [1]\nCaptain CE [2]\nCaptain CE [3]\nCaptain CE [4]");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // when number start index changed
      pNumerize.m_startNo->setCurrentText("99");
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExt = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName, "Captain CE [099]\nCaptain CE [100]\nCaptain CE [101]\nCaptain CE [102]");
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }

    {  // when unique counter per ext changed
      pNumerize.m_isUniqueCounterPerExtension->setChecked(true);
      const QString& sNewName = pNumerize.m_nBaseTE->toPlainText();
      const QString& sNewExt = pNumerize.m_nExtTE->toPlainText();
      QCOMPARE(sNewName,
               "Captain CE\nCaptain CE [099]\nCaptain CE\nCaptain CE [100]");  // the folder and the pson has only itself, no need add number
      QCOMPARE(sNewExt, "\n.jpg\n.pson\n.jpg");
    }
  }

  void reorderNamesInListView_ok() {
    QCOMPARE(AdvanceRenamer::execCore(nullptr), -1);
    QCOMPARE(ImgReorderDialog::execCore(nullptr), -1);

    RenameWidget_Numerize pNumerize;
    pNumerize.init();
    pNumerize.setModal(true);

    pNumerize.m_nameExtIndependent->setChecked(true);  // file | .txt
    pNumerize.m_recursiveCB->setChecked(false);        // not include sub

    pNumerize.m_completeBaseName->setText("super");
    pNumerize.m_startNo->setCurrentText("1");
    pNumerize.m_isUniqueCounterPerExtension->setChecked(false);
    pNumerize.m_numberPattern->setCurrentText(" %1");

    const QStringList selectFileNames{"super", "Henry Cavill.jpg"};
    pNumerize.InitTextEditContent(numerizerPath, selectFileNames);
    QCOMPARE(pNumerize.m_nBaseTE->toPlainText(), (QString{"super 1\nsuper 2"}));
    QCOMPARE(pNumerize.m_nExtTE->toPlainText(), (QString{"\n.jpg"}));
    pNumerize.m_nBaseTE->clear();
    QCOMPARE(pNumerize.m_nBaseTE->toPlainText(), (QString{}));

    MOCKER(ImgReorderDialog::execCore)
        .expects(exactly(3))
        .will(returnValue((int)QDialog::Rejected))
        .then(returnValue((int)QDialog::Accepted))
        .then(returnValue((int)QDialog::Accepted));

    QCOMPARE(pNumerize.reorderNamesInListView(), false);
    QCOMPARE(pNumerize.m_nBaseTE->toPlainText(), (QString{})); // cancelled will not call setNewBaseNames
    QCOMPARE(pNumerize.m_nExtTE->toPlainText(), (QString{"\n.jpg"}));

    QCOMPARE(pNumerize.reorderNamesInListView(), true); // accepted will call setNewBaseNames
    QCOMPARE(pNumerize.m_nBaseTE->toPlainText(), (QString{"super 1\nsuper 2"}));
    QCOMPARE(pNumerize.m_nExtTE->toPlainText(), (QString{"\n.jpg"}));

    QVERIFY(pNumerize.m_dragToReorderNames != nullptr);
    pNumerize.m_dragToReorderNames->trigger();
  }
};

#include "RenameWidget_NumerizeTest.moc"
REGISTER_TEST(RenameWidget_NumerizeTest, false)
