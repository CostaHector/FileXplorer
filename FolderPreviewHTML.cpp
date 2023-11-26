#include "FolderPreviewHTML.h"
#include "Actions/FileBasicOperationsActions.h"
#include "PublicVariable.h"

#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>

const QString FolderPreviewHTML::HTML_IMG_TEMPLATE = "<img src=\"%1\" alt=\"%2\" width=\"%3\"><br/>\n";
constexpr int FolderPreviewHTML::SHOW_IMGS_CNT_LIST[];
constexpr int FolderPreviewHTML::N_SHOW_IMGS_CNT_LIST;

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
  QString headLine = QString("<a href=\"file:///%1\">%2</a>").arg(fi.absoluteFilePath()).arg(fi.fileName());
  QString imgSrc;
  if (fi.isDir()) {
    QDir vidDir(path, {}, QDir::NoSort, QDir::Filter::Files);
    vidDir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
    vidCnt = vidDir.entryList().size();
    imgSrc = InsertImgs(path);
    headLine = QString("<a href=\"file:///%1\">&#9654;%2</a>").arg(fi.absoluteFilePath()).arg(fi.fileName());
  }

  if (m_parent) {
    m_parent->setWindowTitle(QString::number(vidCnt) + "vid(s)");
  }

  QString htmlSrc;
  htmlSrc += "<!DOCTYPE html>\n<html>\n";
  htmlSrc += "<head><title>Title</title></head>\n";
  htmlSrc += QString(
                 "<body align=\"center\">\n"
                 "<h1>%1</h1>"
                 "<br/>\n")
                 .arg(headLine);
  htmlSrc += std::move(imgSrc);
  htmlSrc += "</body>\n</html>\n";
  setHtml(htmlSrc);
  return true;
}

void FolderPreviewHTML::subscribe() {
  connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &FolderPreviewHTML::ShowAllImages);
  connect(this, &QTextBrowser::anchorClicked, this, &FolderPreviewHTML::onAnchorClicked);
}

QSize FolderPreviewHTML::sizeHint() const {
  auto w = PreferenceSettings().value("dockerHtmlWidth", DOCKER_DEFAULT_SIZE.width()).toInt();
  auto h = PreferenceSettings().value("dockerHtmlHeight", DOCKER_DEFAULT_SIZE.height()).toInt();
  return QSize(w, h);
}

QString FolderPreviewHTML::InsertImgs(const QString& dirPath) {
  QString imgSrc;
  QDir dir(dirPath);
  if (not dir.exists()) {
    return imgSrc;
  }
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  m_imgsLst = dir.entryList(QDir::Filter::Files);
  static const auto imgHumanSorter = [](const QString& lhs, const QString& rhs) -> bool {
    if (lhs.size() != rhs.size()) {
      return lhs.size() < rhs.size();
    }
    return lhs < rhs;
  };
  std::sort(m_imgsLst.begin(), m_imgsLst.end(), imgHumanSorter);
  // images human sort 0 < 1 < ... < 9 < 10. not in alphabeit

  for (int i = SHOW_IMGS_CNT_LIST[m_curImgCntIndex]; i < m_imgsLst.size() and i < SHOW_IMGS_CNT_LIST[m_curImgCntIndex + 1]; ++i) {
    const QString& imgName = m_imgsLst[i];
    imgSrc += HTML_IMG_TEMPLATE.arg(dir.absoluteFilePath(imgName)).arg(imgName).arg(600);
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

bool FolderPreviewHTML::ShowAllImages(const int val) {
  if (m_curImgCntIndex >= N_SHOW_IMGS_CNT_LIST - 1 or this->verticalScrollBar()->maximum() != val) {
    return false;
  }

  QDir dir(dirPath);
  QString insertHtmlSrc;
  for (int i = SHOW_IMGS_CNT_LIST[m_curImgCntIndex]; i < m_imgsLst.size() and i < SHOW_IMGS_CNT_LIST[m_curImgCntIndex + 1]; ++i) {
    const QString& imgName = m_imgsLst[i];
    insertHtmlSrc += HTML_IMG_TEMPLATE.arg(dir.absoluteFilePath(imgName)).arg(imgName).arg(600);
  }
  ++m_curImgCntIndex;

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
