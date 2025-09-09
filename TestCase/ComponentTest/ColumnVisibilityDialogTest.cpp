#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ColumnVisibilityDialog.h"
#include "EndToExposePrivateMember.h"

class ColumnVisibilityDialogTest : public PlainTestSuite {
  Q_OBJECT
public:
  ColumnVisibilityDialogTest() : PlainTestSuite{} {
    fprintf(stdout, "ColumnVisibilityDialogTest object[%d] created\n", objectCnt++);
    std::fflush(stdout);
  }
  ~ColumnVisibilityDialogTest() {
    if (dialog != nullptr) delete dialog;
    dialog = nullptr;
  }
private slots:
  // constuctor, initTestCase, {{init, test_XXX,cleanup}_i}, cleanupTestCase, destructor
  void initTestCase() {
    // so follows can be moved to constructor if ColumnVisibilityDialogTest inst is not global variable(because of widget must create after QApplication)
    const QStringList headers{"Name", "Age", "Score"};
    dialog = new (std::nothrow) ColumnVisibilityDialog(headers, initSwitches, "TestDialog");
    QVERIFY(dialog != nullptr);
  }

  void test_switches_count_less_then_columns() {
    const QStringList headers{"Name", "Age", "Score", "Gender", "Country", "Married"};
    QVERIFY(initSwitches.size() < headers.size());
    auto* pInvalid = new (std::nothrow) ColumnVisibilityDialog(headers, initSwitches, "TooLessSwitchesDialog");
    QVERIFY(pInvalid != nullptr);
    QCOMPARE(pInvalid->mSelectToolButton, nullptr);
    QCOMPARE(pInvalid->m_layout, nullptr);
    QCOMPARE(pInvalid->buttons, nullptr);
    QVERIFY(pInvalid->m_checkboxes.isEmpty());
    delete pInvalid;
    pInvalid = nullptr;
  }

  void testSetAllCheckboxes() {
    QCOMPARE(objectCnt, 1);
    QVERIFY(dialog != nullptr);
    dialog->setAllCheckboxes(true);
    QCOMPARE(dialog->getSwitches(), QString("111"));
    dialog->setAllCheckboxes(false);
    QCOMPARE(dialog->getSwitches(), QString("000"));
  }

  void testToggleAllCheckboxes() {
    QCOMPARE(objectCnt, 1);
    QVERIFY(dialog != nullptr);
    dialog->revertCheckboxes(initSwitches);
    QCOMPARE(dialog->getSwitches(), initSwitches.left(3));

    dialog->toggleAllCheckboxes();
    QCOMPARE(dialog->getSwitches(), QString("010"));

    dialog->toggleAllCheckboxes();
    QCOMPARE(dialog->getSwitches(), initSwitches.left(3));
  }

  void testRevertCheckboxes() {
    QCOMPARE(objectCnt, 1);
    QVERIFY(dialog != nullptr);
    dialog->setAllCheckboxes(true);
    QVERIFY(dialog->getSwitches() != initSwitches.left(3));

    dialog->revertCheckboxes(initSwitches);
    QCOMPARE(dialog->getSwitches(), initSwitches.left(3));
  }

  void test_PartOfColumnCheckBox_checked() {
    QCOMPARE(objectCnt, 1);
    QVERIFY(dialog != nullptr);

    dialog->setAllCheckboxes(true);
    QCOMPARE(dialog->getSwitches(), QString("111"));

    QVERIFY(!dialog->m_checkboxes.isEmpty());
    auto* firstCheckBox = dialog->m_checkboxes.front();

    QSignalSpy checkSpy(firstCheckBox, &QCheckBox::clicked);
    QTest::mouseClick(firstCheckBox, Qt::LeftButton);
    QCOMPARE(checkSpy.count(), 1);
    QCOMPARE(dialog->getSwitches(), QString("011"));

    dialog->toggleAllCheckboxes();
    QCOMPARE(dialog->getSwitches(), QString("100"));
  }

  void cleanupTestCase() {
    if (dialog != nullptr) delete dialog;
    dialog = nullptr;
  }

private:
  ColumnVisibilityDialog* dialog {nullptr};
  static const QString initSwitches;
  static int objectCnt;
};
int ColumnVisibilityDialogTest::objectCnt{0};
const QString ColumnVisibilityDialogTest::initSwitches{"10111"}; // 初始开关：冗余最后两列

#include "ColumnVisibilityDialogTest.moc"
REGISTER_TEST(ColumnVisibilityDialogTest, false)
