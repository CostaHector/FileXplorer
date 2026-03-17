#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_ConsecutiveFileNo.h"
#include "EndToExposePrivateMember.h"

#include "TDir.h"

class RenameWidget_ConsecutiveFileNoTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
 private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    const QList<FsNodeEntry> nodesEntries  //
        {
            {"Chris Pine 0.jpg", false, ""},
            {"Chris Pine 5.jpg", false, ""},
            {"Chris Pine 11.jpg", false, ""},
            {"Chris Hemsworth 13.jpg", false, ""},
        };
    QCOMPARE(mTDir.createEntries(nodesEntries), nodesEntries.size());
  }

  void test_ConsecutiveFileNo() {
    const QString consecutivePath{mTDir.path()};
    const QStringList selectFileNames{"Chris Pine 0.jpg", "Chris Pine 5.jpg", "Chris Pine 11.jpg", "Chris Hemsworth 13.jpg"};
    RenameWidget_ConsecutiveFileNo pConse;
    pConse.init();
    pConse.setModal(true);

    // m_nameExtIndependent set indepentent in InitExtraMemberWidget
    pConse.m_fileNoStartIndex->setText("1");
    QCOMPARE(pConse.m_nameExtIndependent->checkState(), Qt::CheckState::Unchecked);

    pConse.InitTextEditContent(consecutivePath, selectFileNames);
    QCOMPARE(pConse.mSelectedNames, (QStringList{"Chris Pine 0.jpg", "Chris Pine 5.jpg", "Chris Pine 11.jpg", "Chris Hemsworth 13.jpg"}));
    QCOMPARE(pConse.mRelToNameWithNoRoot, (QStringList{"", "", "", ""}));
    QCOMPARE(pConse.mNames, (QStringList{"Chris Pine 0.jpg", "Chris Pine 5.jpg", "Chris Pine 11.jpg", "Chris Hemsworth 13.jpg"}));
    QCOMPARE(pConse.mExts, (QStringList{"", "", "", ""}));

    const QString& sNewName = pConse.m_nBaseTE->toPlainText();
    const QString& sNewExt = pConse.m_nExtTE->toPlainText();
    QCOMPARE(sNewName, "Chris Pine 1.jpg\nChris Pine 2.jpg\nChris Pine 3.jpg\nChris Hemsworth 4.jpg");
    QCOMPARE(sNewExt, "\n\n\n");
  }
};

#include "RenameWidget_ConsecutiveFileNoTest.moc"
REGISTER_TEST(RenameWidget_ConsecutiveFileNoTest, false)
