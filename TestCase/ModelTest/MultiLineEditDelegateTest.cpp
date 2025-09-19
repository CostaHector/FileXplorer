#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>

#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "MultiLineEditDelegate.h"
#include "EndToExposePrivateMember.h"
#include "ModelTestHelper.h"

#include <QTableView>
#include <QStandardItemModel>
#include <QPlainTextEdit>
#include <QHeaderView>

class MultiLineEditDelegateTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void firstColumn_PlainTextEditor_ok() {
    QStandardItemModel model;
    ModelTestHelper::InitStdItemModel(model, "Item At (%1, %2)\nSecondLine", 3, 2);

    MultiLineEditDelegate delegate;

    QTableView tableView;
    tableView.setModel(&model);
    tableView.setItemDelegateForColumn(0, &delegate);
    tableView.setEditTriggers(QAbstractItemView::EditKeyPressed);
    tableView.setColumnWidth(0, 200); // ensure first column can show the first without wrap

    // select cell (0, 0)
    QModelIndex frontIndex = model.index(0, 0);
    tableView.setCurrentIndex(frontIndex);
    tableView.setFocus();

    // 0. not in edit mode. editor no available
    QTRY_VERIFY(tableView.findChild<QPlainTextEdit*>("MultiLineEditor") == nullptr);

    // 1. into edit mode, editor available
    QTest::keyClick(&tableView, Qt::Key_F2);
    QPlainTextEdit* plainTextEdit = nullptr;
    QTRY_VERIFY((plainTextEdit = tableView.findChild<QPlainTextEdit*>("MultiLineEditor")) != nullptr);

    // contents at cell (0, 0) is correct
    QCOMPARE(plainTextEdit->toPlainText(), "Item At (0, 0)\nSecondLine");

    // new line insert after first line ok
    plainTextEdit->moveCursor(QTextCursor::MoveOperation::EndOfLine);
    QTest::keyClick(plainTextEdit, Qt::Key_Enter);
    QTRY_VERIFY(tableView.findChild<QPlainTextEdit*>("MultiLineEditor") != nullptr);
    QCOMPARE(plainTextEdit->toPlainText(), "Item At (0, 0)\n\nSecondLine");

    // cancel edit, editor not available, content at cell unchanged
    QTest::keyClick(plainTextEdit, Qt::Key_Escape);
    QTRY_VERIFY((plainTextEdit = tableView.findChild<QPlainTextEdit*>("MultiLineEditor")) == nullptr);
    QCOMPARE(model.itemFromIndex(frontIndex)->text(), "Item At (0, 0)\nSecondLine");

    // 2. into edit mode again
    QTest::keyClick(&tableView, Qt::Key_F2);
    QTRY_VERIFY((plainTextEdit = tableView.findChild<QPlainTextEdit*>("MultiLineEditor")) != nullptr);

    // delete all contents
    plainTextEdit->selectAll();
    QTest::keyClick(plainTextEdit, Qt::Key::Key_Backspace);
    QCOMPARE(plainTextEdit->toPlainText(), "");

    // save, editor not at front row
    QTest::keyClick(&tableView, Qt::Key::Key_Tab);
    QVERIFY(tableView.currentIndex() != frontIndex);
    QCOMPARE(model.itemFromIndex(frontIndex)->text(), "");
  }
};

#include "MultiLineEditDelegateTest.moc"
REGISTER_TEST(MultiLineEditDelegateTest, false)
