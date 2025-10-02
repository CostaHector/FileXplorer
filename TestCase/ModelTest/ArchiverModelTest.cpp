#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ArchiverModel.h"
#include "EndToExposePrivateMember.h"

class ArchiverModelTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void default_contructor_ok() {
    // 测试默认构造不崩溃
    ArchiverModel model;

    // 验证默认行数和列数
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.columnCount(), 3);

    // 验证无效索引返回空
    QModelIndex invalidIndex;
    QVERIFY(!model.data(invalidIndex).isValid());

    // 验证表头数据
    QCOMPARE(model.headerData(0, Qt::Horizontal).toString(), "Name");
    QCOMPARE(model.headerData(1, Qt::Horizontal).toString(), "Compressed(B)");
    QCOMPARE(model.headerData(2, Qt::Horizontal).toString(), "Original(B)");

    // 验证垂直表头对齐
    QCOMPARE(model.headerData(0, Qt::Vertical, Qt::TextAlignmentRole).toInt(), static_cast<int>(Qt::AlignRight));

    // 验证超出范围的列自动转数值
    QCOMPARE(model.headerData(3, Qt::Horizontal), 3 + 1);
    QCOMPARE(model.headerData(-1, Qt::Horizontal), -1 + 1);
    // 未设定角色
    QVERIFY(model.headerData(0, Qt::Horizontal, Qt::ItemDataRole::ForegroundRole).isNull());
  }

  void data_retrieve_ok() {
    ArchiverModel model;

    QList<ArchiveFilesReader::CompressedItem> mockItems{
        {"file1.txt", 2048, 1024, QByteArray("Content 1")},         //
        {"file2.jpg", 4096, 2048, QByteArray("Content 2")},         //
        {"folder/file3.pdf", 8192, 4096, QByteArray("Content 3")},  //
    };
    model.RowsCountBeginChange(0, 3);
    model.m_paf.mockCompressedItem(mockItems);
    model.RowsCountEndChange();

    // 验证行数
    QCOMPARE(model.rowCount(), mockItems.size());

    // 验证列数
    QCOMPARE(model.columnCount(), 3);

    QStringList originSizeExpects{
        "0'0'2'0",  //
        "0'0'4'0",  //
        "0'0'8'0",  //
    };
    QStringList compressedSizeExpects{
        "0'0'1'0",  //
        "0'0'2'0",  //
        "0'0'4'0",  //
    };

    // 验证数据获取
    for (int row = 0; row < mockItems.size(); ++row) {
      const auto& item = mockItems[row];

      // 名称列
      QModelIndex nameIndex = model.index(row, 0);
      QCOMPARE(model.data(nameIndex).toString(), item.m_names);

      // 原始大小列
      QModelIndex originalIndex = model.index(row, 1);
      QString originalSize = model.data(originalIndex).toString();
      QCOMPARE(originalSize, originSizeExpects[row]);

      // 压缩大小列
      QModelIndex compressedIndex = model.index(row, 2);
      QString compressedSize = model.data(compressedIndex).toString();
      QCOMPARE(compressedSize, compressedSizeExpects[row]);

      QCOMPARE(model.GetRelativeName(row), item.m_names);
      QCOMPARE(model.GetByteArrayData(row), item.m_datas);
    }

    // 验证无效行返回空
    QCOMPARE(model.GetRelativeName(-1), "");
    QCOMPARE(model.GetRelativeName(100), "");
    QVERIFY(model.GetByteArrayData(-1).isEmpty());
    QVERIFY(model.GetByteArrayData(100).isEmpty());

    // 验证无效角色返回空
    QModelIndex index = model.index(0, 0);
    QVERIFY(!model.data(index, Qt::UserRole).isValid());

    // 验证无效列返回空
    QModelIndex invalidColIndex = model.index(0, 3);
    QVERIFY(!model.data(invalidColIndex).isValid());
  }

  void setRootPath_ok() {
    ArchiverModel model;

    // 设置根路径
    model.setRootPath("inexist qz file");

    // 验证模型重置后行数为0（因为文件无效）
    QCOMPARE(model.rowCount(), 0);

    // 再次设置根路径
    model.setRootPath("invalid_path");
    QCOMPARE(model.rowCount(), 0);
  }
};

#include "ArchiverModelTest.moc"
REGISTER_TEST(ArchiverModelTest, false)
