#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "TDir.h"
#include "FileDescriptor.h"
#include "PublicTool.h"
#include "Logger.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class FileDescriptorTest : public PlainTestSuite {
  Q_OBJECT
 public:
  FileDescriptorTest() : PlainTestSuite{} {}
  TDir mDir;
  const QString mWorkPath{mDir.path()};
  const QString fi5Char{mWorkPath + "/5CharFile.txt"};
  const QString fi10Char{mWorkPath + "/10CharFile.txt"};
  const QString fiFileTemp{mWorkPath + "/Temp.txt"};
  const QString fiPathTemp{mWorkPath + "/NotExistPath/Temp.txt"};
 private slots:
  void initTestCase() {
    QVERIFY(mDir.IsValid());
    const QList<FsNodeEntry> gNode{
        FsNodeEntry{"5CharFile.txt", false, "ABCDE"},
        FsNodeEntry{"10CharFile.txt", false, ""},
    };
    QCOMPARE(mDir.createEntries(gNode), 2);
  }

#ifdef Q_OS_WIN
  void test_fd_file_not_exist() {
    FileDescriptor fDescriptor;
    const qint64 fd_1 = fDescriptor.GetFileUniquedId(fiFileTemp);
    QCOMPARE(fd_1, -(qint64)ERROR_FILE_NOT_FOUND);
  }

  void test_fd_path_not_exist() {
    FileDescriptor fDescriptor;
    const qint64 fd_1 = fDescriptor.GetFileUniquedId(fiPathTemp);
    QCOMPARE(fd_1, -(qint64)ERROR_PATH_NOT_FOUND);
  }

  void test_fd_get_ok() {
    FileDescriptor fDescriptor;
    const auto fd5 = fDescriptor.GetFileUniquedId(fi5Char);
    const auto fd10 = fDescriptor.GetFileUniquedId(fi10Char);
    QVERIFY(fd5 > 0);
    QVERIFY(fd10 > 0);
    QVERIFY(fd5 != fd10);

    const auto& fds = fDescriptor.GetFileUniquedIds({fi5Char, fi10Char});
    const QList<qint64> expectsLst{fd5, fd10};
    QCOMPARE(fds, expectsLst);
  }
  void test_fd_unchange_after_name_renamed() {
    FileDescriptor fDescriptor;
    const auto fd5 = fDescriptor.GetFileUniquedId(fi5Char);
    QVERIFY(QFile::rename(fi5Char, fiFileTemp));  // rename file name
    const auto fdTemp = fDescriptor.GetFileUniquedId(fiFileTemp);
    QVERIFY(QFile::rename(fiFileTemp, fi5Char));  // recover

    QVERIFY(fd5 > 0);
    QVERIFY(fdTemp > 0);
    QVERIFY(fd5 == fdTemp);
  }
  void test_fd_unchange_after_content_modified() {
    FileDescriptor fDescriptor;
    const auto fd5 = fDescriptor.GetFileUniquedId(fi5Char);
    QVERIFY(FileTool::TextWriter(fi5Char, "ABCDE123", QIODevice::WriteOnly | QIODevice::Text));  // modify content
    const auto fdContentModified = fDescriptor.GetFileUniquedId(fi5Char);

    QVERIFY(FileTool::TextWriter(fi5Char, "ABCDE", QIODevice::WriteOnly | QIODevice::Text));  // recover
    QVERIFY(fd5 > 0);
    QVERIFY(fdContentModified > 0);
    QVERIFY(fd5 == fdContentModified);
  }
  void test_fd_change_after_remove_and_renew() {
    FileDescriptor fDescriptor;
    const auto fd5 = fDescriptor.GetFileUniquedId(fi5Char);
    QVERIFY(QFile::remove(fi5Char));  // remove should ok
    QVERIFY(!QFile::exists(fi5Char));
    QVERIFY(FileTool::TextWriter(fi5Char, "ABCDE", QIODevice::WriteOnly | QIODevice::Text));  // renew should ok
    const auto fdRenew = fDescriptor.GetFileUniquedId(fi5Char);
    QVERIFY(fd5 > 0);
    QVERIFY(fdRenew > 0);
    QVERIFY(fd5 != fdRenew);
  }
#else
  void test_fd_not_in_windows() {
    QCOMPARE(1, 1);
    LOG_W("Not in windows, Fd will not support");
  }
#endif
};

#include "FileDescriptorTest.moc"
REGISTER_TEST(FileDescriptorTest, false)
