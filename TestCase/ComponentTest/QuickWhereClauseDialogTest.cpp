#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "QuickWhereClauseDialog.h"
#include "EndToExposePrivateMember.h"

class QuickWhereClauseDialogTest : public PlainTestSuite {
  Q_OBJECT
public:
  QuickWhereClauseDialogTest() : PlainTestSuite{} {
    LOG_D("QuickWhereClauseDialogTest object created\n");
  }
  ~QuickWhereClauseDialogTest() {
    if (dialog != nullptr) delete dialog;
    dialog = nullptr;
  }

private slots:
  // constuctor, initTestCase, {{init, test_XXX,cleanup}_i}, cleanupTestCase, destructor
  void initTestCase() {
    dialog = new (std::nothrow) QuickWhereClauseDialog;
    QVERIFY(dialog != nullptr);
    dialog->mStrListModel->setStringList(initialHistory);
  }

  void cleanupTestCase() {
    if (dialog != nullptr) delete dialog;
    dialog = nullptr;
  }

  void init() {
    QVERIFY(dialog != nullptr);
    dialog->ClearLineEditsListText();
  }

  void testConditionGeneration() {
    dialog->SetStrPatternCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    dialog->m_Name->setText("Henry Cavill");
    dialog->m_Size->setText(">1000000000");
    emit dialog->m_Size->returnPressed();
    QCOMPARE(dialog->GetWhereString(),
             R"(INSTR(`Name`,"Henry Cavill")>0 AND `Size`>1000000000)");

    dialog->m_Name->setText("A&B");
    emit dialog->m_Name->returnPressed();
    QCOMPARE(dialog->GetWhereString(),
             R"((INSTR(`Name`,"A")>0 AND INSTR(`Name`,"B")>0) AND `Size`>1000000000)");

    dialog->m_Name->setText("A|B");
    emit dialog->m_Name->returnPressed();
    QCOMPARE(dialog->GetWhereString(),
             R"((INSTR(`Name`,"A")>0 OR INSTR(`Name`,"B")>0) AND `Size`>1000000000)");

    dialog->SetStrPatternCaseSensitive(Qt::CaseSensitivity::CaseInsensitive);
    QCOMPARE(dialog->GetWhereString(),
             R"((`Name` LIKE "%A%" OR `Name` LIKE "%B%") AND `Size`>1000000000)");
  }

  void test_History_Management_add() {
    dialog->SetStrPatternCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    dialog->newWhereHistsList.clear();
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
    dialog->SetStrPatternCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    dialog->newWhereHistsList = QStringList{"A", "B"};
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
    QVERIFY(dialog->GetWhereString().isEmpty());  // empty condition return empty clause

    int beforeSize = dialog->mStrListModel->rowCount();
    dialog->m_whereLineEdit->setText("");
    QVERIFY(!dialog->onAddAHistory());
    emit dialog->_ADD_WHERE_CLAUSE_TO_HISTORY->triggered(false);
    QCOMPARE(dialog->mStrListModel->rowCount(), beforeSize);  // skip empty

    beforeSize = dialog->mStrListModel->rowCount();
    dialog->m_whereLineEdit->setText("\t\r\n ");
    QVERIFY(!dialog->onAddAHistory());
    emit dialog->_ADD_WHERE_CLAUSE_TO_HISTORY->triggered(false);
    QCOMPARE(dialog->mStrListModel->rowCount(), beforeSize);  // skip blank char
  }

  void test_WriteUniqueHistoryToQSetting(){
    dialog->SetStrPatternCaseSensitive(Qt::CaseSensitivity::CaseSensitive);
    const QString beforeCfg = Configuration().value(MemoryKey::WHERE_CLAUSE_HISTORY.name, MemoryKey::WHERE_CLAUSE_HISTORY.v).toString();
    ON_SCOPE_EXIT {
      Configuration().setValue(MemoryKey::WHERE_CLAUSE_HISTORY.name, beforeCfg);
    };
    dialog->newWhereHistsList = QStringList{"\t  \n", "", " A", "C ", "A \t", "\t B", "A"};
    dialog->onEditHistory();
    QCOMPARE(dialog->WriteUniqueHistoryToQSetting(), 3);

    const QString expectNewHistStr{"A\nC\nB"};
    QCOMPARE(Configuration().value(MemoryKey::WHERE_CLAUSE_HISTORY.name).toString(), expectNewHistStr);
  }

private:
  QuickWhereClauseDialog* dialog {nullptr};
  const QStringList initialHistory{"INSTR(`NAME`, \"\")>0", "`Size`>1024000000"};
};

#include "QuickWhereClauseDialogTest.moc"
REGISTER_TEST(QuickWhereClauseDialogTest, false)
