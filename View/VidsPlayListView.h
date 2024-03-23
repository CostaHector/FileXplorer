#ifndef VIDSPLAYLISTVIEW_H
#define VIDSPLAYLISTVIEW_H

#include <QAbstractListModel>
#include <QListView>

#include <QMenu>
#include <QStandardItemModel>
#include "Model/VidModel.h"


class VidsPlayListView : public QListView {
 public:
  VidsPlayListView(VidModel* model_, QWidget* parent = nullptr);
  auto InitViewSettings() -> void;
  void subscribe();

  void contextMenuEvent(QContextMenuEvent* event);

  int currentRow() const { return currentIndex().row(); }
  int count() const { return m_vidModel->rowCount(); };
  void setCurrentRow(int r) { setCurrentIndex(m_vidModel->index(r, 0)); }

  QString filePath(int row) const { return m_vidModel->data(m_vidModel->index(row, 0), Qt::DisplayRole).toString(); }
  QString filePath(const QModelIndex& ind) const { return m_vidModel->data(ind, Qt::DisplayRole).toString(); }

  QString currentFilePath() const { return filePath(currentRow()); }

  void onRevealInSystemExplorer();

  int appendToPlayList(const QStringList& fileAbsPathList);
  int onRecycleSelectedItems();


 private:
  VidModel* m_vidModel;
  QMenu* m_vidMenu;
};

#endif  // VIDSPLAYLISTVIEW_H
