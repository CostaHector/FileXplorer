#include <QtTest/QtTest>
#include <QTestEventList>
#include <QSignalSpy>

#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include "TDir.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "ImagesInFolderBrowser.h"
#include "EndToExposePrivateMember.h"
#include "StyleSheet.h"
#include <QScrollBar>

class ImagesInFolderBrowserTest : public PlainTestSuite {
  Q_OBJECT
public:
  const QStringList SVG_FILL_COLORS = {
      "#FF0000", // 红色
      "#00FF00", // 绿色
      "#0000FF", // 蓝色
      "#FFFF00", // 黄色
      "#FF00FF", // 品红
      "#00FFFF", // 青色
      "#FFA500", // 橙色
      "#800080", // 紫色
      "#008000", // 深绿
      "#000080", // 深蓝
      "#FFC0CB", // 粉色
      "#A52A2A", // 棕色
      "#808080", // 灰色
      "#000000", // 黑色
      "#FFFFFF", // 白色
      "#40E0D0"  // 绿松石
  };
  static const QString SVG_TEMPLATE;
  static constexpr int SVG_IMG_COUNT {14};
  TDir mTDir;
  QDir mDir {mTDir.path()};

  void ChangeIconSize();

private slots:
  void initTestCase() {
    Configuration().clear();

    Configuration().setValue(BrowserKey::CLICKABLE_TEXT_BROWSER_FONT_POINT_SIZE.name, 12);
    Configuration().setValue(BrowserKey::CLICKABLE_TEXT_BROWSER_ICON_SIZE_INDEX.name, IMAGE_SIZE::ICON_SIZE_CANDIDATES_N - 1);

    QList<FsNodeEntry> nodes;
    nodes.reserve(SVG_IMG_COUNT);
    for (int imgIndex = 0; imgIndex < SVG_IMG_COUNT; ++imgIndex) {
      const char contentChar = 'A' + imgIndex;
      const QChar contentQChar{contentChar};
      const QString svgContent = SVG_TEMPLATE.arg(SVG_FILL_COLORS[imgIndex % SVG_IMG_COUNT]).arg(contentQChar);
      nodes.push_back(FsNodeEntry{QString{"path/%1.svg"}.arg(contentQChar), false, svgContent.toUtf8()});
    }
    nodes.push_back(FsNodeEntry{"path/FileNotAImage.mp4", false, ""});
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());
  }

  void cleanupTestCase() {
    Configuration().clear();
  }

  void notImageFile_verticalbar_invisible() {
    QString svgImagePath = mDir.absoluteFilePath("path/FileNotAImage.mp4");
    QVERIFY(mDir.exists());

    ImagesInFolderBrowser browser;
    browser.mIconSize = QSize{400, 300};
    browser.setFixedSize(500, 500);
    QVERIFY(browser(svgImagePath));
    browser.show();
    QCOMPARE(QTest::qWaitForWindowExposed(&browser), true);
    // here verital scrollbar should not show

    QScrollBar* verticalScrollBar = browser.verticalScrollBar();
    QVERIFY(verticalScrollBar != nullptr); // 确保滚动条存在
    QVERIFY2(!verticalScrollBar->isVisible(), "Vertical scrollbar should not be visible for only 1 nothing");

    QCOMPARE(browser.m_dirPath, "");
    QCOMPARE(browser.m_imgsLst.size(), 0);
    QVERIFY(browser.m_curImgCntIndex == 0);

    const QString& html = browser.toHtml();
    QCOMPARE(html.count("<img"), 0);
  }

  void singleImageFile_verticalbar_invisible() {
    QString svgImagePath = mDir.absoluteFilePath("path/A.svg");
    QVERIFY(mDir.exists());

    ImagesInFolderBrowser browser;
    browser.mIconSize = QSize{400, 300};
    browser.setFixedSize(500, 500);
    QVERIFY(browser(svgImagePath));
    browser.show();
    QCOMPARE(QTest::qWaitForWindowExposed(&browser), true);
    // here verital scrollbar should not show

    QScrollBar* verticalScrollBar = browser.verticalScrollBar();
    QVERIFY(verticalScrollBar != nullptr); // 确保滚动条存在
    QVERIFY2(!verticalScrollBar->isVisible(), "Vertical scrollbar should not be visible for only 1 image");

    QCOMPARE(browser.m_dirPath, mDir.absoluteFilePath("path"));
    QCOMPARE(browser.m_imgsLst.size(), 1);
    QCOMPARE(browser.m_imgsLst.first(), "A.svg");
    QVERIFY(browser.m_curImgCntIndex > 0);

    // There are 1 and only 1 image "SingleImage" should exist in html source content
    const QString& html = browser.toHtml();
    QCOMPARE(html.count("<img"), 1);
    QVERIFY(html.contains("path/A.svg"));
  }

  void folderWith_11svg_images() {
    QString svgFolderPath = mDir.absoluteFilePath("path");
    QVERIFY(mDir.exists());

    ImagesInFolderBrowser browser;
    browser.mIconSize = QSize{400, 300};
    browser.setFixedSize(500, 500);
    QVERIFY(browser(svgFolderPath));
    browser.show();
    QCOMPARE(QTest::qWaitForWindowExposed(&browser), true);

    QCOMPARE(browser.m_dirPath, svgFolderPath);
    QCOMPARE(browser.m_imgsLst.size(), SVG_IMG_COUNT);
    QVERIFY(browser.m_curImgCntIndex > 0);

    // 1. load first batch
    QString html = browser.toHtml();
    int imgCount = html.count("<img");
    const int firstTimeWheelDownImagesCnt{ImagesInFolderBrowser::SHOW_IMGS_CNT_LIST[1]};
    QVERIFY(SVG_IMG_COUNT >= firstTimeWheelDownImagesCnt);
    QCOMPARE(imgCount, firstTimeWheelDownImagesCnt);
    QCOMPARE(browser.m_curImgCntIndex, 1);
    QCOMPARE(browser.hasNextImgs(), true);

    // 2. load second time
    int maximunY2 = browser.verticalScrollBar()->maximum();
    browser.verticalScrollBar()->setValue(maximunY2);
    emit browser.verticalScrollBar()->valueChanged(maximunY2);
    html = browser.toHtml();
    imgCount = html.count("<img");
    const int secondTimeWheelDownImagesCnt{ImagesInFolderBrowser::SHOW_IMGS_CNT_LIST[2]};
    QVERIFY(SVG_IMG_COUNT >= secondTimeWheelDownImagesCnt);
    QCOMPARE(imgCount, secondTimeWheelDownImagesCnt);
    QCOMPARE(browser.m_curImgCntIndex, 2);
    QCOMPARE(browser.hasNextImgs(), true);

    // 3. load third time, all other left
    int maximunY3 = browser.verticalScrollBar()->maximum();
    browser.verticalScrollBar()->setValue(maximunY3);
    emit browser.verticalScrollBar()->valueChanged(maximunY3);
    html = browser.toHtml();
    imgCount = html.count("<img");
    const int thirdTimeWheelDownImagesCnt{ImagesInFolderBrowser::SHOW_IMGS_CNT_LIST[3]};
    QVERIFY(SVG_IMG_COUNT < thirdTimeWheelDownImagesCnt);
    QCOMPARE(imgCount, SVG_IMG_COUNT);
    QCOMPARE(browser.m_curImgCntIndex, 3);

    // 4. try reload again. no need load now
    QCOMPARE(browser.hasNextImgs(), false);
  }

};
const QString ImagesInFolderBrowserTest::SVG_TEMPLATE{R"(<svg xmlns="http://www.w3.org/2000/svg" width="540" height="360" viewBox="0 0 540 360">
  <rect width="540" height="360" fill="%1"/>
  <text x="0" y="300" font-size="360" fill="white">%2</text>
  </svg>)"}; // WARNING: don't use R"" inside class when the class need .moc
constexpr int ImagesInFolderBrowserTest::SVG_IMG_COUNT;

#include "ImagesInFolderBrowserTest.moc"
REGISTER_TEST(ImagesInFolderBrowserTest, false)

