#ifndef ArchiveFilesPreview_H
#define ArchiveFilesPreview_H
#include "ArchiveFiles.h"
#include "ArchiverModel.h"
#include "CustomTableView.h"
#include "ThumbnailImageViewer.h"
#include <QMainWindow>
#include <QSplitter>
#include <QSlider>

class Archiver : public QMainWindow {
 public:
  explicit Archiver(QWidget* parent = nullptr);
  bool operator()(const QString& qzPath);

  bool onSelectNewItemRow(const QModelIndex& current, const QModelIndex& previous);

  void ChangeWindowTitle(const QString& name, const int& Bytes);

  void UpdateWindowsSize();
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;
  void onSilderChangedUpdateImageScaledIndex(int scaleIndex);
  void setSliderValueAndLabelDisplayText(int scaleIndex);

 private:
  void subscribe();

  QSplitter* m_splitter{nullptr};
  CustomTableView* m_itemsTable{nullptr};
  ArchiverModel* m_archiverModel{nullptr};
  ThumbnailImageViewer* m_thumbnailViewer{nullptr};

  QLabel* m_ImageSizeHint{nullptr};
  QSlider* m_ImageSizeScale{nullptr};
};

#endif  // ArchiveFilesPreview_H
