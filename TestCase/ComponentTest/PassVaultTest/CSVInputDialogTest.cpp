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
    DragDropTextEdit te{"RANDOM_AES_KEY"};
    QCOMPARE(te.placeholderText(), "Enter your text here or drag encrypt csv file here...");

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
    // 不真正读文件, 假定首次读取失败(文本损坏), 文本恢复后第二次读取成功
    const AccountInfo acc{"Type","Name","Account","Password","Others"};
    const QString plainText = acc.toCsvLine() + "\n";
    const std::pair<bool, QString> failedPair{false, ""}, succeedPair{true, plainText};
    MOCKER(AccountFileHelper::ParseEncryptCsvFile)
        .expects(exactly(2))              //
        .will(returnValue(failedPair))    // 1st: parse failed
        .then(returnValue(succeedPair));  // 2nd: parse again succeed

    const QString encFileFullPath = "enc_file1.csv";
    DragDropTextEdit te{"RANDOM_AES_KEY"};
    QCOMPARE(te.ParseEncryptCsvFileContents(encFileFullPath), false);
    QCOMPARE(te.toPlainText(), "Parse encrypt csv file[enc_file1.csv] failed");
    // 此处假定文本恢复了
    QCOMPARE(te.ParseEncryptCsvFileContents(encFileFullPath), true);
    QCOMPARE(te.toPlainText(), plainText);
  }

  void onSelectEncCsvFileToParse_ok() {
    const QString aesKey{"RANDOM_AES_KEY"};
    const QString encFileFullPath{"File.enc.csv"};
    MOCKER(QFileDialog::getOpenFileName)
        .expects(exactly(2))                  //
        .will(returnValue(QString{}))         // 1st: cancel
        .then(returnValue(encFileFullPath));  // 2nd: ok

    // empty file.enc.csv
    std::pair<bool, QString> succeedPair{true, ""};
    MOCKER(AccountFileHelper::ParseEncryptCsvFile)
        .expects(exactly(1))  //
        .with(eq(encFileFullPath), eq(aesKey))
        .will(returnValue(succeedPair));

    DragDropTextEdit te{aesKey};
    QVERIFY(te.mSelectEncCsvFile != nullptr);
    QCOMPARE(te.toPlainText(), ""); // no text at initial

    te.mSelectEncCsvFile->trigger();
    QCOMPARE(te.toPlainText(), "User cancel select encrypt file");

    QVERIFY(te.onSelectEncCsvFileToParse());
    QCOMPARE(te.toPlainText(), "");
  }

  void dragDropEnterEvent_ok() {
    const QString aesKey{"RANDOM_AES_KEY"};

    DragDropTextEdit te{aesKey};
    QPoint dragEnterPos{te.geometry().center()};

    // 没有url, 不走 ParseEncryptCsvFileContents
    {
      QMimeData textOnlyMimeData;
      textOnlyMimeData.setText("0 urls only text");
      QDragEnterEvent rejectDragEnter(dragEnterPos, Qt::IgnoreAction, &textOnlyMimeData, Qt::LeftButton, Qt::NoModifier);
      te.dragEnterEvent(&rejectDragEnter);
      QCOMPARE(rejectDragEnter.isAccepted(), false);

      QDropEvent ignoreDropEvent(dragEnterPos, Qt::IgnoreAction, &textOnlyMimeData, Qt::LeftButton, Qt::NoModifier);
      te.dropEvent(&ignoreDropEvent);
      QCOMPARE(ignoreDropEvent.isAccepted(), false);
    }

    const QString encFileFullPath{"File.enc.csv"};
    const std::pair<bool, QString> failedPair{false, ""}, succeedPair{true, ""};
    MOCKER(AccountFileHelper::ParseEncryptCsvFile)
        .expects(exactly(2))  //
        .will(returnValue(failedPair))
        .then(returnValue(succeedPair));

    // 两个url时, 不会接受drag/drop
    {
      QMimeData urlsMimeData;
      urlsMimeData.setText("2 urls");
      QList<QUrl> urlsList{QUrl::fromLocalFile(encFileFullPath), QUrl::fromLocalFile(encFileFullPath)};
      urlsMimeData.setUrls(urlsList);
      QDragEnterEvent acceptDragEnter(dragEnterPos, Qt::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
      te.dragEnterEvent(&acceptDragEnter);
      QCOMPARE(acceptDragEnter.isAccepted(), false);

      QDropEvent acceptDropEvent(dragEnterPos, Qt::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
      te.dropEvent(&acceptDropEvent);
      QCOMPARE(acceptDropEvent.isAccepted(), false);
    }

    // 有1个url, dropEvent时走 ParseEncryptCsvFileContents
    {
      QMimeData urlsMimeData;
      urlsMimeData.setText("1 urls");
      QList<QUrl> urlsList{QUrl::fromLocalFile(encFileFullPath)};
      urlsMimeData.setUrls(urlsList);

      QDragEnterEvent acceptDragEnter(dragEnterPos, Qt::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
      te.dragEnterEvent(&acceptDragEnter);
      QCOMPARE(acceptDragEnter.isAccepted(), true);

      QDropEvent acceptDropEvent(dragEnterPos, Qt::IgnoreAction, &urlsMimeData, Qt::LeftButton, Qt::NoModifier);
      te.dropEvent(&acceptDropEvent);  // 1st: mocker返回失败, 导致不接受
      QCOMPARE(acceptDropEvent.isAccepted(), false);
      QCOMPARE(te.toPlainText(), "Parse encrypt csv file[File.enc.csv] failed");

      // user fixed file content in "file1.csv"
      te.dropEvent(&acceptDropEvent);  // 2nd: mocker返回成功, 文件内容为空, 显示为空
      QCOMPARE(acceptDropEvent.isAccepted(), true);
      QCOMPARE(te.toPlainText(), "");
    }
  }

  void test_initialization() {
    const QString aesKey{"RANDOM_AES_KEY"};

    CSVInputDialog dialog{aesKey};
    QVERIFY(dialog.textEdit != nullptr);
    QVERIFY(dialog.buttonBox != nullptr);
    QVERIFY(dialog.mainLayout != nullptr);

    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Ok) != nullptr);
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Cancel) != nullptr);
    QVERIFY(dialog.buttonBox->button(QDialogButtonBox::Help) != nullptr);

    QCOMPARE(dialog.mainLayout->count(), 2);
    QCOMPARE(dialog.mainLayout->itemAt(0)->widget(), dialog.textEdit);
    QCOMPARE(dialog.mainLayout->itemAt(1)->widget(), dialog.buttonBox);

    QCOMPARE(dialog.windowTitle(), "Append record(s) from CSV input");
  }

  void onParseRequest_ok() {
    CSVInputDialog dialog{"RANDOM_AES_KEY"};
    // empty contens
    {
      const QString emptyContents;
      dialog.textEdit->setPlainText(emptyContents);
      QVERIFY(dialog.onParseRequest());
      QVERIFY(dialog.GetAppendAccounts().isEmpty());
      QCOMPARE(dialog.windowTitle(), "0 line(s) accounts found.");
    }

    // all 3 lines ok
    {
      const QString validCSV {
          "Email,Personal,user@example.com,pass123,Additional info\n"
          "Social Media,Facebook,fb_user,fb_pass,Notes with\\, comma\\nand newline\n"
          "Bank,Savings,bank_user,bank_pass,"
      };
      dialog.textEdit->setPlainText(validCSV);
      QVERIFY(dialog.onParseRequest());
      QCOMPARE(dialog.GetAppendAccounts().size(), 3);
      QCOMPARE(dialog.windowTitle(), "3 line(s) accounts found.");
    }

    // 1 in 3 lines failed
    {
      const QString partialCSV {
          "Email,Personal,user@example.com,pass123,Additional info\n"
          "Invalid Line\n"
          "Bank,Savings,bank_user,bank_pass,"
      };
      dialog.textEdit->setPlainText(partialCSV);
      QVERIFY(!dialog.onParseRequest());
      QCOMPARE(dialog.GetAppendAccounts().size(), 2);
      QCOMPARE(dialog.windowTitle(), "1 line(s) incomplete. Need fix at first");
    }

    // 3 in 3 lines failed
    {
      const QString invalidCSV {
          "Invalid Line 1\n"
          "Invalid Line 2\n"
          "Invalid Line 3"
      };
      dialog.textEdit->setPlainText(invalidCSV);
      QVERIFY(!dialog.onParseRequest());
      QCOMPARE(dialog.GetAppendAccounts().size(), 0);
      QCOMPARE(dialog.windowTitle(), "3 line(s) incomplete. Need fix at first");
    }
  }
};

#include "CSVInputDialogTest.moc"
REGISTER_TEST(CSVInputDialogTest, false)
