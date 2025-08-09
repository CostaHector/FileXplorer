#ifndef CLICKABLETEXTBROWSER_H
#define CLICKABLETEXTBROWSER_H

#include <QTextBrowser>
#include <QContextMenuEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QAction>

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
    const bool bHasSelectionText{!GetCurrentSelectedText().isEmpty()};
    m_searchCurSelect->setEnabled(bHasSelectionText);
    m_searchCurSelectAdvance->setEnabled(bHasSelectionText);
    m_clearMultiSelections->setEnabled(!mMultiSelections.isEmpty());
    m_menu->exec(event->globalPos());
  }

  void onSearchSelectionReq();
  void onSearchSelectionAdvanceReq();
  void onSearchMultiSelectionReq();

  static QString FormatSearchSentence(QString text);
  static QString GetSearchResultParagraphDisplay(const QString& searchText);
  static QString BuildMultiKeywordLikeCondition(const QStringList& keywords);

protected:
  void mouseDoubleClickEvent(QMouseEvent *e) override;
  void mousePressEvent(QMouseEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

private:
  void AppendASelection(const QTextCursor &cursor);
  void ClearAllSelections();
  void SearchAndAppendParagraphOfResult(const QString& searchText);

  bool onAnchorClicked(const QUrl& url);

  QMenu *m_menu{nullptr};
  QAction *m_searchCurSelect{nullptr};
  QAction *m_searchCurSelectAdvance{nullptr};
  QAction *m_searchMultiSelect{nullptr};
  QAction *m_clearMultiSelections{nullptr};

  bool mbDragging = false;                  // 是否正在拖拽
  QPoint mDraggingStartPos;                      // 拖拽起始坐标
  QList<QTextEdit::ExtraSelection> mMultiSelections;  // 存储多个选区
};

#endif  // CLICKABLETEXTBROWSER_H
