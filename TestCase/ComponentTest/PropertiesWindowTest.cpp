#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"

#include <QSignalSpy>

#include "Logger.h"
#include "Configuration.h"
#include "BeginToExposePrivateMember.h"
#include "PropertiesWindow.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "PropertiesWindowActions.h"

class PropertiesWindowTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
  const PropertiesWindowActions& actInst = g_propertiesWindowAct();

 private slots:
  void initTestCase() {
    Configuration().clear();
    QList<FsNodeEntry> entries = {{"file1.txt", false, "Content of file1"},
                                  {"file2.txt", false, "Content of file2"},
                                  {"video1.mp4", false, ""},
                                  {"video2.mp4", false, ""},
                                  {"folder1/subfile.txt", false, "Sub content"},
                                  {"folder2", true, ""}};
    QVERIFY(mTDir.IsValid());
    QCOMPARE(mTDir.createEntries(entries), entries.size());

    QCOMPARE(actInst.SHOW_FILES_MD5->isChecked(), false);
    QCOMPARE(actInst.SHOW_VIDS_DURATION->isChecked(), false);
  }

  void cleanupTestCase() {
    Configuration().clear();  //
  }

  void no_row_selected_ok() {
    PropertiesWindow propWindow;
    propWindow.showEvent(nullptr);  // will not crash

    QVERIFY(propWindow(QStringList{}));
    QCOMPARE(propWindow.windowTitle(), "Property | [0] item(s)");
    QCOMPARE(propWindow.m_propertiesInfoTextEdit->toPlainText(), "Nothing selected");

    QVERIFY(propWindow(QList<qint64>{}, QList<int>{}));
    QCOMPARE(propWindow.windowTitle(), "Property | [0] item(s)");
    QCOMPARE(propWindow.m_propertiesInfoTextEdit->toPlainText(), "Nothing selected");
  }

  void singleFile_ok() {
    PropertiesWindow propWindow;

    propWindow({mTDir.itemPath("file1.txt")});
    QCOMPARE(propWindow.windowTitle(), QString("Property | [1] item(s)"));

    QCOMPARE(propWindow.m_commonInfomation.isEmpty(), false);
    QVERIFY(propWindow.m_commonInfomation.contains("Contents: 1 file(s), 0 folder(s)"));
    QVERIFY(propWindow.m_commonInfomation.contains("Size:"));

    QVERIFY(propWindow.m_fileIdentifier.isEmpty());
    QVERIFY(propWindow.m_durations.isEmpty());
  }

  void multipleFiles_ok() {
    PropertiesWindow propWindow;
    QStringList items = {
        mTDir.itemPath("file1.txt"),   //
        mTDir.itemPath("folder1"),     // 1 file under it
        mTDir.itemPath("video1.mp4"),  //
    };
    QCOMPARE(propWindow(items), true);
    QCOMPARE(propWindow.windowTitle(), QString("Property | [3] item(s)"));

    QVERIFY(!propWindow.m_commonInfomation.isEmpty());
    const QString commonInfo = propWindow.m_commonInfomation;
    QVERIFY(commonInfo.contains("Contents: 3 file(s)"));
    QVERIFY(commonInfo.contains("Size:"));

    QVERIFY(propWindow.m_fileIdentifier.isEmpty());

    QVERIFY(propWindow.m_durations.isEmpty());
  }

  void fileSizeDisplay_ok() {
    actInst.SHOW_FILES_SIZE->setChecked(true);
    OnScopeExit {
      actInst.SHOW_FILES_SIZE->setChecked(false);
    };

    PropertiesWindow propWindow;
    QStringList items = {
        mTDir.itemPath("file1.txt"),  //
        mTDir.itemPath("folder1"),    // 1 file under it
    };
    propWindow(items);
    QString html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains(PropertiesWindow::STRING_SPLITTER));
    QVERIFY(html.contains("Contents: 2 file(s), 1 folder(s)"));
    QVERIFY(html.contains("Size:"));
  }

  void duration_display_stringList_ok() {
    actInst.SHOW_VIDS_DURATION->setChecked(true);
    OnScopeExit {
      actInst.SHOW_VIDS_DURATION->setChecked(false);
    };
    QStringList videoFiles{
        mTDir.itemPath("video1.mp4"),  //
        mTDir.itemPath("video2.mp4"),  //
    };
    PropertiesWindow propWindow;
    propWindow(videoFiles);
    QString html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains(PropertiesWindow::STRING_SPLITTER));
  }

  void duration_display_intList_ok() {
    actInst.SHOW_VIDS_DURATION->setChecked(true);
    actInst.SHOW_FILES_SIZE->setChecked(true);
    OnScopeExit {
      actInst.SHOW_VIDS_DURATION->setChecked(false);
      actInst.SHOW_FILES_SIZE->setChecked(false);
    };
    QList<qint64> fileSizes{
        1 * 1024,         //
        1 * 1024 * 1024,  //
        1,                //
    };
    QList<int> videoDurations{
        60 * 1000,  //
        30 * 1000,  //
        10 * 1000,  //
    };
    PropertiesWindow propWindow;
    QVERIFY(propWindow(fileSizes, videoDurations));
    QString html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains("3 file(s) durations: 00:01:40"));
    QVERIFY(html.contains("3 file(s) sizes: 0'1'1'1"));
  }

  void md5_display_ok() {
    actInst.SHOW_FILES_MD5->setChecked(true);
    OnScopeExit {
      actInst.SHOW_FILES_MD5->setChecked(false);
    };

    PropertiesWindow propWindow;
    QString filePath = mTDir.itemPath("file1.txt");
    propWindow({filePath});

    QString html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains(PropertiesWindow::STRING_SPLITTER));
    QVERIFY(html.contains("MD5"));

    QString expectedMd5 = QCryptographicHash::hash("Content of file1", QCryptographicHash::Md5).toHex();
    QVERIFY(html.contains(expectedMd5));
  }

  void settings_persistence_ok() {
    PropertiesWindow propWindow;
    QByteArray originalGeometry = propWindow.saveGeometry();
    QCloseEvent closeEvent;
    propWindow.closeEvent(&closeEvent);

    QByteArray savedGeometry = Configuration().value("Geometry/PropertiesWindow").toByteArray();
    QCOMPARE(savedGeometry, originalGeometry);
    propWindow.setGeometry(100, 100, 800, 600);
  }

  void toggle_display_options_ok() {
    OnScopeExit {
      actInst.SHOW_FILES_SIZE->setChecked(false);
      actInst.SHOW_VIDS_DURATION->setChecked(false);
      actInst.SHOW_FILES_MD5->setChecked(false);
    };

    PropertiesWindow propWindow;

    // 设置测试数据
    QStringList items = {mTDir.itemPath("file1.txt"), mTDir.itemPath("video1.mp4"), mTDir.itemPath("folder1")};
    propWindow(items);

    // 启用文件大小显示
    actInst.SHOW_FILES_SIZE->setChecked(true);
    propWindow.UpdateMessage();
    QString html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains("Contents:"));
    QVERIFY(!html.contains("MD5"));

    // 启用时长显示
    actInst.SHOW_VIDS_DURATION->setChecked(true);
    propWindow.UpdateMessage();
    html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains("Contents:"));
    QVERIFY(!html.contains("MD5"));

    // 启用MD5显示
    actInst.SHOW_FILES_MD5->setChecked(true);
    propWindow.UpdateMessage();
    html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains("Contents:"));
    QVERIFY(html.contains("MD5"));

    // 禁用所有显示选项
    actInst.SHOW_FILES_SIZE->setChecked(false);
    actInst.SHOW_VIDS_DURATION->setChecked(false);
    actInst.SHOW_FILES_MD5->setChecked(false);
    propWindow.UpdateMessage();
    html = propWindow.m_propertiesInfoTextEdit->toPlainText();
    QVERIFY(html.isEmpty());
  }
};

#include "PropertiesWindowTest.moc"
REGISTER_TEST(PropertiesWindowTest, false)
