#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "PropertiesWindow.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "PropertiesWindowActions.h"

class PropertiesWindowTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir testDir;

 private slots:
  void initTestCase() {
    Configuration().clear();
    QList<FsNodeEntry> entries = {{"file1.txt", false, "Content of file1"},
                                  {"file2.txt", false, "Content of file2"},
                                  {"video1.mp4", false, ""},
                                  {"video2.mp4", false, ""},
                                  {"folder1/subfile.txt", false, "Sub content"},
                                  {"folder2", true, ""}};
    QVERIFY(testDir.IsValid());
    QCOMPARE(testDir.createEntries(entries), entries.size());
  }

  void cleanupTestCase() {
    Configuration().clear();  //
  }

  void testEmptyItems() {
    PropertiesWindow propWindow;
    // 测试空项目列表
    propWindow(QStringList());
    // 验证窗口标题
    QCOMPARE(propWindow.windowTitle(), "Property | [0] item(s)");
    // 验证显示内容
    QCOMPARE(propWindow.m_propertiesInfoTextEdit->toPlainText(), "Nothing selected");

    //
    propWindow(nullptr, nullptr);
  }

  void testSingleFile() {
    PropertiesWindow propWindow;

    // 测试单个文件
    QString filePath = testDir.itemPath("file1.txt");
    propWindow({filePath});

    // 验证窗口标题
    QCOMPARE(propWindow.windowTitle(), QString("Property | [1] item(s)"));

    // 验证通用信息初始化
    QVERIFY(!propWindow.m_commonInfomation.isEmpty());
    QVERIFY(propWindow.m_commonInfomation.contains("Contents: 1 file(s), 0 folder(s)"));
    QVERIFY(propWindow.m_commonInfomation.contains("Size:"));

    // 验证MD5信息未初始化（默认不显示）
    QVERIFY(propWindow.m_fileIdentifier.isEmpty());

    // 验证时长信息未初始化（默认不显示）
    QVERIFY(propWindow.m_durations.isEmpty());
  }

  void testMultipleFiles() {
    PropertiesWindow propWindow;

    // 测试多个文件
    QStringList items = {
        testDir.itemPath("file1.txt"),   //
        testDir.itemPath("folder1"),     // 1 file under it
        testDir.itemPath("video1.mp4"),  //
    };
    propWindow(items);

    // 验证窗口标题
    QCOMPARE(propWindow.windowTitle(), QString("Property | [3] item(s)"));

    // 验证通用信息初始化
    QVERIFY(!propWindow.m_commonInfomation.isEmpty());
    const QString commonInfo = propWindow.m_commonInfomation;
    QVERIFY(commonInfo.contains("Contents: 3 file(s)"));
    QVERIFY(commonInfo.contains("Size:"));

    // 验证MD5信息未初始化（默认不显示）
    QVERIFY(propWindow.m_fileIdentifier.isEmpty());

    // 验证时长信息未初始化（默认不显示）
    QVERIFY(propWindow.m_durations.isEmpty());
  }

  void testFileSizeDisplay() {
    PropertiesWindow propWindow;

    // 启用文件大小显示
    g_propertiesWindowAct().SHOW_FILES_SIZE->setChecked(true);

    // 测试文件
    QStringList items = {
        testDir.itemPath("file1.txt"),  //
        testDir.itemPath("folder1"),    // 1 file under it
    };
    propWindow(items);

    // 验证显示内容
    QString html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains(PropertiesWindow::STRING_SPLITTER));
    QVERIFY(html.contains("Contents: 2 file(s), 1 folder(s)"));
    QVERIFY(html.contains("Size:"));

    // 重置状态
    g_propertiesWindowAct().SHOW_FILES_SIZE->setChecked(false);
  }

  void testDurationDisplay() {
    PropertiesWindow propWindow;

    // 启用时长显示
    g_propertiesWindowAct().SHOW_VIDS_DURATION->setChecked(true);

    // 测试视频文件
    QStringList videoFiles = {
        testDir.itemPath("video1.mp4"),  //
        testDir.itemPath("video2.mp4"),  //
    };
    propWindow(videoFiles);

    // 验证显示内容
    QString html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains(PropertiesWindow::STRING_SPLITTER));

    // 重置状态
    g_propertiesWindowAct().SHOW_VIDS_DURATION->setChecked(false);
  }

  void testMd5Display() {
    PropertiesWindow propWindow;

    // 启用MD5显示
    g_propertiesWindowAct().SHOW_FILES_MD5->setChecked(true);

    // 测试文件
    QString filePath = testDir.itemPath("file1.txt");
    propWindow({filePath});

    // 验证显示内容
    QString html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains(PropertiesWindow::STRING_SPLITTER));
    QVERIFY(html.contains("MD5"));

    // 验证MD5值计算正确
    QString expectedMd5 = QCryptographicHash::hash("Content of file1", QCryptographicHash::Md5).toHex();
    QVERIFY(html.contains(expectedMd5));

    // 重置状态
    g_propertiesWindowAct().SHOW_FILES_MD5->setChecked(false);
  }

  void testSettingsPersistence() {
    PropertiesWindow propWindow;

    // 保存初始几何信息
    QByteArray originalGeometry = propWindow.saveGeometry();

    // 模拟关闭事件
    QCloseEvent closeEvent;
    propWindow.closeEvent(&closeEvent);

    // 验证设置已保存
    QByteArray savedGeometry = Configuration().value("PropertiesWindowGeometry").toByteArray();
    QCOMPARE(savedGeometry, originalGeometry);

    // 修改几何信息
    propWindow.setGeometry(100, 100, 800, 600);

    // 读取设置
    propWindow.ReadSetting();
  }

  void testToggleDisplayOptions() {
    PropertiesWindow propWindow;

    // 设置测试数据
    QStringList items = {testDir.itemPath("file1.txt"), testDir.itemPath("video1.mp4"), testDir.itemPath("folder1")};
    propWindow(items);

    // 启用文件大小显示
    g_propertiesWindowAct().SHOW_FILES_SIZE->setChecked(true);
    propWindow.UpdateMessage();
    QString html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains("Contents:"));
    QVERIFY(!html.contains("MD5"));

    // 启用时长显示
    g_propertiesWindowAct().SHOW_VIDS_DURATION->setChecked(true);
    propWindow.UpdateMessage();
    html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains("Contents:"));
    QVERIFY(!html.contains("MD5"));

    // 启用MD5显示
    g_propertiesWindowAct().SHOW_FILES_MD5->setChecked(true);
    propWindow.UpdateMessage();
    html = propWindow.m_propertiesInfoTextEdit->toHtml();
    QVERIFY(html.contains("Contents:"));
    QVERIFY(html.contains("MD5"));

    // 禁用所有显示选项
    g_propertiesWindowAct().SHOW_FILES_SIZE->setChecked(false);
    g_propertiesWindowAct().SHOW_VIDS_DURATION->setChecked(false);
    g_propertiesWindowAct().SHOW_FILES_MD5->setChecked(false);
    propWindow.UpdateMessage();
    html = propWindow.m_propertiesInfoTextEdit->toPlainText();
    QVERIFY(html.isEmpty());
  }
};

#include "PropertiesWindowTest.moc"
REGISTER_TEST(PropertiesWindowTest, false)
