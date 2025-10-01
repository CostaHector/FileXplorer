#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "HarFiles.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

class HarFilesTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir tDir;
 private slots:
  void initTestCase() { QVERIFY(tDir.IsValid()); }

  void test_ValidHarFileParsing();
  void test_UnsupportedMimeTypes();
  void test_MissingRequiredFields();
  void test_InvalidJson();
  void test_EmptyHarFile();
  void test_GetPathStem();
  void test_EncodingTypes();
};

void HarFilesTest::test_ValidHarFileParsing() {
  // 创建有效的.har文件, "aGVsbG8=", "hello" in base64
  QString harContent = R"({
      "log": {
        "entries": [
          {
            "request": { "url": "https://example.com/image1.jpeg" },
            "response": {
              "content": {
                "mimeType": "image/jpeg",
                "text": "aGVsbG8=",
                "encoding": "base64"
              }
            }
          },
          {
            "request": { "url": "https://example.com/image2.png" },
            "response": {
              "content": {
                "mimeType": "image/png",
                "text": "plain text content",
                "encoding": "literal"
              }
            }
          }
        ]
      }
    })";

  // 使用tDir.touch创建.har文件
  QString harFilePath = tDir.itemPath("valid.har");
  QVERIFY(tDir.touch("valid.har", harContent.toUtf8()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(harFiles(harFilePath));

  // 验证解析结果
  QCOMPARE(harFiles.size(), 2);
  QVERIFY(!harFiles.empty());

  // 验证第一个条目
  const HAR_FILE_ITEM& item1 = harFiles[0];
  QCOMPARE(item1.name, "image1.jpg");
  QCOMPARE(item1.content, QByteArray("hello"));
  QCOMPARE(item1.type, ".jpg");
  QCOMPARE(item1.url, "https://example.com/image1.jpeg");

  // 验证第二个条目
  const HAR_FILE_ITEM& item2 = harFiles[1];
  QCOMPARE(item2.name, "image2.png");
  QCOMPARE(item2.content, QByteArray("plain text content"));
  QCOMPARE(item2.type, ".png");
  QCOMPARE(item2.url, "https://example.com/image2.png");
}

void HarFilesTest::test_UnsupportedMimeTypes() {
  // 创建包含不支持MIME类型的.har文件
  QString harContent = R"({
      "log": {
        "entries": [
          {
            "request": { "url": "https://example.com/image1.html" },
            "response": {
              "content": {
                "mimeType": "text/html",
                "text": "<html>content</html>"
              }
            }
          },
          {
            "request": { "url": "https://example.com/image2.css" },
            "response": {
              "content": {
                "mimeType": "text/css",
                "text": "body {color: red;}"
              }
            }
          }
        ]
      }
    })";

  // 使用tDir.touch创建.har文件
  QString harFilePath = tDir.itemPath("unsupported.har");
  QVERIFY(tDir.touch("unsupported.har", harContent.toUtf8()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(harFiles(harFilePath));

  // 验证没有解析任何条目
  QCOMPARE(harFiles.size(), 0);
  QVERIFY(harFiles.empty());
}

void HarFilesTest::test_MissingRequiredFields() {
  // 创建缺少必需字段的.har文件,  // 缺少content字段  // 缺少url字段  // 缺少request字段
  QString harContent = R"({
      "log": {
        "entries": [
          {
            "request": { "url": "https://example.com/image1.jpeg" },
            "response": {}
          },
          {
            "request": {},
            "response": {
              "content": {
                "mimeType": "image/png",
                "text": "content"
              }
            }
          },
          {
            "response": {
              "content": {
                "mimeType": "image/jpeg",
                "text": "content"
              }
            }
          }
        ]
      }
    })";

  // 使用tDir.touch创建.har文件
  QString harFilePath = tDir.itemPath("missing_fields.har");
  QVERIFY(tDir.touch("missing_fields.har", harContent.toUtf8()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(!harFiles(harFilePath));  // 应返回false
  QVERIFY(harFiles.empty());
}

void HarFilesTest::test_InvalidJson() {
  // 创建无效JSON的.har文件
  QString harContent = "This is not valid JSON content";

  // 使用tDir.touch创建.har文件
  QString harFilePath = tDir.itemPath("invalid_json.har");
  QVERIFY(tDir.touch("invalid_json.har", harContent.toUtf8()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(!harFiles(harFilePath));
  QVERIFY(harFiles.empty());
}

void HarFilesTest::test_EmptyHarFile() {
  // 创建空.har文件
  QString harFilePath = tDir.itemPath("empty.har");
  QVERIFY(tDir.touch("empty.har", QByteArray()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(!harFiles(harFilePath));
  QVERIFY(harFiles.empty());
}

void HarFilesTest::test_GetPathStem() {
  // 测试各种URL格式
  QCOMPARE(GetPathStem("https://example.com/image.jpg"), "image");
  QCOMPARE(GetPathStem("https://example.com/path/to/image.png"), "image");
  QCOMPARE(GetPathStem("https://example.com/image.with.dots.jpeg"), "image.with.dots");
  QCOMPARE(GetPathStem("https://example.com/no_extension"), "no_extension");
  QCOMPARE(GetPathStem("https://example.com/long.extension"), "long.extension");  // 扩展名长度>5
  QCOMPARE(GetPathStem("relative/path/file.txt"), "file");
  QCOMPARE(GetPathStem("filename-only"), "filename-only");
}

void HarFilesTest::test_EncodingTypes() {
  // 测试不同的编码类型
  QString harContent = R"({
    "log": {
      "entries": [
        {
          "request": { "url": "https://example.com/image1.jpeg" },
          "response": {
            "content": {
              "mimeType": "image/jpeg",
              "text": "aGVsbG8=",
              "encoding": "base64"
            }
          }
        },
        {
          "request": { "url": "https://example.com/image2.png" },
          "response": {
            "content": {
              "mimeType": "image/png",
              "text": "plain text content",
              "encoding": "literal"
            }
          }
        },
        {
          "request": { "url": "https://example.com/image3.gif" },
          "response": {
            "content": {
              "mimeType": "image/gif",
              "text": "aGVsbG8="
            }
          }
        }
      ]
    }
  })";

  // 使用tDir.touch创建.har文件
  QString harFilePath = tDir.itemPath("encoding.har");
  QVERIFY(tDir.touch("encoding.har", harContent.toUtf8()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(harFiles(harFilePath));

  // 验证解析结果
  QCOMPARE(harFiles.size(), 3);

  // 验证base64编码
  QCOMPARE(harFiles[0].content, QByteArray("hello"));

  // 验证literal编码
  QCOMPARE(harFiles[1].content, QByteArray("plain text content"));

  // 验证默认编码（应为literal）
  QCOMPARE(harFiles[2].content, QByteArray("aGVsbG8="));  // 因为没有指定encoding，所以不进行base64解码
}

#include "HarFilesTest.moc"
REGISTER_TEST(HarFilesTest, false)
