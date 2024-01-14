#include "PerformersPreviewTextBrowser.h"
#include "Actions/FileBasicOperationsActions.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "Tools/PerformerJsonFileHelper.h"

#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QKeyEvent>
#include <QSqlField>
#include <QSqlQuery>
#include <QTextStream>

constexpr int PerformersPreviewTextBrowser::SHOW_IMGS_CNT_LIST[];
constexpr int PerformersPreviewTextBrowser::N_SHOW_IMGS_CNT_LIST;

const QString PerformersPreviewTextBrowser::HTML_IMG_TEMPLATE = "<a href=\"file:///%1\"><img src=\"%1\" alt=\"%2\" width=\"%3\"></a><br/>\n";
constexpr int PerformersPreviewTextBrowser::HTML_IMG_FIXED_WIDTH;

const QString PerformersPreviewTextBrowser::VID_LINK_TEMPLATE = "<a href=\"file:///%1\">&#9654;%1</a>";

QString PerformersPreviewTextBrowser::PERFORMER_HTML_TEMPLATE;
const QRegExp PerformersPreviewTextBrowser::IMG_VID_SEP_COMP("\\||\r\n|\n");

PerformersPreviewTextBrowser::PerformersPreviewTextBrowser(QWidget* parent) : QTextBrowser(parent) {
  setReadOnly(true);
  setOpenLinks(false);
  setOpenExternalLinks(true);
  subscribe();
}

bool PerformersPreviewTextBrowser::operator()(const QSqlRecord& record, const QString& m_imageHostPath, const int m_performerImageHeight) {
  const QString& name = record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString();
  const int rates = record.field(PERFORMER_DB_HEADER_KEY::Rate).value().toInt();
  const QString& akas = record.field(PERFORMER_DB_HEADER_KEY::AKA).value().toString();
  const QString& tags = record.field(PERFORMER_DB_HEADER_KEY::Tags).value().toString();
  const QString& ori = record.field(PERFORMER_DB_HEADER_KEY::Orientation).value().toString();
  const QString& imgs = record.field(PERFORMER_DB_HEADER_KEY::Imgs).value().toString();
  const QString& vids = record.field(PERFORMER_DB_HEADER_KEY::Vids).value().toString();
  QString details = record.field(PERFORMER_DB_HEADER_KEY::Detail).value().toString();  // for display in html. don't mix toString().replace() together
  details.replace(IMG_VID_SEP_COMP, "<br/>");

  m_imgsLst = InitImgsList(imgs);

  QString vidsLinks;
  if (not vids.isEmpty()) {
    for (const QString& vidPath : vids.split(IMG_VID_SEP_COMP)) {
      vidsLinks += (VID_LINK_TEMPLATE.arg(vidPath) + "<br/>");
    }
  }

  dirPath = m_imageHostPath + '/' + ori + '/' + name;
  m_curImgCntIndex = 0;
  const QString& firstImgPath = m_imgsLst.isEmpty() ? "" : dirPath + '/' + m_imgsLst.front();
  if (PERFORMER_HTML_TEMPLATE.isEmpty()){
      PERFORMER_HTML_TEMPLATE = TextReader(":/PERFORMER_HTML_TEMPLATE");
  }
  const QString& htmlSrc = PERFORMER_HTML_TEMPLATE.arg(name)
                               .arg(firstImgPath)
                               .arg(m_performerImageHeight)
                               .arg(rates)
                               .arg(akas)
                               .arg(tags)
                               .arg(ori)
                               .arg(vidsLinks)
                               .arg(details);
  setHtml(htmlSrc);
  return true;
}

void PerformersPreviewTextBrowser::subscribe() {
  connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &PerformersPreviewTextBrowser::ShowRemainImages);
  //  connect(this->verticalScrollBar(), &QScrollBar::actionTriggered, this, &PerformersPreviewTextBrowser::onVerticalScrollBarAction);
  connect(this, &QTextBrowser::anchorClicked, this, &PerformersPreviewTextBrowser::onAnchorClicked);
}

QSize PerformersPreviewTextBrowser::sizeHint() const {
  auto w = PreferenceSettings().value("PerformersManagerWidgetDockerWidth", DOCKER_DEFAULT_SIZE.width()).toInt();
  auto h = PreferenceSettings().value("PerformersManagerWidgetDockerlHeight", DOCKER_DEFAULT_SIZE.height()).toInt();
  return QSize(w, h);
}

auto PerformersPreviewTextBrowser::keyPressEvent(QKeyEvent* e) -> void {
  if (e->key() == Qt::Key_PageDown) {
    bool loadNextSucceed = onVerticalScrollBarAction(QScrollBar::SliderToMaximum);
    if (not loadNextSucceed) {  // already loaded finished;
      QTextBrowser::keyPressEvent(e);
    }
    return;
  }
  QTextBrowser::keyPressEvent(e);
}

QStringList PerformersPreviewTextBrowser::InitImgsList(const QString& imgs) const {
  static const auto imgHumanSorter = [](const QString& lhs, const QString& rhs) -> bool {
    if (lhs.size() != rhs.size()) {
      return lhs.size() < rhs.size();
    }
    return lhs < rhs;
  };
  QStringList imgsLst = imgs.split(IMG_VID_SEP_COMP);
  std::sort(imgsLst.begin(), imgsLst.end(), imgHumanSorter);
  // images human sort 0 < 1 < ... < 9 < 10. not in alphabeit
  return imgsLst;
}

bool PerformersPreviewTextBrowser::hasNextImgs() const {
  return (not m_imgsLst.isEmpty()) and m_curImgCntIndex + 1 < N_SHOW_IMGS_CNT_LIST;
}

QString PerformersPreviewTextBrowser::nextImgsHTMLSrc() {
  QString imgSrc;
  const QDir dir(dirPath);
  for (int i = SHOW_IMGS_CNT_LIST[m_curImgCntIndex]; i < m_imgsLst.size() and i < SHOW_IMGS_CNT_LIST[m_curImgCntIndex + 1]; ++i) {
    const QString& imgName = m_imgsLst[i];
    imgSrc += HTML_IMG_TEMPLATE.arg(dir.absoluteFilePath(imgName)).arg(imgName).arg(HTML_IMG_FIXED_WIDTH);
  }
  ++m_curImgCntIndex;
  return imgSrc;
}

bool PerformersPreviewTextBrowser::onAnchorClicked(const QUrl& url) {
  if (not url.isLocalFile()) {
    return false;
  }
  QDesktopServices::openUrl(url);
  return true;
}

bool PerformersPreviewTextBrowser::ShowRemainImages(const int val) {
  if (this->verticalScrollBar()->maximum() != val) {
    return false;
  }
  return onVerticalScrollBarAction(QScrollBar::SliderToMaximum);
}

bool PerformersPreviewTextBrowser::onVerticalScrollBarAction(const int action) {
  if (action != QScrollBar::SliderToMaximum or not hasNextImgs()) {
    return false;
  }
  const QString& insertHtmlSrc = nextImgsHTMLSrc();
  if (insertHtmlSrc.isEmpty()) {
    return true;
  }

  auto cur = this->textCursor();
  cur.movePosition(QTextCursor::MoveOperation::End);
  setTextCursor(cur);
  insertHtml(insertHtmlSrc);
  return true;
}
