#include "ClickableTextBrowser.h"
#include <QDesktopServices>

const QString ClickableTextBrowser::HTML_H1_TEMPLATE{R"(<a href="file:///%1">%2</a>)"};
const QString ClickableTextBrowser::HTML_IMG_TEMPLATE{R"(<a href="file:///%1"><img src="%1" alt="%2" width="%3"></a><br/>\n)"};
const QString ClickableTextBrowser::VID_LINK_TEMPLATE{R"(<a href="file:///%1">&#9654;%2</a>)"};
constexpr int ClickableTextBrowser::HTML_IMG_FIXED_WIDTH;

ClickableTextBrowser::ClickableTextBrowser(QWidget* parent) : QTextBrowser{parent}
{
  setReadOnly(true);
  setOpenLinks(false);
  setOpenExternalLinks(true);

  connect(this, &QTextBrowser::anchorClicked, this, &ClickableTextBrowser::onAnchorClicked);
}

bool ClickableTextBrowser::onAnchorClicked(const QUrl& url) {
  if (!url.isLocalFile()) {
    return false;
  }
  return QDesktopServices::openUrl(url);
}
