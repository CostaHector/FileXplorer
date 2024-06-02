#ifndef ArchiveFilesPreview_H
#define ArchiveFilesPreview_H

#include <QTextEdit>
class ArchiveFilesPreview : public QTextEdit {
 public:
  explicit ArchiveFilesPreview(QWidget* parent = nullptr);
  bool operator()(const QString& qzPath);
  auto sizeHint() const -> QSize override { return QSize(1024, 768); }
};

#endif  // ArchiveFilesPreview_H
