#ifndef STYLESHEETTREEMODEL_H
#define STYLESHEETTREEMODEL_H

#include "StyleItemData.h"
#include "QAbstractTreeModelPub.h"
#include "StyleEnum.h"
#include <QSet>

extern template class QAbstractTreeModelPub<StyleTreeNode>;

class StyleSheetTreeModel : public QAbstractTreeModelPub<StyleTreeNode> {
  Q_OBJECT
public:
  explicit StyleSheetTreeModel(QObject* parent = nullptr);
  ~StyleSheetTreeModel() = default;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  int ClearNewValues(const QModelIndexList& indexes);
  int recoverNewValuesToBackup(const QModelIndexList& indexes);
  QVariantHash ApplyNewValues(const QModelIndexList& indexes);
  void onInstantApplyChanged(bool bInstantApply) { m_bInstantApply = bInstantApply; }

signals:
  void requestApplyChanges(const QString& cfgKey, const QVariant& value);

private:
  bool initFontRelated(std::unique_ptr<StyleTreeNode>& pRoot) const;
  bool initColorRelated(std::unique_ptr<StyleTreeNode>& pRoot, Style::StyleSheetE styleE) const;
  void editCell(const QModelIndex& ind, bool bSucceed);
  void editCellFailed(const QModelIndex& failedInd);
  void editCellSucceed(const QModelIndex& okInd);
  bool editCellEraseIndex(const QModelIndex& okInd);
  QSet<QModelIndex> mEditFailedCells;

  bool m_bInstantApply{false};
};

#endif // STYLESHEETTREEMODEL_H
