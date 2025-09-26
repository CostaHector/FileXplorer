#include <QtTest/QtTest>
#include <QTestEventList>
#include <QSignalSpy>

#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include "TDir.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "ArchiveFiles.h"
#include "BeginToExposePrivateMember.h"
#include "ImagesInFolderBrowser.h"
#include "ImagesInFolderSlider.h"
#include "ImgVidOthInFolderPreviewer.h"
#include "EndToExposePrivateMember.h"
#include "DraggableToolButton.h"
#include "StyleSheet.h"
#include "ImageTestPrecoditionTools.h"
#include <QScrollBar>


using namespace ImageTestPrecoditionTools;

// a mixed testcase contains 3 class

class ImagesInFolderBrowserTest : public PlainTestSuite {
  Q_OBJECT
public:

  static constexpr int SVG_IMG_COUNT {14};
  static constexpr int MP4_VID_COUNT {1};
  static constexpr int OTH_JSON_COUNT {1};
  TDir mTDir;
  QDir mDir {mTDir.path()};
  QString itemsFolderPath {mDir.absoluteFilePath("path")};

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
      const QString svgContent = GetSvgContentTemplate().arg(SVG_FILL_COLORS[imgIndex % SVG_IMG_COUNT]).arg(contentQChar);
      nodes.push_back(FsNodeEntry{QString{"path/%1.svg"}.arg(contentQChar), false, svgContent.toUtf8()});
    }
    nodes.push_back(FsNodeEntry{"path/FileNotAImage.mp4", false, ""});
    nodes.push_back(FsNodeEntry{"path/FileNotAImage.json", false, ""});
    QCOMPARE(mTDir.createEntries(nodes), nodes.size());

    QVERIFY(mDir.exists());
  }

  void cleanupTestCase() {
    Configuration().clear();
  }

  void notImageFile_browser_verticalbar_invisible() {
    QString notImagePath = mDir.absoluteFilePath("path/FileNotAImage.mp4");
    QVERIFY(QFile::exists(notImagePath));

    ImagesInFolderBrowser browser;
    browser.mIconSize = QSize{400, 300};
    browser.setFixedSize(500, 500);
    QVERIFY(browser(notImagePath));
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

  void singleImageFile_browser_verticalbar_invisible() {
    QString svgImagePath = mDir.absoluteFilePath("path/A.svg");
    QVERIFY(QFile::exists(svgImagePath));

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

  void folderWith_3BatchImages_browser_ok() {
    ImagesInFolderBrowser browser;
    browser.mIconSize = QSize{200, 200};
    browser.setFixedSize(256, 256);
    QVERIFY(browser(itemsFolderPath));
    browser.show();
    browser.activateWindow();

    QCOMPARE(browser.m_dirPath, itemsFolderPath);
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
    QVERIFY(browser.verticalScrollBar()->value() < browser.verticalScrollBar()->maximum()); // not at the bottom

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
    QVERIFY(browser.verticalScrollBar()->value() < browser.verticalScrollBar()->maximum()); // not at the bottom after load

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
    QVERIFY(browser.verticalScrollBar()->value() < browser.verticalScrollBar()->maximum()); // not at the bottom after load

    // 4. try reload again. no need load now
    QCOMPARE(browser.hasNextImgs(), false);
  }

  void wheelDownToMaximun_browser_willShowRemainImages_ok() {
    ImagesInFolderBrowser browser;
    browser.mIconSize = QSize{200, 200};
    browser.setFixedSize(256, 256);
    QVERIFY(browser(itemsFolderPath));
    browser.show();
    browser.activateWindow();
    // QCOMPARE(QTest::qWaitForWindowExposed(&browser), true);

    QCOMPARE(browser.m_dirPath, itemsFolderPath);
    QCOMPARE(browser.m_imgsLst.size(), SVG_IMG_COUNT);

    const QPoint centerPos = browser.geometry().center();
    QPoint defaultPixelDelta;

    // 1. load first batch
    QString html = browser.toHtml();
    int imgCount = html.count("<img");
    const int firstTimeWheelDownImagesCnt{ImagesInFolderBrowser::SHOW_IMGS_CNT_LIST[1]};
    QVERIFY(SVG_IMG_COUNT >= firstTimeWheelDownImagesCnt);
    QCOMPARE(imgCount, firstTimeWheelDownImagesCnt);
    QCOMPARE(browser.m_curImgCntIndex, 1);
    QCOMPARE(browser.hasNextImgs(), true);

    QScrollBar* scrollBar = browser.verticalScrollBar();
    const int maximunY2 = scrollBar->maximum();
    QVERIFY(scrollBar != nullptr);
    QVERIFY(maximunY2 > 0);

    // 2. wheel up no load, content unchange at all
    const int scrollUpDistance = scrollBar->value();
    QPoint angleDeltaUp(0, 15 * (scrollUpDistance + 3));

    // QPointF pos, QPointF globalPos, QPoint pixelDelta, QPoint angleDelta, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::ScrollPhase phase, bool inverted, Qt::MouseEventSource source = Qt::MouseEventNotSynthesized
    QWheelEvent wheelUpEvent(centerPos, browser.mapToGlobal(centerPos), defaultPixelDelta, angleDeltaUp, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false, Qt::MouseEventNotSynthesized);
    browser.wheelEvent(&wheelUpEvent);
    html = browser.toHtml();
    imgCount = html.count("<img");
    QCOMPARE(imgCount, firstTimeWheelDownImagesCnt);
    QCOMPARE(browser.m_curImgCntIndex, 1);
    QCOMPARE(browser.hasNextImgs(), true);
    QCOMPARE(scrollBar->value(), 0);

    // 3. wheel down even a large distance will only load next batch (not all batches left)
    const int scrollDownDistance = maximunY2 - scrollBar->value();
    QPoint angleDeltaDown(0, -15 * (scrollDownDistance + 4096)); // large enough distance
    QWheelEvent wheelDownEvent(centerPos, browser.mapToGlobal(centerPos), defaultPixelDelta, angleDeltaDown, Qt::NoButton, Qt::NoModifier, Qt::NoScrollPhase, false, Qt::MouseEventNotSynthesized);
    browser.wheelEvent(&wheelDownEvent);

    html = browser.toHtml();
    imgCount = html.count("<img");
    const int secondTimeWheelDownImagesCnt{ImagesInFolderBrowser::SHOW_IMGS_CNT_LIST[2]};
    QVERIFY(SVG_IMG_COUNT >= secondTimeWheelDownImagesCnt);
    QCOMPARE(imgCount, secondTimeWheelDownImagesCnt);
    QCOMPARE(browser.m_curImgCntIndex, 2);
    QCOMPARE(browser.hasNextImgs(), true);
  }

  void singleImageFile_timerSlideShow_timer_inactive_ok() {
    QString svgImagePath = mDir.absoluteFilePath("path/A.svg");
    QVERIFY(QFile::exists(svgImagePath));

    ImagesInFolderSlider slider;
    slider.setFixedSize(200, 200);
    QCOMPARE(slider.m_inFolderImgIndex, 0 * ImagesInFolderSlider::MAX_LABEL_CNT);
    QCOMPARE(slider.m_nextImgTimer->isActive(), false);
    QCOMPARE(slider.m_nextImgTimer->interval(), ImagesInFolderSlider::SLIDE_TO_NEXT_IMG_TIME_INTERVAL);
    slider(svgImagePath);

    QVERIFY(slider.m_imgsUnderAPath != nullptr);
    QCOMPARE(slider.m_imgsUnderAPath->size(), 1);
    QCOMPARE(slider.m_nextImgTimer->isActive(), false);
    QCOMPARE(slider.m_inFolderImgIndex, 1);
    QCOMPARE(slider.hasNextImgs(), false);

    // change path to a not image file, memory release ok
    QString notImagePath = mDir.absoluteFilePath("path/FileNotAImage.mp4");
    QVERIFY(QFile::exists(notImagePath));
    slider(notImagePath);
    QCOMPARE(slider.m_imgsUnderAPath, nullptr);
    QCOMPARE(slider.m_nextImgTimer->isActive(), false);

    for (int i = 0; i < ImagesInFolderSlider::MAX_LABEL_CNT; ++i) { // should be cleared
      const QPixmap& pm = slider.m_imgLabelsList[i]->pixmap(Qt::ReturnByValueConstant::ReturnByValue);
      QVERIFY(pm.isNull());
    }
  }

  void folderWith_7BatchImages_timerSlideShow_ok() {
    ImagesInFolderSlider slider;
    slider.setFixedSize(400, 300);
    QCOMPARE(slider.m_inFolderImgIndex, 0 * ImagesInFolderSlider::MAX_LABEL_CNT);
    QCOMPARE(slider.m_nextImgTimer->isActive(), false);
    QCOMPARE(slider.m_nextImgTimer->interval(), ImagesInFolderSlider::SLIDE_TO_NEXT_IMG_TIME_INTERVAL);
    slider(itemsFolderPath);

    QVERIFY(slider.m_imgsUnderAPath != nullptr);
    QCOMPARE(slider.m_imgsUnderAPath->size(), SVG_IMG_COUNT);

    QCOMPARE(slider.m_nextImgTimer->isActive(), true);
    QCOMPARE(slider.m_inFolderImgIndex, 1 * ImagesInFolderSlider::MAX_LABEL_CNT);
    QCOMPARE(slider.hasNextImgs(), true);

    // timeout signal is not connected, manual call slot instead
    slider.nxtImgInFolder();
    QCOMPARE(slider.m_nextImgTimer->isActive(), true);
    QCOMPARE(slider.m_inFolderImgIndex, 2 * ImagesInFolderSlider::MAX_LABEL_CNT);
    QCOMPARE(slider.hasNextImgs(), true);

    slider.nxtImgInFolder();
    QCOMPARE(slider.m_nextImgTimer->isActive(), true);
    QCOMPARE(slider.m_inFolderImgIndex, 3 * ImagesInFolderSlider::MAX_LABEL_CNT);
    QCOMPARE(slider.hasNextImgs(), true);

    slider.nxtImgInFolder();
    QCOMPARE(slider.m_nextImgTimer->isActive(), true);
    QCOMPARE(slider.m_inFolderImgIndex, 4 * ImagesInFolderSlider::MAX_LABEL_CNT);
    QCOMPARE(slider.hasNextImgs(), true);

    slider.nxtImgInFolder();
    QCOMPARE(slider.m_nextImgTimer->isActive(), true);
    QCOMPARE(slider.m_inFolderImgIndex, 5 * ImagesInFolderSlider::MAX_LABEL_CNT);
    QCOMPARE(slider.hasNextImgs(), true);

    slider.nxtImgInFolder();
    QCOMPARE(slider.m_nextImgTimer->isActive(), true);
    QCOMPARE(slider.m_inFolderImgIndex, 6 * ImagesInFolderSlider::MAX_LABEL_CNT);
    QCOMPARE(slider.hasNextImgs(), true);

    slider.nxtImgInFolder();
    QCOMPARE(slider.m_nextImgTimer->isActive(), false);  //  all 14 images have been shown once
    QCOMPARE(slider.m_inFolderImgIndex, SVG_IMG_COUNT);
    QCOMPARE(slider.hasNextImgs(), false);

    slider.nxtImgInFolder();
    QCOMPARE(slider.m_inFolderImgIndex, SVG_IMG_COUNT); // try show next batch, nothing happend

    for (int i = 0; i < ImagesInFolderSlider::MAX_LABEL_CNT; ++i) { // should valid
      const QPixmap& pm = slider.m_imgLabelsList[i]->pixmap(Qt::ReturnByValueConstant::ReturnByValue);
      QCOMPARE(pm.isNull(), false);
    }
  }

  static QStringList defActionsTextList(ReorderableToolBar* toolbar) {
    if (toolbar == nullptr) {return {};}
    QStringList result;
    foreach (QAction* action, toolbar->actions()) {
      auto* pWid = toolbar->widgetForAction(action);
      if (const DraggableToolButton* tb = dynamic_cast<const DraggableToolButton*>(pWid)) {
        if (const QAction* pDefaultAct = tb->defaultAction()) {
          result.append(pDefaultAct->text());
        }
      }
    }
    return result;
  }

  void default_order_imgVidOthInFolderPreviewer_ok() {
    // precondition:
    Configuration().clear();
    Configuration().setValue(BrowserKey::FLOATING_MEDIA_TYPE_SEQ.name, "012");
    Configuration().setValue(BrowserKey::FLOATING_IMAGE_VIEW_SHOW.name, true);
    Configuration().setValue(BrowserKey::FLOATING_VIDEO_VIEW_SHOW.name, true);
    Configuration().setValue(BrowserKey::FLOATING_OTHER_VIEW_SHOW.name, true);

    // in default order
    ImgVidOthInFolderPreviewer previewer("defaultOrder_imgVidOthInFolderPreview");
    previewer.setFixedSize(300, 600);
    previewer(itemsFolderPath);

    QVERIFY(previewer.mTypeToDisplayTB != nullptr);
    QVERIFY(previewer.mImgVidOtherSplitter != nullptr);
    QCOMPARE(previewer.mImgVidOtherSplitter->count(), 3);

    const QString imgCntStr{QString::number(SVG_IMG_COUNT)};
    const QString vidCntStr{QString::number(MP4_VID_COUNT)};
    const QString othCntStr{QString::number(OTH_JSON_COUNT)};

    QCOMPARE(previewer._IMG_ACT->text(), imgCntStr);
    QCOMPARE(previewer._VID_ACT->text(), vidCntStr);
    QCOMPARE(previewer._OTH_ACT->text(), othCntStr);

    QVERIFY(previewer.mImgTv != nullptr);
    QVERIFY(previewer.mVidTv != nullptr);
    QVERIFY(previewer.mOthTv != nullptr);

    // before 012
    QCOMPARE(Configuration().value(BrowserKey::FLOATING_MEDIA_TYPE_SEQ.name).toString(), "012");
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(0), previewer.mImgTv);
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(1), previewer.mVidTv);
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(2), previewer.mOthTv);
    QCOMPARE(defActionsTextList(previewer.mTypeToDisplayTB), (QStringList{imgCntStr, vidCntStr, othCntStr}));

    // reorder, after 120
    QVERIFY(MoveWidgetAtFromIndexInFrontOfDestIndex(0, 3, *previewer.mTypeToDisplayTB));
    emit previewer.mTypeToDisplayTB->widgetMoved(0, 3);
    QCOMPARE(Configuration().value(BrowserKey::FLOATING_MEDIA_TYPE_SEQ.name).toString(), "120");
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(0), previewer.mVidTv);
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(1), previewer.mOthTv);
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(2), previewer.mImgTv);
    QCOMPARE(defActionsTextList(previewer.mTypeToDisplayTB), (QStringList{vidCntStr, othCntStr, imgCntStr}));

    // reorder, after 012
    QVERIFY(MoveWidgetAtFromIndexInFrontOfDestIndex(2, 0, *previewer.mTypeToDisplayTB));
    emit previewer.mTypeToDisplayTB->widgetMoved(2, 0);
    QCOMPARE(Configuration().value(BrowserKey::FLOATING_MEDIA_TYPE_SEQ.name).toString(), "012");
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(0), previewer.mImgTv);
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(1), previewer.mVidTv);
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(2), previewer.mOthTv);
    QCOMPARE(defActionsTextList(previewer.mTypeToDisplayTB), (QStringList{imgCntStr, vidCntStr, othCntStr}));
  }

  void specified_order_imgVidOthInFolderPreviewer_ok() {
    // precondition:
    Configuration().clear();
    Configuration().setValue(BrowserKey::FLOATING_MEDIA_TYPE_SEQ.name, "210");
    Configuration().setValue(BrowserKey::FLOATING_IMAGE_VIEW_SHOW.name, true);
    Configuration().setValue(BrowserKey::FLOATING_VIDEO_VIEW_SHOW.name, true);
    Configuration().setValue(BrowserKey::FLOATING_OTHER_VIEW_SHOW.name, false);

    // in default order
    ImgVidOthInFolderPreviewer previewer("specifiedOrder_imgVidOthInFolderPreview");
    previewer.setFixedSize(300, 600);
    previewer(itemsFolderPath);
    previewer.show();
    QVERIFY(QTest::qWaitForWindowExposed(&previewer));

    QVERIFY(previewer.mTypeToDisplayTB != nullptr);
    QVERIFY(previewer.mImgVidOtherSplitter != nullptr);
    QCOMPARE(previewer.mImgVidOtherSplitter->count(), 3);

    const QString imgCntStr{QString::number(SVG_IMG_COUNT)};
    const QString vidCntStr{QString::number(MP4_VID_COUNT)};
    const QString othCntStr{"0"}; // invisible. the text will not update at all

    QCOMPARE(previewer._IMG_ACT->text(), imgCntStr);
    QCOMPARE(previewer._VID_ACT->text(), vidCntStr);
    QCOMPARE(previewer._OTH_ACT->text(), othCntStr);

    QVERIFY(previewer.mImgTv != nullptr);
    QVERIFY(previewer.mVidTv != nullptr);
    QVERIFY(previewer.mOthTv != nullptr);

    QCOMPARE(previewer.mImgTv->isVisible(), true);
    QCOMPARE(previewer.mVidTv->isVisible(), true);
    QCOMPARE(previewer.mOthTv->isVisible(), false); // should hide

    // before 210
    QCOMPARE(Configuration().value(BrowserKey::FLOATING_MEDIA_TYPE_SEQ.name).toString(), "210");
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(0), previewer.mOthTv);
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(1), previewer.mVidTv);
    QCOMPARE(previewer.mImgVidOtherSplitter->widget(2), previewer.mImgTv);
    QCOMPARE(defActionsTextList(previewer.mTypeToDisplayTB), (QStringList{othCntStr, vidCntStr, imgCntStr}));
  }
};

constexpr int ImagesInFolderBrowserTest::SVG_IMG_COUNT;
constexpr int ImagesInFolderBrowserTest::MP4_VID_COUNT;
constexpr int ImagesInFolderBrowserTest::OTH_JSON_COUNT;

#include "ImagesInFolderBrowserTest.moc"
REGISTER_TEST(ImagesInFolderBrowserTest, false)
