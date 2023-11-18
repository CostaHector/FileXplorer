#include "FolderPreviewHTML.h"
#include "PublicVariable.h"

#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>

const QString FolderPreviewHTML::HTML_IMG_TEMPLATE = "<img src=\"%1\" alt=\"%2\" width=\"%3\"><br/>\n";

QString FolderPreviewHTML::InsertImgs(const QString& dirPath) {
  QString imgSrc;
  QDir dir(dirPath, {}, QDir::SortFlag::Name, QDir::Filter::Files);
  if (not dir.exists()) {
    return imgSrc;
  }
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  m_imgsLst = dir.entryList();
  for (int i = 0; i < m_firstSightImgCnt and i < m_imgsLst.size(); ++i) {
    const QString& imgName = m_imgsLst[i];
    imgSrc += HTML_IMG_TEMPLATE.arg(dir.absoluteFilePath(imgName)).arg(imgName).arg(600);
  }
  return imgSrc;
}

bool FolderPreviewHTML::operator()(const QString& path) {
  QFileInfo fi(path);
  m_scrollAtEndBefore = false;
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

bool FolderPreviewHTML::ShowAllImages(const int val) {
  if (m_scrollAtEndBefore or this->verticalScrollBar()->maximum() != val) {
    return false;
  }
  m_scrollAtEndBefore = true;
  QDir dir(dirPath);
  QString insertHtmlSrc;
  for (int i = m_firstSightImgCnt + 1; i < m_imgsLst.size(); ++i) {
    const QString& imgName = m_imgsLst[i];
    insertHtmlSrc += HTML_IMG_TEMPLATE.arg(dir.absoluteFilePath(imgName)).arg(imgName).arg(600);
  }
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
