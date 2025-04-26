#include <QCoreApplication>
#include <QtTest>
#include "TestCase/pub/MyTestSuite.h"
#include "Tools/FileDescriptor/FileDescriptor.h"
#include "public/PublicTool.h"

const QString rootpath = QFileInfo(__FILE__).absolutePath() + "/FileDescriptor";
const QString fi5Char = rootpath + "/5CharFile.txt";
const QString fi10Char = rootpath + "/10CharFile.txt";
const QString fiTemp = rootpath + "/Temp.txt";
class FileDescriptorTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() {
    QVERIFY(QFile::exists(fi5Char));
    QVERIFY(QFile::exists(fi10Char));
    QVERIFY(!QFile::exists(fiTemp));
  }

  void test_fd_file_not_exist() {
    FileDescriptor fDescriptor;
    const auto fd_1 = fDescriptor.GetFileUniquedId(fiTemp);
    QCOMPARE(fd_1, -1);
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
    QVERIFY(QFile::rename(fi5Char, fiTemp));  // rename file name
    const auto fdTemp = fDescriptor.GetFileUniquedId(fiTemp);
    QVERIFY(QFile::rename(fiTemp, fi5Char));  // recover

    QVERIFY(fd5 > 0);
    QVERIFY(fdTemp > 0);
    QVERIFY(fd5 == fdTemp);
  }
  void test_fd_unchange_after_content_modified() {
    FileDescriptor fDescriptor;
    const auto fd5 = fDescriptor.GetFileUniquedId(fi5Char);
    QVERIFY(TextWriter(fi5Char, "ABCDE123", QIODevice::WriteOnly | QIODevice::Text));  // modify content
    const auto fdContentModified = fDescriptor.GetFileUniquedId(fi5Char);

    QVERIFY(TextWriter(fi5Char, "ABCDE", QIODevice::WriteOnly | QIODevice::Text));  // recover
    QVERIFY(fd5 > 0);
    QVERIFY(fdContentModified > 0);
    QVERIFY(fd5 == fdContentModified);
  }
  void test_fd_change_after_remove_and_renew() {
    FileDescriptor fDescriptor;
    const auto fd5 = fDescriptor.GetFileUniquedId(fi5Char);
    QVERIFY(QFile::remove(fi5Char));  // remove should ok
    QVERIFY(!QFile::exists(fi5Char));
    QVERIFY(TextWriter(fi5Char, "ABCDE", QIODevice::WriteOnly | QIODevice::Text));  // renew should ok
    const auto fdRenew = fDescriptor.GetFileUniquedId(fi5Char);
    QVERIFY(fd5 > 0);
    QVERIFY(fdRenew > 0);
    QVERIFY(fd5 != fdRenew);
  }
};

FileDescriptorTest g_FileDescriptorTest;
#include "FileDescriptorTest.moc"
