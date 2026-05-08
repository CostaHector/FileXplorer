#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "HarFiles.h"
#include "EndToExposePrivateMember.h"
#include "TDir.h"

#include "ImageTool.h"
#include "ByteArrayImageViewer.h"

#include <QMovie>
#include <QPixmap>

class HarFilesTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
private slots:
  void initTestCase() { //
    QVERIFY(mTDir.IsValid());
  }

  void test_ValidHarFileParsing();
  void gif_jpeg_ParseFilesInJsonByteArray_SaveToLocal_ok();
  void test_UnsupportedMimeTypes();
  void test_MissingRequiredFields();
  void test_InvalidJson();
  void test_EmptyHarFile();
  void test_GetPathItem();
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
  QString harFilePath = mTDir.itemPath("valid.har");
  QVERIFY(mTDir.touch("valid.har", harContent.toUtf8()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(harFiles(harFilePath));

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

void HarFilesTest::gif_jpeg_ParseFilesInJsonByteArray_SaveToLocal_ok() {
  const QByteArray gif_jpeg_har{
      R"({
      "log": {
        "entries": [
          {
            "request": { "url": "https://example.com/settings.gif" },
            "response": {
              "content": {
                "mimeType": "image/gif",
                "text": "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",
                "encoding": "base64"
              }
            }
          },
          {
            "request": { "url": "https://example.com/unlocked_person.jpg" },
            "response": {
              "content": {
                "mimeType": "image/jpeg",
                "text": "/9j/4AAQSkZJRgABAQEAkACQAAD//gA7Q1JFQVRPUjogZ2QtanBlZyB2MS4wICh1c2luZyBJSkcgSlBFRyB2NjIpLCBxdWFsaXR5ID0gOTAK/9sAQwADAgIDAgIDAwMDBAMDBAUIBQUEBAUKBwcGCAwKDAwLCgsLDQ4SEA0OEQ4LCxAWEBETFBUVFQwPFxgWFBgSFBUU/9sAQwEDBAQFBAUJBQUJFA0LDRQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQUFBQU/8AAEQgAIAAgAwEiAAIRAQMRAf/EAB8AAAEFAQEBAQEBAAAAAAAAAAABAgMEBQYHCAkKC//EALUQAAIBAwMCBAMFBQQEAAABfQECAwAEEQUSITFBBhNRYQcicRQygZGhCCNCscEVUtHwJDNicoIJChYXGBkaJSYnKCkqNDU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6g4SFhoeIiYqSk5SVlpeYmZqio6Slpqeoqaqys7S1tre4ubrCw8TFxsfIycrS09TV1tfY2drh4uPk5ebn6Onq8fLz9PX29/j5+v/EAB8BAAMBAQEBAQEBAQEAAAAAAAABAgMEBQYHCAkKC//EALURAAIBAgQEAwQHBQQEAAECdwABAgMRBAUhMQYSQVEHYXETIjKBCBRCkaGxwQkjM1LwFWJy0QoWJDThJfEXGBkaJicoKSo1Njc4OTpDREVGR0hJSlNUVVZXWFlaY2RlZmdoaWpzdHV2d3h5eoKDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uLj5OXm5+jp6vLz9PX29/j5+v/aAAwDAQACEQMRAD8A+uKKK9b+Hvgmyj0aHUL23jurm4G9RKoZUXtgHjJ6596APJKK9b+IXgmyl0ebULO2S2ubcb2EShVde+QOMjrn2rySgAr2n4b+I4NV0OCyLhby0TY0Z6lRwGHtjA+tec+BPDcfiXWxDNn7NChklCnBYdAue2SfyBr2uw0qz0uPZZ20VuuMfu0Az9T3oA5r4keIrfStDubPeGvLtNixDkhTwWPoMZx7/jXi1fR1/pNnqsQju7WK5TGAJEBx9D2/CvFPHnhqPw1rflQZ+zSp5sYJyVGSCufbH6igD//Z",
                "encoding": "base64"
              }
            }
          }
        ]
      }
    })"};

  // ParseFilesInJsonByteArray ok
  HarFiles harFiles;
  QCOMPARE(harFiles.size(), 0);
  QCOMPARE(harFiles.empty(), true);

  QVERIFY(harFiles.ParseFilesInJsonByteArray(gif_jpeg_har));

  QCOMPARE(harFiles.size(), 2);
  QCOMPARE(harFiles.empty(), false);

  QSize gifBADim, jpgBADim;
  {
    QByteArray gifRawContent = harFiles[0].content;
    std::unique_ptr<QMovie> upMovie{ByteArrayImageViewer::GetMovieCoreStatic(gifRawContent, "gif", gifBADim)};
    QVERIFY(upMovie != nullptr);
    QVERIFY(gifBADim.width() > 0);
    QVERIFY(gifBADim.height() > 0);

    const QByteArray& jpgRawContent = harFiles[1].content;
    QPixmap jpgPmFromBA{ByteArrayImageViewer::GetPixmapCoreStatic(jpgRawContent, "jpg")};
    QVERIFY(!jpgPmFromBA.isNull());
    jpgBADim = jpgPmFromBA.size();
    QVERIFY(jpgBADim.width() > 0);
    QVERIFY(jpgBADim.height() > 0);
  }

  // SaveToLocal ok
  QCOMPARE(harFiles.SaveToLocal(mTDir.path(), {}), 0);                  // no row selected
  QCOMPARE(harFiles.SaveToLocal("path/to/inexist folder", {0, 1}), -1); // export to path not exist
  QCOMPARE(harFiles.SaveToLocal(mTDir.path(), {0, 1}), 2);              // all selected
  QVERIFY(mTDir.exists("settings.gif"));
  QVERIFY(mTDir.exists("unlocked_person.jpg"));

  {
    QMovie gifMovieFromFile{mTDir.itemPath("settings.gif"), "gif"};
    QVERIFY(gifMovieFromFile.isValid());
    QCOMPARE(ImageTool::GetImageDimensionPixel(mTDir.itemPath("settings.gif")), gifBADim);

    QPixmap jpgPmFromFile{mTDir.itemPath("unlocked_person.jpg"), "jpg"};
    QVERIFY(!jpgPmFromFile.isNull());
    QCOMPARE(jpgPmFromFile.size(), jpgBADim);
  }
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
  QString harFilePath = mTDir.itemPath("unsupported.har");
  QVERIFY(mTDir.touch("unsupported.har", harContent.toUtf8()));

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
  QString harFilePath = mTDir.itemPath("missing_fields.har");
  QVERIFY(mTDir.touch("missing_fields.har", harContent.toUtf8()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(!harFiles(harFilePath)); // 应返回false
  QVERIFY(harFiles.empty());
}

void HarFilesTest::test_InvalidJson() {
  // 创建无效JSON的.har文件
  QString harContent = "This is not valid JSON content";

  // 使用tDir.touch创建.har文件
  QString harFilePath = mTDir.itemPath("invalid_json.har");
  QVERIFY(mTDir.touch("invalid_json.har", harContent.toUtf8()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(!harFiles(harFilePath));
  QVERIFY(harFiles.empty());
}

void HarFilesTest::test_EmptyHarFile() {
  // 创建空.har文件
  QString harFilePath = mTDir.itemPath("empty.har");
  QVERIFY(mTDir.touch("empty.har", QByteArray()));

  // 解析.har文件
  HarFiles harFiles;
  QVERIFY(!harFiles(harFilePath));
  QVERIFY(harFiles.empty());
}

void HarFilesTest::test_GetPathItem() {
  // 测试各种URL格式
  QCOMPARE(GetPathItemName("https://example.com/image.jpg"), "image");
  QCOMPARE(GetPathItemName("https://example.com/path/to/image.png"), "image");
  QCOMPARE(GetPathItemName("https://example.com/image.with.dots.jpeg"), "image.with.dots");
  QCOMPARE(GetPathItemName("https://example.com/no_extension"), "no_extension");
  QCOMPARE(GetPathItemName("https://example.com/long.extension"), "long.extension"); // 扩展名长度>5
  QCOMPARE(GetPathItemName("relative/path/file.txt"), "file");
  QCOMPARE(GetPathItemName("filename-only"), "filename-only");
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
  QString harFilePath = mTDir.itemPath("encoding.har");
  QVERIFY(mTDir.touch("encoding.har", harContent.toUtf8()));

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
  QCOMPARE(harFiles[2].content, QByteArray("aGVsbG8=")); // 因为没有指定encoding，所以不进行base64解码
}

#include "HarFilesTest.moc"
REGISTER_TEST(HarFilesTest, false)
