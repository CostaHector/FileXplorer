#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ArchiveFiles.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"
#include "UndoRedo.h"

class ArchiveFilesTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
 private slots:
  void initTestCase() { QVERIFY(tDir.IsValid()); }

  void isQzFile_ok() {
    QVERIFY(ArchiveFilesReader::isQZFile("a.qz"));
    QVERIFY(!ArchiveFilesReader::isQZFile("a.txt"));
    QVERIFY(!ArchiveFilesReader::isQZFile("a.mp4"));
    QVERIFY(!ArchiveFilesReader::isQZFile("a.jpg"));

    QVERIFY(ArchiveFilesReader::isQZFile(QFileInfo("a.qz")));
    QVERIFY(!ArchiveFilesReader::isQZFile(QFileInfo("a.txt")));
    QVERIFY(!ArchiveFilesReader::isQZFile(QFileInfo("a.mp4")));
    QVERIFY(!ArchiveFilesReader::isQZFile(QFileInfo("a.jpg")));
  }

  void reader_defaultConstruct_not_crash_down() {
    ArchiveFilesReader reader;
    QVERIFY(reader.isEmpty());
    QCOMPARE(reader.size(), 0);
    reader.clear();
  }

  void reader_empty_qzfile_not_qz_file_ok() {
    const QString notAQZFilePath = __FILE__;
    QVERIFY(!notAQZFilePath.endsWith(".qz", Qt::CaseInsensitive));

    const QString inexistQzFilePath = "inexist.qz";

    ArchiveFilesReader reader;
    QVERIFY(!reader.ReadAchiveFile(notAQZFilePath));
    QVERIFY(!reader.ReadAchiveFile(inexistQzFilePath));
    QVERIFY(reader.isEmpty());
  }

  void reader_validQzfile_ok_readAnotherPath_DecompressToPath_ok() {
    tDir.ClearAll();
    // 创建第一个测试环境
    QList<FsNodeEntry> entries1 = {{"file1.txt", false, "This is file 1 content"},
                                   {"folder1/file2.txt", false, "File 2 content"},
                                   {"folder1/subfolder/file3.bin", false, QByteArray("\x00\x01\x02\x03", 4)}};
    int createResult1 = tDir.createEntries(entries1);
    QCOMPARE(createResult1, entries1.size());

    // 创建第一个压缩文件
    QStringList filesToCompress1 = {"file1.txt", "folder1"};
    QString archivePath1 = tDir.itemPath("test1.qz");

    ArchiveFilesWriter writer;
    QCOMPARE(writer.CompressNow(tDir.path(), filesToCompress1, archivePath1, false), 3);  // 3 file get compressed

    // 创建第二个测试环境（不同的内容）
    QList<FsNodeEntry> entries2 = {{"doc.pdf", false, "PDF content"},         //
                                   {"images/photo.jpg", false, "JPEG data"},  //
                                   {"config.ini", false, "[settings]\nkey=value"}};
    int createResult2 = tDir.createEntries(entries2);
    QCOMPARE(createResult2, entries2.size());

    // 创建第二个压缩文件
    QStringList filesToCompress2 = {"doc.pdf", "images", "config.ini"};
    QString archivePath2 = tDir.itemPath("test2.qz");

    QCOMPARE(writer.CompressNow(tDir.path(), filesToCompress2, archivePath2, false), 3);  // 3 file get compressed

    // 读取第一个压缩文件
    ArchiveFilesReader reader;
    QVERIFY(reader.ReadAchiveFile(archivePath1));

    // 验证读取的内容
    QCOMPARE(reader.size(), 3);
    QCOMPARE(reader.key(0), "file1.txt");
    QCOMPARE(reader.beforeSize(0), entries1[0].contents.size());
    QCOMPARE(reader.value(0), entries1[0].contents);

    QCOMPARE(reader.key(1), "folder1/file2.txt");
    QCOMPARE(reader.value(1), entries1[1].contents);

    QCOMPARE(reader.key(2), "folder1/subfolder/file3.bin");
    QCOMPARE(reader.value(2), entries1[2].contents);

    // 切换到第二个压缩文件
    QVERIFY(reader.ReadAchiveFile(archivePath2));

    // 验证读取的内容
    QCOMPARE(reader.size(), 3);
    QCOMPARE(reader.key(0), "doc.pdf");
    QCOMPARE(reader.beforeSize(0), entries2[0].contents.size());
    QCOMPARE(reader.value(0), entries2[0].contents);

    QCOMPARE(reader.key(1), "images/photo.jpg");
    QCOMPARE(reader.value(1), entries2[1].contents);

    QCOMPARE(reader.key(2), "config.ini");
    QCOMPARE(reader.value(2), entries2[2].contents);

    // 解压第二个压缩文件到新路径
    QString decompressPath = tDir.itemPath("decompressed");
    QVERIFY(tDir.mkpath("decompressed"));
    QVERIFY(reader.DecompressToPath(decompressPath));

    // 验证解压后的文件
    // 验证文件结构
    QSet<QString> expectedFilesUnderDecompressed = {"doc.pdf", "images/photo.jpg", "config.ini"};
    QSet<QString> actualFilesUnderDecompressed = tDir.SnapshotAtPath(decompressPath, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
    QCOMPARE(actualFilesUnderDecompressed, expectedFilesUnderDecompressed);

    // 检查文件内容
    bool isValid = false;
    QCOMPARE(tDir.readByteArray("doc.pdf", &isValid), entries2[0].contents);
    QVERIFY(isValid);

    QCOMPARE(tDir.readByteArray("images/photo.jpg", &isValid), entries2[1].contents);
    QVERIFY(isValid);

    QCOMPARE(tDir.readByteArray("config.ini", &isValid), entries2[2].contents);
    QVERIFY(isValid);
  }

  void writer_defaultConstruct_not_crash_down() {
    ArchiveFilesWriter writer;
    QCOMPARE(writer.m_ds.device(), nullptr);
  }

  void writer_inexists_selection_inexist_destination_ok() {
    tDir.ClearAll();
    ArchiveFilesWriter writer;

    // 空输入
    QCOMPARE(writer.CompressNow(tDir.path(), {}, tDir.path(), false), 0);

    // 源路径不存在
    QCOMPARE(writer.CompressNow("inexist sre path", {__FILE__}, tDir.path(), false), -1);

    // 目的路径无法打开(文件夹)
    QCOMPARE(writer.CompressNow(tDir.path(), {__FILE__}, tDir.path(), false), -1);

    // 源文件不存在
    QString archivePath = tDir.itemPath("test.qz");
    QCOMPARE(writer.CompressNow(tDir.path(), {"nonexistent.txt"}, archivePath, false), 0);
  }

  void writer_2file_then_1file_override_append_ok() {
    tDir.ClearAll();
    // 创建测试文件
    QList<FsNodeEntry> nodes{
        {"file1.txt", false, "Content 1"},
        {"file2.txt", false, "Content 2"},
        {"file3.txt", false, "Content 3"},
        {"file4.mp4", false, "Content 4"},  // vid skipped
    };
    QCOMPARE(tDir.createEntries(nodes), 4);

    const QString archivePath = tDir.itemPath("test.qz");

    // 第一次压缩两个文件
    ArchiveFilesWriter writer;
    QCOMPARE(writer.CompressNow(tDir.path(), {"file1.txt", "file2.txt"}, archivePath, false), 2);

    // 读取验证
    ArchiveFilesReader reader;
    QVERIFY(reader.ReadAchiveFile(archivePath));
    QCOMPARE(reader.size(), 2);

    // 1.0 覆盖模式压缩一个新文件, 1 ignored video
    QCOMPARE(writer.CompressNow(tDir.path(), {"file3.txt", "file4.mp4"}, archivePath, false), 1);

    // 验证覆盖
    QVERIFY(reader.ReadAchiveFile(archivePath));
    QCOMPARE(reader.size(), 1);
    QCOMPARE(reader.key(0), "file3.txt");

    // 2.0 追加模式压缩两个新文件, 1 ignored video
    QCOMPARE(writer.CompressNow(tDir.path(), {"file1.txt", "file2.txt", "file4.mp4"}, archivePath, true), 2);

    // 验证追加
    QVERIFY(reader.ReadAchiveFile(archivePath));
    QCOMPARE(reader.size(), 3);

    QStringList expectedFiles = {"file3.txt", "file1.txt", "file2.txt"};
    for (int i = 0; i < reader.size(); i++) {
      QCOMPARE(reader.key(i), expectedFiles[i]);
    }
  }

  void writer_mixedFileFolders_override_ok() {
    // 创建测试环境
    QList<FsNodeEntry> entries = {{"file1.txt", false, "File 1"},
                                  {"folder1/file2.txt", false, "File 2"},
                                  {"folder1/subfolder/file3.txt", false, "File 3"},
                                  {"folder2/file4.bin", false, QByteArray("\x01\x02\x03", 3)}};
    int createResult = tDir.createEntries(entries);
    QCOMPARE(createResult, entries.size());

    // 压缩混合内容
    QString archivePath = tDir.itemPath("mixed.qz");
    ArchiveFilesWriter writer;
    QCOMPARE(writer.CompressNow(tDir.path(), {"file1.txt", "folder1", "folder2"}, archivePath, false), 4);

    // 验证压缩内容
    ArchiveFilesReader reader;
    QVERIFY(reader.ReadAchiveFile(archivePath));
    QCOMPARE(reader.size(), 4);

    // 验证文件路径和内容
    for (int i = 0; i < reader.size(); i++) {
      const auto& entry = entries[i];
      QCOMPARE(reader.key(i), entry.relativePathToNode);
      QCOMPARE(reader.value(i), entry.contents);
    }
  }

  void writer_reader_bidirectional_conversion_ok() {
    tDir.ClearAll();
    // 创建复杂的测试环境
    QList<FsNodeEntry> entries = {
        {"doc.txt", false, "Document content"},
        {"images/photo1.jpg", false, "JPEG data 1"},
        {"images/photo2.jpg", false, "JPEG data 2"},
        {"data/config.ini", false, "[settings]\nkey=value"},
        {"empty_dir", true, QByteArray()}  // empty folder will be ignored when compressed
    };
    int createResult = tDir.createEntries(entries);
    QCOMPARE(createResult, entries.size());

    // 存储当前文件结构快照, 不包含空文件夹
    QSet<QString> originalSnapshot = tDir.Snapshot();
    originalSnapshot.remove("empty_dir");

    // 压缩
    QString archivePath = tDir.itemPath("archive.qz");
    ArchiveFilesWriter writer;
    QCOMPARE(writer.CompressNow(tDir.path(), {"doc.txt", "images", "data", "empty_dir"}, archivePath, false), 4);  // 1txt + 2jpg + 1ini

    // 解压到新位置
    QVERIFY(tDir.mkpath("decompressed"));
    QString decompressPath = tDir.itemPath("decompressed");

    ArchiveFilesReader reader;
    QVERIFY(reader.ReadAchiveFile(archivePath));
    QVERIFY(reader.DecompressToPath(decompressPath));

    // 验证文件内容
    for (const auto& entry : entries) {
      const QString absPath2fileUnderDecompressed = tDir.itemPath("decompressed/" + entry.relativePathToNode);
      if (entry.isDir) {
        QVERIFY(!tDir.exists("decompressed/" + entry.relativePathToNode));
        continue;
      }
      bool isValid = false;
      QByteArray content = tDir.readByteArrayAtPath(absPath2fileUnderDecompressed, &isValid);
      QVERIFY(isValid);
      QCOMPARE(content, entry.contents);
    }

    // 验证解压后的文件结构快照
    QSet<QString> decompressedSnapshot = tDir.SnapshotAtPath(decompressPath);
    QCOMPARE(decompressedSnapshot, originalSnapshot);
  }

  void archiveImagesRecusive_autorecycle_ok() {
    QVERIFY(tDir.ClearAll());
    QList<FsNodeEntry> entries = {
        {"baseFolderName/file1.jpg", false, "File 1"},
        {"baseFolderName/file2.jpg", false, "File 2"},
        {"baseFolderName/subfolder/file3.jpg", false, "File 3"},
        {"baseFolderName/subfolder/file4.jpg", false, "File 4"},
    };
    int createResult = tDir.createEntries(entries);
    QCOMPARE(createResult, 4);

    ArchiveImagesRecusive achiveRecursive(true);

    ArchiveImagesRecusive::setMaxCompressImageCnt(2);                                  // count(image under a path)>=2, 不允许压缩
    {                                                                                  //
      QCOMPARE(achiveRecursive.CompressImgRecur(tDir.itemPath("baseFolderName")), 0);  // 0个qz
    }

    ArchiveImagesRecusive::setMaxCompressImageCnt(3);  // 压缩成功, 两个qz文件, 一个是baseFolderName.qz, 一个是subfolder.qz
    {
      QCOMPARE(achiveRecursive.CompressImgRecur(tDir.itemPath("baseFolderName")), 2);  // 2个qz文件, 预期图片被回收

      QSet<QString> expectSnapshot2QzOnly{
          "baseFolderName/baseFolderName.qz",
          "baseFolderName/subfolder/subfolder.qz",
      };
      auto actualAns = tDir.Snapshot(QDir::Filter::Files);
      QCOMPARE(actualAns, expectSnapshot2QzOnly);
    }

    // 撤销2次
    UndoRedo::GetInst().on_Undo();
    UndoRedo::GetInst().on_Undo();

    // 现在预期图片和qz都并存
    QSet<QString> expectSnapshot{
        "baseFolderName/file1.jpg",               //
        "baseFolderName/file2.jpg",               //
        "baseFolderName/baseFolderName.qz",       //
        "baseFolderName/subfolder/file3.jpg",     //
        "baseFolderName/subfolder/file4.jpg",     //
        "baseFolderName/subfolder/subfolder.qz",  //
    };
    QCOMPARE(tDir.Snapshot(QDir::Filter::Files), expectSnapshot);
  }

  void archiveImagesRecusive_not_autorecycle_ok() {
    QVERIFY(tDir.ClearAll());
    QList<FsNodeEntry> entries = {
        {"baseFolderName/file1.jpg", false, "File 1"},
        {"baseFolderName/file2.jpg", false, "File 2"},
        {"baseFolderName/subfolder/file3.jpg", false, "File 3"},
        {"baseFolderName/subfolder/file4.jpg", false, "File 4"},
    };
    int createResult = tDir.createEntries(entries);
    QCOMPARE(createResult, 4);

    ArchiveImagesRecusive achiveRecursive(false);      // 图片不回收
    ArchiveImagesRecusive::setMaxCompressImageCnt(3);  // 压缩成功, 两个qz文件, 一个是baseFolderName.qz, 一个是subfolder.qz
    {
      QCOMPARE(achiveRecursive.CompressImgRecur(tDir.itemPath("baseFolderName")), 2);  // 2个qz文件
    }

    QSet<QString> expectSnapshot{
        "baseFolderName/file1.jpg",               //
        "baseFolderName/file2.jpg",               //
        "baseFolderName/baseFolderName.qz",       //
        "baseFolderName/subfolder/file3.jpg",     //
        "baseFolderName/subfolder/file4.jpg",     //
        "baseFolderName/subfolder/subfolder.qz",  //
    };
    QCOMPARE(tDir.Snapshot(QDir::Filter::Files), expectSnapshot);
  }
};

#include "ArchiveFilesTest.moc"
REGISTER_TEST(ArchiveFilesTest, false)
