#include "ImagesInFolderBrowser.h"
#include "PublicVariable.h"
#include "CastBrowserHelper.h"
#include "StyleSheet.h"
#include "StringTool.h"

#include <QScrollBar>
#include <QTextCursor>
#include <QDir>
#include <QFileInfo>
#include <QPixmapCache>

constexpr int ImagesInFolderBrowser::SHOW_IMGS_CNT_LIST[];
constexpr int ImagesInFolderBrowser::N_SHOW_IMGS_CNT_LIST;

ImagesInFolderBrowser::ImagesInFolderBrowser(QWidget* parent)  //
  : ClickableTextBrowser{parent}, m_parentDocker{parent} {
  subscribe();
}

void ImagesInFolderBrowser::wheelEvent(QWheelEvent *event) {
  if (event->modifiers() == Qt::NoModifier) {
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
      if (numDegrees.y() / 15 < 0) {
        ShowRemainImages(verticalScrollBar()->value());
      }
    }
  }
  ClickableTextBrowser::wheelEvent(event);
}

bool ImagesInFolderBrowser::operator()(const QString& path) {
  QPixmapCache::clear(); // release memory occupied before
  m_curImgCntIndex = 0;
  m_imgsLst.clear();
  const QFileInfo fi{path};
  const QString fileName = fi.fileName();
  if (fi.isDir()) {
    m_dirPath = path;
    m_imgsLst += InitImgsList(m_dirPath);
  } else if (TYPE_FILTER::IMAGE_TYPE_SET.contains("*." + fi.suffix().toLower())) {
    m_dirPath = fi.absolutePath();
    m_imgsLst.push_back(fileName);
  } else {
    m_dirPath = "";
  }

  QString htmlSrc;
  htmlSrc += R"(<html><head><title>Images In Folder Browser</title></head>)" "\n";
  htmlSrc += R"(<body align="center">)" "\n";
  htmlSrc += R"(<h1>)";
  htmlSrc += CastBrowserHelper::HTML_H1_TEMPLATE.arg(path).arg(fileName);
  htmlSrc += R"(</h1>)";
  htmlSrc += R"(<br/>)";
  if (hasNextImgs()) {
    htmlSrc += nextImgsHTMLSrc();
  }
  htmlSrc += R"(</body></html>)";
  setHtml(htmlSrc);
  return true;
}

void ImagesInFolderBrowser::subscribe() {
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &ImagesInFolderBrowser::ShowRemainImages);
}

QStringList ImagesInFolderBrowser::InitImgsList(const QString& dirPath) const {
  const QDir dir{dirPath};
  if (!dir.exists()) {
    return {};
  }
  QStringList imgsLst = dir.entryList(TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDir::SortFlag::Name);
  StringTool::ImgsSortNameLengthFirst(imgsLst);
  return imgsLst;
}

bool ImagesInFolderBrowser::hasNextImgs() const {
  return (!m_imgsLst.isEmpty()) //
         && m_curImgCntIndex + 1 < N_SHOW_IMGS_CNT_LIST //
         && m_imgsLst.size() > SHOW_IMGS_CNT_LIST[m_curImgCntIndex]; //
}

QString ImagesInFolderBrowser::nextImgsHTMLSrc() {
  QString imgSrc;
  const QDir dir{m_dirPath};
  const auto& ICON_SIZE = iconSize();
  for (int i = SHOW_IMGS_CNT_LIST[m_curImgCntIndex]; i < m_imgsLst.size() && i < SHOW_IMGS_CNT_LIST[m_curImgCntIndex + 1]; ++i) {
    const QString& imgName = m_imgsLst[i];
    imgSrc += CastBrowserHelper::GenerateSingleImageInHtml(dir.absoluteFilePath(imgName), imgName, ICON_SIZE);
    imgSrc += '\n';
  }
  ++m_curImgCntIndex;
  return imgSrc;
}

bool ImagesInFolderBrowser::ShowRemainImages(const int val) {
  if (verticalScrollBar()->maximum() != val) {
    return false;
  }
  if (!hasNextImgs()) {
    return false;
  }
  const QString insertHtmlSrc {nextImgsHTMLSrc()};
  if (insertHtmlSrc.isEmpty()) {
    return true;
  }

  QTextCursor cur = textCursor();
  cur.movePosition(QTextCursor::MoveOperation::End);
  setTextCursor(cur);
  insertHtml(insertHtmlSrc);
  return true;
}
