#ifndef VIDSPLAYLISTVIEW_H
#define VIDSPLAYLISTVIEW_H

#include <QMenu>
#include <QStandardItemModel>

#include "VidModel.h"
#include "CustomListView.h"

class VidsPlayListView : public CustomListView {
 public:
  VidsPlayListView(VidModel* model_, QWidget* parent = nullptr);
  void subscribe();

  int currentRow() const { return currentIndex().row(); }
  int count() const { return m_vidModel->rowCount(); };
  void setCurrentRow(int r) { setCurrentIndex(m_vidModel->index(r, 0)); }

  QString filePath(int row) const { return m_vidModel->filePath(row); }
  QString filePath(const QModelIndex& ind) const { return m_vidModel->filePath(ind); }

  QString currentFilePath() const { return filePath(currentRow()); }

  void onRevealInSystemExplorer();

  int appendToPlayList(const QStringList& fileAbsPathList);

 private:
  VidModel* m_vidModel;
  QMenu* m_vidMenu;
};

#endif  // VIDSPLAYLISTVIEW_H
