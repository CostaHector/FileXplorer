#ifndef CLICKABLETEXTBROWSER_H
#define CLICKABLETEXTBROWSER_H

#include <QTextBrowser>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QToolBar>
#include "CastBrowserHelper.h"

class ClickableTextBrowser : public QTextBrowser {
  Q_OBJECT
public:
  explicit ClickableTextBrowser(QWidget* parent = nullptr);
  ~ClickableTextBrowser();
  void wheelEvent(QWheelEvent *event) override;
  const QSize& iconSize() const { return mIconSize; }

  QStringList GetSelectedTexts() const {
    QStringList texts;
    texts.reserve(mMultiSelections.size());
    for (const auto& sel : mMultiSelections) {
      texts << sel.cursor.selectedText().trimmed();
    }
    return texts;
  }

  QString GetCurrentSelectedText() const {
    return textCursor().selectedText();
  }

  void contextMenuEvent(QContextMenuEvent *event) override {
    if (mBrowserMenu != nullptr) {
      mBrowserMenu->exec(event->globalPos());
    }
  }

  void onSearchSelectionReq(); // search selection in DB_TABLE::MOVIES
  void onSearchSelectionAdvanceReq(); // search selection(editable) in DB_TABLE::MOVIES
  void onSearchMultiSelectionReq(); // search multi selection in DB_TABLE::MOVIES
  int onAppendMultiSelectionToCastDbReq(); // append selections to DB_TABLE::PERFORMERS

  static QString FormatSearchSentence(QString text);
  static QString GetSearchResultParagraphDisplay(const QString& searchText);
  static QString BuildMultiKeywordLikeCondition(const QStringList& keywords, bool& pNeedSearchDb);
  static QString& UpdateImagesSizeInHtmlSrc(QString& htmlSrc, const QSize& newSize);

  /* for cast preview only below */
  void SetCastHtmlParts(const CastHtmlParts& castHtmls) {mCastHtmls = castHtmls;}
  void UpdateHtmlContents() { setHtml(mCastHtmls.fullHtml(mCastVideosVisisble, mCastImagesVisisble)); }
  /* for cast preview only above */

signals:
  void iconSizeChanged(QSize newSize);

protected:
  void mouseDoubleClickEvent(QMouseEvent *e) override;
  void mousePressEvent(QMouseEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;
  void resizeEvent(QResizeEvent *event) override {
    QTextBrowser::resizeEvent(event);
    AdjustButtonPosition();
  }

  void AdjustButtonPosition() {
    if (mFloatingTb == nullptr) {return;}
    static constexpr int marginX = 16, marginY = 32;
    mFloatingTb->move(width() - mFloatingTb->width() - marginX, height() - mFloatingTb->height() - marginY);
    mFloatingTb->raise();
  }

private:
  void AppendASelection(const QTextCursor &cursor);
  void ClearAllSelections();
  void SearchAndAppendParagraphOfResult(const QString& searchText);
  void CopySelectedTextToClipboard() const;
  bool onAnchorClicked(const QUrl& url);

  QMenu *mBrowserMenu{nullptr};
  QToolBar* mFloatingTb{nullptr};

  bool mbDragging = false;                            // 是否正在拖拽
  QPoint mDraggingStartPos;                           // 拖拽起始坐标
  QList<QTextEdit::ExtraSelection> mMultiSelections;  // 存储多个选区
  static constexpr int MIN_SINGLE_SEARCH_PATTERN_LEN{2 + 4}; // "%keyword%"
  static constexpr int MIN_EACH_KEYWORD_LEN{4};       // "%" + "keyword" + "%"

  CastHtmlParts mCastHtmls;
  bool mCastVideosVisisble{true}, mCastImagesVisisble{true};

  int mCurIconSizeIndex{14};
  QSize mIconSize;
};

#endif  // CLICKABLETEXTBROWSER_H
