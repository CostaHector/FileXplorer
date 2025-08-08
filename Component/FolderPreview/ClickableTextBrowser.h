#ifndef CLICKABLETEXTBROWSER_H
#define CLICKABLETEXTBROWSER_H

#include <QTextBrowser>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

class ClickableTextBrowser : public QTextBrowser {
public:
  ClickableTextBrowser(QWidget* parent = nullptr);
  static const QString VID_LINK_TEMPLATE;
  static const QString HTML_IMG_TEMPLATE;
  static const QString HTML_H1_TEMPLATE;
  static constexpr int HTML_IMG_FIXED_WIDTH{600};

  void contextMenuEvent(QContextMenuEvent *event) override {
    const bool bHasSelectionText{!textCursor().selectedText().isEmpty()};
    m_searchSelectionAction->setEnabled(bHasSelectionText);
    m_editBeforeSearchAction->setEnabled(bHasSelectionText);
    m_menu->exec(event->globalPos());
  }

  void onSearchSelectionRequested();
  void onEditSelectionBeforeSearchRequested();

  static QString GetStandardSearchKeyWords(QString text);
  static QString GetSearchResultParagraphDisplay(const QString& searchText);

private:
  void SearchAndAppendParagraphOfResult(const QString& searchText);

  bool onAnchorClicked(const QUrl& url);
  QMenu *m_menu{nullptr};
  QAction *m_searchSelectionAction{nullptr};
  QAction *m_editBeforeSearchAction{nullptr};
};

#endif  // CLICKABLETEXTBROWSER_H
