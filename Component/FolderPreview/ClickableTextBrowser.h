#ifndef CLICKABLETEXTBROWSER_H
#define CLICKABLETEXTBROWSER_H

#include <QTextBrowser>

class ClickableTextBrowser : public QTextBrowser {
 public:
  ClickableTextBrowser(QWidget* parent = nullptr);

 protected:
  static const QString HTML_H1_TEMPLATE;
  static const QString HTML_IMG_TEMPLATE;
  static const QString VID_LINK_TEMPLATE;
  static constexpr int HTML_IMG_FIXED_WIDTH{600};

 private:
  bool onAnchorClicked(const QUrl& url);
};

#endif  // CLICKABLETEXTBROWSER_H
