#include "FolderPreviewHTML.h"
#include "Actions/FileBasicOperationsActions.h"
#include "PublicVariable.h"

#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>

constexpr int FolderPreviewHTML::SHOW_IMGS_CNT_LIST[];
constexpr int FolderPreviewHTML::N_SHOW_IMGS_CNT_LIST;

const QString FolderPreviewHTML::HTML_H1_TEMPLATE = "<a href=\"file:///%1\">%2</a>";
const QString FolderPreviewHTML::HTML_H1_WITH_VIDS_TEMPLATE = "<a href=\"file:///%1\">&#9654;%2</a>";
const QString FolderPreviewHTML::HTML_IMG_TEMPLATE = "<a href=\"file:///%1\"><img src=\"%1\" alt=\"%2\" width=\"%3\"></a><br/>\n";
constexpr int FolderPreviewHTML::HTML_IMG_FIXED_WIDTH;

FolderPreviewHTML::FolderPreviewHTML(QWidget* parent) : m_parent(parent), m_PLAY_ACTION(g_fileBasicOperationsActions().OPEN->actions()[0]) {
  setReadOnly(true);
  setOpenLinks(false);
  setOpenExternalLinks(true);
  subscribe();
}

bool FolderPreviewHTML::operator()(const QString& path) {
  QFileInfo fi(path);
  m_curImgCntIndex = 0;
  dirPath = path;
  m_imgsLst.clear();

  int vidCnt = 0;

  if (fi.isDir()) {
    QDir vidDir(path, {}, QDir::NoSort, QDir::Filter::Files);
    vidDir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
    vidCnt = vidDir.entryList().size();
    m_imgsLst = InitImgsList(path);
  }
  const QString& headLine = fi.isDir() ? HTML_H1_WITH_VIDS_TEMPLATE.arg(fi.absoluteFilePath()).arg(fi.fileName())
                                       : HTML_H1_TEMPLATE.arg(fi.absoluteFilePath()).arg(fi.fileName());
  if (m_parent) {
    m_parent->setWindowTitle(QString::number(vidCnt) + '|' + QString::number(m_imgsLst.size()));
  }

  QString htmlSrc;
  htmlSrc += "<!DOCTYPE html>\n<html>\n";
  htmlSrc += "<head><title>Title</title></head>\n";
  htmlSrc += QString(
                 "<body align=\"center\">\n"
                 "<h1>%1</h1>"
                 "<br/>\n")
                 .arg(headLine);
  if (hasNextImgs()) {
    htmlSrc += nextImgsHTMLSrc();
  }
  htmlSrc += "</body>\n</html>\n";
  setHtml(htmlSrc);
  return true;
}

void FolderPreviewHTML::subscribe() {
  connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &FolderPreviewHTML::ShowRemainImages);
  connect(this, &QTextBrowser::anchorClicked, this, &FolderPreviewHTML::onAnchorClicked);
}

QSize FolderPreviewHTML::sizeHint() const {
  auto w = PreferenceSettings().value("dockerHtmlWidth", DOCKER_DEFAULT_SIZE.width()).toInt();
  auto h = PreferenceSettings().value("dockerHtmlHeight", DOCKER_DEFAULT_SIZE.height()).toInt();
  return QSize(w, h);
}

QStringList FolderPreviewHTML::InitImgsList(const QString& dirPath) const {
  QDir dir(dirPath);
  if (not dir.exists()) {
    return {};
  }
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  QStringList imgsLst = dir.entryList(QDir::Filter::Files);
  static const auto imgHumanSorter = [](const QString& lhs, const QString& rhs) -> bool {
    if (lhs.size() != rhs.size()) {
      return lhs.size() < rhs.size();
    }
    return lhs < rhs;
  };
  std::sort(imgsLst.begin(), imgsLst.end(), imgHumanSorter);
  // images human sort 0 < 1 < ... < 9 < 10. not in alphabeit
  return imgsLst;
}

bool FolderPreviewHTML::hasNextImgs() const {
  return (not m_imgsLst.isEmpty()) and m_curImgCntIndex + 1 < N_SHOW_IMGS_CNT_LIST;
}

QString FolderPreviewHTML::nextImgsHTMLSrc() {
  QString imgSrc;
  const QDir dir(dirPath);
  for (int i = SHOW_IMGS_CNT_LIST[m_curImgCntIndex]; i < m_imgsLst.size() and i < SHOW_IMGS_CNT_LIST[m_curImgCntIndex + 1]; ++i) {
    const QString& imgName = m_imgsLst[i];
    imgSrc += HTML_IMG_TEMPLATE.arg(dir.absoluteFilePath(imgName)).arg(imgName).arg(HTML_IMG_FIXED_WIDTH);
  }
  ++m_curImgCntIndex;
  return imgSrc;
}

bool FolderPreviewHTML::onAnchorClicked(const QUrl& url) {
  if (not url.isLocalFile()) {
    return false;
  }
  QFileInfo fi(url.toLocalFile());
  if (TYPE_FILTER::VIDEO_TYPE_SET.contains("*." + fi.suffix()) or fi.isDir()) {
    if (m_PLAY_ACTION) {
      emit m_PLAY_ACTION->triggered(false);
    }
    return true;
  }
  QDesktopServices::openUrl(url);
  return true;
}

bool FolderPreviewHTML::ShowRemainImages(const int val) {
  if (this->verticalScrollBar()->maximum() != val) {
    return false;
  }
  if (not hasNextImgs()) {
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

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  FolderPreviewHTML previewer;
  previewer.show();
  previewer("E:/115/fin/1009/ActiveDuty - Damien Dominates Private Evans - Damien King & Brandon Evans BB 1080p");
  return a.exec();
}
#endif
