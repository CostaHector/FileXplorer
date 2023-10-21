#include "FolderPreviewHTML.h"

#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QTextStream>
#include "PublicVariable.h"

FolderPreviewHTML::FolderPreviewHTML() {}

auto FolderPreviewHTML::HtmlContentRelativePath2AbsPath(QString relPathSrc, const QString& prePath) -> QString {
  QString oldStr("<img src=\"");
  return relPathSrc.replace(oldStr, QString("%1%2/").arg(oldStr).arg(prePath));
}

bool FolderPreviewHTML::operator()(const QString& path) {
  QFileInfo fi(path);
  if (not fi.exists()) {
    qDebug("Path[%s] Not Exist", path.toStdString().c_str());
    return false;
  }
  QString htmlFilePath(fi.absoluteFilePath());
  if (fi.isDir()) {
    QDir dir(fi.absoluteFilePath());
    for (const QString& name : dir.entryList(QDir::Files)) {
      if (name.endsWith(".html", Qt::CaseInsensitive)) {
        htmlFilePath = dir.absoluteFilePath(name);
        break;
      }
    }
  }
  if (not htmlFilePath.endsWith(".html", Qt::CaseInsensitive)) {
    setHtml(htmlFilePath);
    return true;
  }
  const QString& relHtmlSrc = TextReader(htmlFilePath);
  const QString& prePath = QFileInfo(htmlFilePath).absolutePath();
  const QString& absHtmlSrc = HtmlContentRelativePath2AbsPath(relHtmlSrc, prePath);
  setHtml(absHtmlSrc);
  return true;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  FolderPreviewHTML previewer;
  previewer.show();
  previewer("E:\\3\\0719\\Alexander Pictures - Part 2\\.index.html");
  return a.exec();
}
#endif
