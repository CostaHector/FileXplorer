#ifndef DUPLICATEIMAGESTABLE_H
#define DUPLICATEIMAGESTABLE_H

#include "CustomTableView.h"
#include "DuplicateImagesModel.h"
#include <QSortFilterProxyModel>

class DuplicateImagesTable : public CustomTableView {
  Q_OBJECT
public:
  explicit DuplicateImagesTable(QWidget* parent = nullptr);

  bool operator()(const QString& folderPath);

private:
  void subscribe();
  bool onFindByChanged(DuplicateImageDetectionCriteria::DICriteriaE findBy);
  bool onIncludeEmptyImgChanged(bool bInclude);
  QString GetWinTitle() const;

  bool onOpenImageDoubleClicked(const QModelIndex& proxyClickedIndex) const;
  int RecycleSelection();
  bool SelectRowsToDelete();

  QAction* mSelectSameHashRows{nullptr};
  DuplicateImagesModel* m_imgModel{nullptr};
  QSortFilterProxyModel* m_imgProxy{nullptr};
};
#endif // DUPLICATEIMAGESTABLE_H
