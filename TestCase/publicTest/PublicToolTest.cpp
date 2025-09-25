#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "PublicTool.h"
#include "TDir.h"
#include "PublicVariable.h"
#include "OnScopeExit.h"
#include "MemoryKey.h"
#include <QPushButton>

class PublicToolTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void test_mvToANewPath() {
    QString mvToANewPath = "5";
    QAction act1{"1", this};
    QAction act2{"2", this};
    QAction act3{"3", this};

    QActionGroup ag{this};
    ag.addAction(&act1);
    ag.addAction(&act2);
    ag.addAction(&act3);

    const QStringList& expectList{"5", "1", "2"};
    const QString& expectStr = expectList.join('\n');
    QCOMPARE(MoveToNewPathAutoUpdateActionText(mvToANewPath, &ag), expectStr);
  }

  void test_mvToANewPath_again() {
    QString mvToANewPath = "3";
    QAction act1{"1", this};
    QAction act2{"2", this};
    QAction act3{"3", this};
    QAction act4{"4", this};
    QAction act5{"5", this};

    QActionGroup ag{this};
    ag.addAction(&act1);
    ag.addAction(&act2);
    ag.addAction(&act3);
    ag.addAction(&act4);
    ag.addAction(&act5);

    const QStringList& expectList{"3", "1", "2", "4", "5"};
    const QString& expectStr = expectList.join('\n');
    QCOMPARE(MoveToNewPathAutoUpdateActionText(mvToANewPath, &ag), expectStr);
  }

  void file_read_write_ok() {
    const QString inexistFileTextPath{"any random inexists path.txt"};
    QCOMPARE(FileTool::GetLastNLinesOfFile(inexistFileTextPath, 10), "");
    QCOMPARE(FileTool::TextReader(inexistFileTextPath), "");

    TDir tDir;
    tDir.touch("randomTextFile.txt", "0\n1\n2\n3\n4\n5\n6\n7\n8\n9");
    const QString textFileAbsPath = tDir.itemPath("randomTextFile.txt");

    QByteArray last0ba = FileTool::GetLastNLinesOfFile(textFileAbsPath, 0);
    QCOMPARE(last0ba, "");

    QByteArray last1ba = FileTool::GetLastNLinesOfFile(textFileAbsPath, 1);
    QCOMPARE(last1ba, "9");

    QByteArray last5ba = FileTool::GetLastNLinesOfFile(textFileAbsPath, 5);
    QCOMPARE(last5ba, "5\n6\n7\n8\n9");

    QCOMPARE(QFile::exists(textFileAbsPath), true); // file already exist. not override with OpenModeFlag::NewOnly
    QCOMPARE(FileTool::TextWriter(textFileAbsPath,
                                  "Cannot override", //
                                  QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::NewOnly),
             false);
    QByteArray lastAllba = FileTool::GetLastNLinesOfFile(textFileAbsPath, 100);
    QCOMPARE(lastAllba, "0\n1\n2\n3\n4\n5\n6\n7\n8\n9");

    QCOMPARE(FileTool::TextWriter(textFileAbsPath,
                                  "Can only override", //
                                  QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::ExistingOnly),
             true);
    QCOMPARE(FileTool::TextReader(textFileAbsPath), "Can only override");
  }

  void CreateUserPath_ok() { // UserPath is Service Running Precondition
    QCOMPARE(CreateUserPath(), true);


    QCOMPARE(QFile::exists(SystemPath::WORK_PATH()), true);


    // namespace SystemPath {
    // const QString& HOME_PATH();
    // inline const QString& WORK_PATH() {
    //   static const QString path = HOME_PATH() + "/" + PROJECT_NAME;
    //   return path;
    // }
    // inline const QString& STARRED_PATH() {
    //   static const QString path = HOME_PATH() + "/Documents";
    //   return path;
    // }
    // inline const QString& VIDS_DATABASE() {
    //   static const QString path = WORK_PATH() + "/VIDS_DATABASE.db";
    //   return path;
    // }

    // inline const QString& DEVICES_AND_DRIVES_DATABASE() {
    //   static const QString path = WORK_PATH() + "/DEVICES_AND_DRIVES.db";
    //   return path;
    // }

    // inline const QString& AI_MEDIA_DUP_DATABASE() {
    //   static const QString path = WORK_PATH() + "/DUPLICATES_DB.db";
    //   return path;
    // }

    // inline const QString& RECYCLE_BIN_DATABASE() {
    //   static const QString path = WORK_PATH() + "/RECYCLE_BIN_DATABASE.db";
    //   return path;
    // }

    // inline const QString& PEFORMERS_DATABASE() {
    //   static const QString path = WORK_PATH() + "/PERFORMERS_DATABASE.db";
    //   return path;
    // }

    // inline const QString& TORRENTS_DATABASE() {
    //   static const QString path = WORK_PATH() + "/TORRENTS_DATABASE.db";
    //   return path;
    // }

    // inline const QString& PRODUCTION_STUDIOS_DATABASE() {
    //   static const QString path = WORK_PATH() + "/PRODUCTION_STUDIOS_DATABASE.db";
    //   return path;
    // }
    // }

  }

  void ChooseCopyDestination_ok() {
    // precondition
    const QFileInfo currentFile{__FILE__};
    const QString fileParentFolder{currentFile.absolutePath()};
    QCOMPARE(QFileInfo{fileParentFolder}.isDir(), true);
    const QString fileParentParentFolder{QFileInfo(fileParentFolder).absolutePath()};
    QCOMPARE(QFileInfo{fileParentParentFolder}.isDir(), true);

    Configuration().setValue(MemoryKey::PATH_LAST_TIME_COPY_TO.name, fileParentParentFolder);

    // user specified
    QCOMPARE(ChooseCopyDestination(fileParentFolder, nullptr), fileParentFolder);
    QCOMPARE(Configuration().value(MemoryKey::PATH_LAST_TIME_COPY_TO.name).toString(), fileParentFolder);

    // from Configs file
    Configuration().setValue(MemoryKey::PATH_LAST_TIME_COPY_TO.name, fileParentParentFolder);
    QCOMPARE(ChooseCopyDestination("", nullptr), fileParentParentFolder);
    QCOMPARE(Configuration().value(MemoryKey::PATH_LAST_TIME_COPY_TO.name).toString(), fileParentParentFolder);
  }

  void load_and_remove_language_pack_ok() {
    // 0. use english by default
    QPushButton oldApplyButton(tr("Apply"));
    QPushButton oldCancelButton(tr("Cancel"));
    QCOMPARE(oldApplyButton.text(), "Apply");
    QCOMPARE(oldCancelButton.text(), "Cancel");

    QTranslator translator;
    QCOMPARE(LoadCNLanguagePack(translator, "inexist qm file path"), false);

    QString qmFileInqrc = ":/translations/test_zh_CN.qm";
    bool loaded = LoadCNLanguagePack(translator, qmFileInqrc);
    QVERIFY2(loaded, "Failed to load Chinese translation file");
    QCoreApplication::installTranslator(&translator);

    // 1. for old widget translate not works ok
    QCOMPARE(oldApplyButton.text(), "Apply");
    QCOMPARE(oldCancelButton.text(), "Cancel");

    // 2. new widget translate into chinese works ok
    QPushButton newApplyButton(tr("Apply"));
    QPushButton newCancelButton(tr("Cancel"));
    QCOMPARE(newApplyButton.text(), "应用");
    QCOMPARE(newCancelButton.text(), "取消");

    // 3. recover use english by default ok
    QCoreApplication::removeTranslator(&translator);
    QPushButton recoverApplyButton(tr("Apply"));
    QPushButton recoverCancelButton(tr("Cancel"));
    QCOMPARE(recoverApplyButton.text(), "Apply");
    QCOMPARE(recoverCancelButton.text(), "Cancel");
  }
};

#include "PublicToolTest.moc"
REGISTER_TEST(PublicToolTest, false)
