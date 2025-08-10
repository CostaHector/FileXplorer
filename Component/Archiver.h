#ifndef ArchiveFilesPreview_H
#define ArchiveFilesPreview_H
#include "ArchiveFiles.h"
class QLabel;
class CustomTableView;
class ArchiverModel;
class QSplitter;
class QCheckBox;
class QMenu;
class QSlider;

#include <QMainWindow>

class Archiver : public QMainWindow {
 public:
  explicit Archiver(QWidget* parent = nullptr);
  bool operator()(const QString& qzPath);

  bool onNewRow(const QModelIndex &current, const QModelIndex &previous);

  void ChangeWindowTitle(const QString& name, const int& Bytes);

  void UpdateWindowsSize();
  void showEvent(QShowEvent *event) override;
  void closeEvent(QCloseEvent* event) override;

 private:
  void subscribe();

  QCheckBox* m_showFullListAnyway;
  QSlider* m_archiveSizeScale;
  QMenu* m_archiveMenu;
  QSplitter* m_splitter;
  CustomTableView* m_itemsTable;
  ArchiverModel* m_archiverModel;
  QLabel* m_thumbnailViewer;

  ArchiveFiles m_af;
};

#endif  // ArchiveFilesPreview_H
