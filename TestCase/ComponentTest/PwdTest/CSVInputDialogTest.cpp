#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "CSVInputDialog.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "AccountStorage.h"
#include <QPushButton>
#include <QFileDialog>
#include <QAction>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class CSVInputDialogTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
  const QList<FsNodeEntry> mNodes{
      {"file1.csv", false, "contents not matter"},  //
      {"file2.mp4", false, "1"},                    //
  };
 private slots:
  void initTestCase() {  //
    QVERIFY(mTDir.IsValid());
    QCOMPARE(mTDir.createEntries(mNodes), 2);
  }

  void cleanupTestCase() {  //
  }

  void init() { GlobalMockObject::verify(); }

  void cleanup() { GlobalMockObject::reset(); }

  void DragDropTextEdit_default_ok() {
    DragDropTextEdit te;
    QVERIFY(te.mSelectEncCsvFile != nullptr);
    QVERIFY(te.mMenu != nullptr);
    QCOMPARE(te.acceptDrops(), true);
    te.contextMenuEvent(nullptr);
    te.dragEnterEvent(nullptr);
    te.dropEvent(nullptr);

    QMimeData mimeData;
    QVERIFY(!te.isContainsOneFile(mimeData));

    const QUrl fileUrl{QUrl::fromLocalFile(mTDir.itemPath("file1.csv"))};
    mimeData.setUrls(QList<QUrl>{fileUrl});
    QVERIFY(te.isContainsOneFile(mimeData));

    mimeData.setUrls(QList<QUrl>{fileUrl, fileUrl});
    QVERIFY(!te.isContainsOneFile(mimeData));
  }

  void ParseEncryptCsvFileContents_ok() {
    MOCKER(AccountStorage::ParseEncryptCsvFile)
        .expects(exactly(2))       //
        .will(returnValue(false))  // 1st: parse failed
        .then(returnValue(true));  // 2nd: parse again succeed

    QString parseFailedPath = mTDir.itemPath("file1.csv");
    DragDropTextEdit te;
    QCOMPARE(te.ParseEncryptCsvFileContents(parseFailedPath), false);
    QCOMPARE(te.toPlainText().contains("failed", Qt::CaseInsensitive), true);

    // here user changed the contents
    QString parseSucceedPath{"a valid encrypted file"};
    QCOMPARE(te.ParseEncryptCsvFileContents(parseSucceedPath), true);
    QCOMPARE(te.toPlainText().contains("failed", Qt::CaseInsensitive), false);
  }

  void onSelectEncCsvFileToParse_ok() {
    MOCKER(AccountStorage::ParseEncryptCsvFile)
        .expects(exactly(1))  //
        .will(returnValue(true));

    QString validContentsNotInital{"a valid path"};
    MOCKER(QFileDialog::getOpenFileName)
        .expects(exactly(2))                         //
        .will(returnValue(QString{}))                // 1st: cancel
        .then(returnValue(validContentsNotInital));  // 2nd: ok
    DragDropTextEdit te;
    te.mSelectEncCsvFile->trigger();
    QCOMPARE(te.toPlainText().contains("failed", Qt::CaseInsensitive), false);

    QCOMPARE(te.onSelectEncCsvFileToParse(), true);
    QCOMPARE(te.toPlainText().contains("failed", Qt::CaseInsensitive), false);
  }

  void dragDropEnterEvent_ok() {
    MOCKER(AccountStorage::ParseEncryptCsvFile)
        .expects(exactly(2))  //
        .will(returnValue(false))
        .then(returnValue(true));
    DragDropTextEdit te;
    QPoint dragEnterPos{te.geometry().center()};

    QMimeData textOnlyMimeData;
    textOnlyMimeData.setText("0 urls only text");
    QDragEnterEvent rejectDragEnter(dragEnterPos, Qt::IgnoreAction, &textOnlyMimeData, Qt::LeftButton, Qt::NoModifier);
    te.dragEnterEvent(&rejectDragEnter);
    QCOMPARE(rejectDragEnter.isAccepted(), false);

    QDropEvent ignoreDropEvent(dragEnterPos, Qt::IgnoreAction, &textOnlyMimeData, Qt::LeftButton, Qt::NoModifier);
    te.dropEvent(&ignoreDropEvent);
    QCOMPARE(ignoreDropEvent.isAccepted(), false);

    QMimeData urlsMimeData;
    urlsMimeData.setText("2 urls");
    QList<QUrl> urlsList{QUrl::fromLocalFile(mTDir.itemPath("file1.csv"))};
    urlsMimeData.setUrls(urlsList);
    QDragEnterEvent acceptDragEnter(dragEnterPos, Qt::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
    te.dragEnterEvent(&acceptDragEnter);
    QCOMPARE(acceptDragEnter.isAccepted(), true);

    QDropEvent acceptDropEvent(dragEnterPos, Qt::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
    te.dropEvent(&acceptDropEvent);  // 1st: parse ok
    QCOMPARE(acceptDropEvent.isAccepted(), false);
    QCOMPARE(te.toPlainText().contains("failed", Qt::CaseInsensitive), true);

    // user fixed file content in "file1.csv"

    te.dropEvent(&acceptDropEvent);  // 2nd: parse failed
    QCOMPARE(acceptDropEvent.isAccepted(), true);
    QCOMPARE(te.toPlainText().contains("failed", Qt::CaseInsensitive), false);
  }

  void test_initialization() {
    CSVInputDialog dialog;
    QVERIFY(dialog.textEdit != nullptr);
    QVERIFY(dialog.buttonBox != nullptr);
    QVERIFY(dialog.mainLayout != nullptr);

    QCOMPARE(dialog.textEdit->placeholderText(), "Enter your text here or drag encrypt csv file here...");

    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok) != nullptr);
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Cancel) != nullptr);
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Help) != nullptr);

    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());

    QCOMPARE(dialog.mainLayout->count(), 2);
    QCOMPARE(dialog.mainLayout->itemAt(0)->widget(), dialog.textEdit);
    QCOMPARE(dialog.mainLayout->itemAt(1)->widget(), dialog.buttonBox);

    QCOMPARE(dialog.windowTitle(), "Get record(s) from CSV input");
    QVERIFY(!dialog.windowIcon().isNull());
    dialog.sizeHint();
  }

  void test_raise_method() {
    CSVInputDialog dialog;
    dialog.setWindowTitle("Modified Title");
    dialog.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

    dialog.raise();

    QCOMPARE(dialog.windowTitle(), "Get record(s) from CSV input");
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_empty_input() {
    CSVInputDialog dialog;
    dialog.textEdit->setText("");
    dialog.onHelpRequest();

    QVERIFY(dialog.tempAccounts.isEmpty());
    QCOMPARE(dialog.windowTitle(), "0/0 line(s) are ok. no unexpected line(s) find");
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_valid_input() {
    QString validCSV =
        "Email,Personal,user@example.com,pass123,Additional info\n"
        "Social Media,Facebook,fb_user,fb_pass,Notes with\\, comma\\nand newline\n"
        "Bank,Savings,bank_user,bank_pass,";
    CSVInputDialog dialog;
    dialog.textEdit->setText(validCSV);
    dialog.onHelpRequest();

    QCOMPARE(dialog.tempAccounts.size(), 3);
    QCOMPARE(dialog.tempAccounts[0].typeStr, "Email");
    QCOMPARE(dialog.tempAccounts[1].accountStr, "fb_user");
    QCOMPARE(dialog.tempAccounts[2].othersStr, "");

    QCOMPARE(dialog.windowTitle(), "3/3 line(s) are ok. no unexpected line(s) find");
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_partial_valid_input() {
    CSVInputDialog dialog;
    QString partialCSV =
        "Email,Personal,user@example.com,pass123,Additional info\n"
        "Invalid Line\n"
        "Bank,Savings,bank_user,bank_pass,";
    dialog.textEdit->setText(partialCSV);
    dialog.onHelpRequest();

    QCOMPARE(dialog.tempAccounts.size(), 2);
    QCOMPARE(dialog.tempAccounts[0].typeStr, "Email");
    QCOMPARE(dialog.tempAccounts[1].typeStr, "Bank");

    QCOMPARE(dialog.windowTitle(), "2/3 line(s) are ok. Others need fixed at first.");
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_onHelpRequest_invalid_input() {
    QString invalidCSV =
        "Invalid Line 1\n"
        "Invalid Line 2\n"
        "Invalid Line 3";
    CSVInputDialog dialog;
    dialog.textEdit->setText(invalidCSV);
    dialog.onHelpRequest();

    QVERIFY(dialog.tempAccounts.isEmpty());

    QCOMPARE(dialog.windowTitle(), "0/3 line(s) are ok. Others need fixed at first.");
    QVERIFY(!dialog.buttonBox->button(QDialogButtonBox::Ok)->isEnabled());
  }

  void test_button_signals() {
    CSVInputDialog dialog;

    emit dialog.buttonBox->accepted();
    emit dialog.buttonBox->rejected();
    emit dialog.buttonBox->helpRequested();
  }
};

#include "CSVInputDialogTest.moc"
REGISTER_TEST(CSVInputDialogTest, false)
