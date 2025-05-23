#include "PreviewBrowser.h"
#include "public/PublicVariable.h"

#include <QScrollBar>
#include <QTextCursor>
#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>

constexpr int PreviewBrowser::SHOW_IMGS_CNT_LIST[];
constexpr int PreviewBrowser::N_SHOW_IMGS_CNT_LIST;

PreviewBrowser::PreviewBrowser(QWidget* parent)  //
    : ClickableTextBrowser{parent}, m_parentDocker(parent) {
  subscribe();
}

bool PreviewBrowser::operator()(const QString& path) {
  setHtml("");  // release memory occupied before
  dirPath = QDir::fromNativeSeparators(path);
  m_curImgCntIndex = 0;
  m_imgsLst.clear();

  QFileInfo fi(dirPath);
  int vidCnt = 0;
  if (fi.isDir()) {
    QDir vidDir(dirPath, {}, QDir::NoSort, QDir::Filter::Files);
    vidDir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
    vidCnt = vidDir.entryList().size();
    m_imgsLst = InitImgsList(dirPath);
  }
  const QString& headLine = fi.isDir() ? VID_LINK_TEMPLATE.arg(fi.absoluteFilePath()).arg(fi.fileName()) : HTML_H1_TEMPLATE.arg(fi.absoluteFilePath()).arg(fi.fileName());
  setDockerWindowTitle(vidCnt);

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

void PreviewBrowser::subscribe() {
  connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &PreviewBrowser::ShowRemainImages);
}

QStringList PreviewBrowser::InitImgsList(const QString& dirPath) const {
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

bool PreviewBrowser::hasNextImgs() const {
  return (not m_imgsLst.isEmpty()) and m_curImgCntIndex + 1 < N_SHOW_IMGS_CNT_LIST;
}

QString PreviewBrowser::nextImgsHTMLSrc() {
  QString imgSrc;
  const QDir dir(dirPath);
  for (int i = SHOW_IMGS_CNT_LIST[m_curImgCntIndex]; i < m_imgsLst.size() and i < SHOW_IMGS_CNT_LIST[m_curImgCntIndex + 1]; ++i) {
    const QString& imgName = m_imgsLst[i];
    imgSrc += HTML_IMG_TEMPLATE.arg(dir.absoluteFilePath(imgName)).arg(imgName).arg(HTML_IMG_FIXED_WIDTH);
  }
  ++m_curImgCntIndex;
  return imgSrc;
}

bool PreviewBrowser::ShowRemainImages(const int val) {
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
