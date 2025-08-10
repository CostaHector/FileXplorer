#ifndef CLICKABLETEXTBROWSER_H
#define CLICKABLETEXTBROWSER_H

#include <QTextBrowser>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QMenu>
#include <QAction>
#include <QToolBar>

class ClickableTextBrowser : public QTextBrowser {
public:
  ClickableTextBrowser(QWidget* parent = nullptr);
  static const QString VID_LINK_TEMPLATE;
  static const QString HTML_IMG_TEMPLATE;
  static const QString HTML_H1_TEMPLATE;
  static constexpr int HTML_IMG_FIXED_WIDTH{600};

  QStringList GetSelectedTexts() const {
    QStringList texts;
    for (const auto& sel : mMultiSelections) {
      texts << sel.cursor.selectedText();
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

  void onSearchSelectionReq();
  void onSearchSelectionAdvanceReq();
  void onSearchMultiSelectionReq();

  static QString FormatSearchSentence(QString text);
  static QString GetSearchResultParagraphDisplay(const QString& searchText);
  static QString BuildMultiKeywordLikeCondition(const QStringList& keywords, bool& pNeedSearchDb);

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
    static constexpr int marginX = 32, marginY = 32;
    mFloatingTb->move(width() - mFloatingTb->width() - marginX, height() - mFloatingTb->height() - marginY);
    mFloatingTb->raise();
  }

private:
  void AppendASelection(const QTextCursor &cursor);
  void ClearAllSelections();
  void SearchAndAppendParagraphOfResult(const QString& searchText);

  bool onAnchorClicked(const QUrl& url);

  QMenu *mBrowserMenu{nullptr};
  QToolBar* mFloatingTb{nullptr};

  bool mbDragging = false;                  // 是否正在拖拽
  QPoint mDraggingStartPos;                      // 拖拽起始坐标
  QList<QTextEdit::ExtraSelection> mMultiSelections;  // 存储多个选区
  static constexpr int MIN_KEYWORD_LEN{7};
};

#endif  // CLICKABLETEXTBROWSER_H
