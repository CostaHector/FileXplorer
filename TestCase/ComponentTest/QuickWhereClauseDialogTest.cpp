#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include "MemoryKey.h"
#include "Logger.h"
#include <QSignalSpy>
#include <QPushButton>
#include "BeginToExposePrivateMember.h"
#include "QuickWhereClauseDialogMovie.h"
#include "QuickWhereClauseDialogCast.h"
#include "EndToExposePrivateMember.h"

class QuickWhereClauseDialogTest : public PlainTestSuite {
  Q_OBJECT
public:
  ~QuickWhereClauseDialogTest() {
    QuickWhereClauseDialogMock::mockWhereHistsList().clear();
    if (dialog != nullptr) {
      delete dialog;
    }
    dialog = nullptr;
  }

private slots:
  // constuctor, initTestCase, {{init, test_XXX,cleanup}_i}, cleanupTestCase, destructor
  void initTestCase() {
    dialog = new (std::nothrow) QuickWhereClauseDialogMovie;
    QVERIFY(dialog != nullptr);
    dialog->Init();
    dialog->mStrListModel->setStringList(initialHistory);

    dialogCast = new (std::nothrow) QuickWhereClauseDialogCast;
    QVERIFY(dialogCast != nullptr);
    dialogCast->Init();
    dialogCast->mStrListModel->setStringList(initialHistory);
  }

  void init() {
    QVERIFY(dialog != nullptr);
    dialog->ClearLineEditsListText();
    dialog->sizeHint();

    QVERIFY(dialogCast != nullptr);
    dialogCast->ClearLineEditsListText();
    dialogCast->sizeHint();
  }

  void conditionGeneration_movie() {
    // dialog->m_Name->setText("Henry Cavill");
    dialog->m_Size->setText(">1000000000");
    // emit dialog->m_Size->returnPressed();
    // QCOMPARE(dialog->GetWhereString(), R"(INSTR(`Name`,"Henry Cavill")>0 AND `Size`>1000000000)");

    // dialog->m_Name->setText("A&B");
    // emit dialog->m_Name->returnPressed();
    // QCOMPARE(dialog->GetWhereString(), R"((INSTR(`Name`,"A")>0 AND INSTR(`Name`,"B")>0) AND `Size`>1000000000)");

    dialog->m_Name->setText("A|B");
    emit dialog->m_Name->returnPressed();
    QCOMPARE(dialog->GetWhereString(), R"((`Name` LIKE "%A%" OR `Name` LIKE "%B%") AND `Size`>1000000000)");
  }

  void conditionGeneration_cast() {
    dialogCast->m_Name->setText("A|B");
    dialogCast->m_Size->setText(">1000000000");
    emit dialogCast->m_Name->returnPressed();
    QCOMPARE(dialogCast->GetWhereString(), R"((`Name` LIKE "%A%" OR `Name` LIKE "%B%") AND `Size`>1000000000)");
  }

  void test_History_Management_add() {
    QuickWhereClauseDialogMock::mockWhereHistsList().clear();
    dialog->onEditHistory();
    QCOMPARE(dialog->mStrListModel->rowCount(), 0);

    dialog->m_whereLineEdit->setText("`Rate`>8");
    QVERIFY(dialog->onAddAHistory());
    QCOMPARE(dialog->mStrListModel->rowCount(), 1);
    emit dialog->_ADD_WHERE_CLAUSE_TO_HISTORY->triggered(false); // can add the same one multiple times
    QCOMPARE(dialog->mStrListModel->rowCount(), 2);

    dialog->m_whereLineEdit->setText("`Rate`<5");
    QVERIFY(dialog->onAddAHistory());
    QCOMPARE(dialog->mStrListModel->rowCount(), 3);
  }

  void test_History_Management_remove() {
    QuickWhereClauseDialogMock::mockWhereHistsList() = QStringList{"A", "B"};
    dialog->onEditHistory();
    QCOMPARE(dialog->mStrListModel->rowCount(), 2);

    QVERIFY(dialog->onRemoveAHistory());
    QCOMPARE(dialog->mStrListModel->rowCount(), 1);
    emit dialog->_RMV_WHERE_CLAUSE_FROM_HISTORY->triggered(false);
    QCOMPARE(dialog->mStrListModel->rowCount(), 0);
    QVERIFY(dialog->onRemoveAHistory()); // already empty removed
    QCOMPARE(dialog->mStrListModel->rowCount(), 0);

    QCOMPARE(dialog->onEditHistory(), 2);
    QCOMPARE(dialog->onClearHistory(), 2);
    QCOMPARE(dialog->mStrListModel->rowCount(), 0);

    QCOMPARE(dialog->onEditHistory(), 2);
    emit dialog->_CLEAR_WHERE_CLAUSE_FROM_HISTORY->triggered(false);
    QCOMPARE(dialog->mStrListModel->rowCount(), 0);
  }

  void test_Empty_or_Blank_InputHandling() {
    dialog->onConditionsChanged();
    QVERIFY(dialog->GetWhereString().isEmpty()); // empty condition return empty clause

    int beforeSize = dialog->mStrListModel->rowCount();
    dialog->m_whereLineEdit->setText("");
    QVERIFY(!dialog->onAddAHistory());
    emit dialog->_ADD_WHERE_CLAUSE_TO_HISTORY->triggered(false);
    QCOMPARE(dialog->mStrListModel->rowCount(), beforeSize); // skip empty

    beforeSize = dialog->mStrListModel->rowCount();
    dialog->m_whereLineEdit->setText("\t\r\n ");
    QVERIFY(!dialog->onAddAHistory());
    emit dialog->_ADD_WHERE_CLAUSE_TO_HISTORY->triggered(false);
    QCOMPARE(dialog->mStrListModel->rowCount(), beforeSize); // skip blank char
  }

  void test_WriteUniqueHistoryToQSetting() {
    const QString beforeCfg = Configuration().value(MemoryKey::WHERE_CLAUSE_HISTORY.name, MemoryKey::WHERE_CLAUSE_HISTORY.v).toString();
    ON_SCOPE_EXIT {
      Configuration().setValue(MemoryKey::WHERE_CLAUSE_HISTORY.name, beforeCfg);
    };
    QuickWhereClauseDialogMock::mockWhereHistsList() = QStringList{"\t  \n", "", " A", "C ", "A \t", "\t B", "A"};
    dialog->onEditHistory();
    QCOMPARE(dialog->WriteUniqueHistoryToQSetting(), 3);

    const QString expectNewHistStr{"A\nC\nB"};
    QCOMPARE(Configuration().value(MemoryKey::WHERE_CLAUSE_HISTORY.name).toString(), expectNewHistStr);
  }

  void test_enter_will_not_close_the_dialog_and_f10_will() {
    dialog->m_Name->setFocus();
    QSignalSpy acceptedSpy(dialog, &QDialog::accepted);

    QKeyEvent enterPressEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    dialog->m_Name->keyPressEvent(&enterPressEvent);
    QVERIFY(enterPressEvent.isAccepted()); // will not express to its parent
    QCOMPARE(acceptedSpy.count(), 0);

    QKeyEvent enterReturnEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    dialog->m_Name->keyPressEvent(&enterReturnEvent);
    QVERIFY(enterReturnEvent.isAccepted()); // will not express to its parent
    QCOMPARE(acceptedSpy.count(), 0);

    QVERIFY(dialog->mDialogButtonBox != nullptr);
    QPushButton* pOk = dialog->mDialogButtonBox->button(QDialogButtonBox::StandardButton::Ok);
    QVERIFY(pOk != nullptr);
    emit pOk->clicked();
    QCOMPARE(acceptedSpy.count(), 1);
  }

private:
  QuickWhereClauseDialogMovie* dialog{nullptr};
  QuickWhereClauseDialogCast* dialogCast{nullptr};
  const QStringList initialHistory{"INSTR(`NAME`, \"\")>0", "`Size`>1024000000"};
};

#include "QuickWhereClauseDialogTest.moc"
REGISTER_TEST(QuickWhereClauseDialogTest, false)
