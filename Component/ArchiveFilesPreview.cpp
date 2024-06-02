#include "ArchiveFilesPreview.h"
#include "Tools/ArchiveFiles.h"

#include <QIcon>
#include <QTextDocument>

ArchiveFilesPreview::ArchiveFilesPreview(QWidget* parent) : QTextEdit{parent} {
  setWindowTitle("ArchiveFilesPreview");
  setWindowIcon(QIcon(":/themes/COMPRESS_ITEM"));
}

bool ArchiveFilesPreview::operator()(const QString& qzPath) {
  clear();

  if (not qzPath.toLower().endsWith(".qz")) {
    qWarning("Not a qz file");
    setWindowTitle(QString("ArchiveFilesPreview | [%1] not a qz file").arg(qzPath));
    return false;
  }

  ArchiveFiles af(qzPath, ArchiveFiles::ONLY_IMAGE);
  static constexpr int K = 4;
  // Todo, loading more button
  QStringList paths;
  QList<QByteArray> datas;
  paths.reserve(K);
  datas.reserve(K);

  if (not af.ReadFirstKItemsOut(K, paths, datas)) {
    setWindowTitle(QString("ArchiveFilesPreview | [%1] read failed").arg(qzPath));
    return false;
  }
  setWindowTitle(QString("ArchiveFilesPreview | %1 item(s)").arg(paths.size()));

  QTextDocument* textDocument = document();
  for (int i = 0; i < paths.size(); ++i) {
    QUrl url{paths[i]};
    textDocument->addResource(QTextDocument::ImageResource, url, QVariant(datas[i]));
    QTextImageFormat imageFormat;
    imageFormat.setName(url.toString());

    QTextCursor cursor = textCursor();
    cursor.insertText(url.toString());
    cursor.insertText("\n");
    cursor.insertImage(imageFormat);
    cursor.insertText("\n");
  }
  return true;
}
// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  ArchiveFilesPreview afPreview;
  afPreview.show();
  afPreview.operator()("E:/py/1/achieve.qz");
  return a.exec();
}
#endif
