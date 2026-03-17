#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "RenameWidget_ConvertBoldUnicodeCharset2Ascii.h"
#include "EndToExposePrivateMember.h"
#include <QPushButton>

#include "TDir.h"

class RenameWidget_ConvertBoldUnicodeCharset2AsciiTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
 private slots:
  void initTestCase() {
    QVERIFY(mTDir.IsValid());
    const QList<FsNodeEntry> nodesEntries  //
        {
            {"𝗔𝗕𝗖.txt", false, ""},
            {"𝘅𝘆𝘇.txt", false, ""},
            {"def.txt", false, ""},
        };
    QCOMPARE(mTDir.createEntries(nodesEntries), nodesEntries.size());
  }

  void ConvertBoldUnicodeCharset2Ascii_ok() {
    const QString unicodePath{mTDir.path()};
    const QStringList selectFileNames{"𝗔𝗕𝗖.txt", "𝘅𝘆𝘇.txt", "def.txt"};
    RenameWidget_ConvertBoldUnicodeCharset2Ascii pCovert;
    pCovert.init();
    pCovert.setModal(true);
    pCovert.m_nameExtIndependent->setCheckState(Qt::CheckState::Unchecked);

    pCovert.InitTextEditContent(unicodePath, selectFileNames);
    QCOMPARE(pCovert.mSelectedNames, (QStringList{"𝗔𝗕𝗖.txt", "𝘅𝘆𝘇.txt", "def.txt"}));
    QCOMPARE(pCovert.mRelToNameWithNoRoot, (QStringList{"", "", ""}));
    QCOMPARE(pCovert.mNames, (QStringList{"𝗔𝗕𝗖.txt", "𝘅𝘆𝘇.txt", "def.txt"}));
    QCOMPARE(pCovert.mExts, (QStringList{"", "", ""}));

    const QString& sNewName = pCovert.m_nBaseTE->toPlainText();
    const QString& sNewExt = pCovert.m_nExtTE->toPlainText();
    QCOMPARE(sNewName, "ABC.txt\nxyz.txt\ndef.txt");
    QCOMPARE(sNewExt, "\n\n");

    // 1. show command preview ok
    QVERIFY(pCovert.m_commandsPreview == nullptr);
    auto* pHelpBtn = pCovert.m_buttonBox->button(QDialogButtonBox::StandardButton::Help);
    QVERIFY(pHelpBtn != nullptr);
    pHelpBtn->setChecked(true);
    emit pHelpBtn->toggled(true);
    QVERIFY(pCovert.m_commandsPreview != nullptr);
    QVERIFY(!pCovert.m_commandsPreview->isHidden());

    // 2. hide command preview ok
    pHelpBtn->setChecked(false);
    emit pHelpBtn->toggled(false);
    QVERIFY(pCovert.m_commandsPreview != nullptr);
    QVERIFY(!pCovert.m_commandsPreview->isVisible());

    // 3. Apply rename ok
    QSignalSpy acceptedSpy(&pCovert, &QDialog::accepted);
    auto* pOkBtn = pCovert.m_buttonBox->button(QDialogButtonBox::StandardButton::Ok);
    QVERIFY(pOkBtn != nullptr);
    emit pOkBtn->clicked();
    QCOMPARE(acceptedSpy.count(), 1);
    acceptedSpy.clear();

    QDir unicodeDir{unicodePath, "", QDir::SortFlag::Name, QDir::Filter::Files};
    QStringList actualNames = unicodeDir.entryList();
    QCOMPARE(actualNames, (QStringList{"ABC.txt", "def.txt", "xyz.txt"}));
  }
};

#include "RenameWidget_ConvertBoldUnicodeCharset2AsciiTest.moc"
REGISTER_TEST(RenameWidget_ConvertBoldUnicodeCharset2AsciiTest, false)
