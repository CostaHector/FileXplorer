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

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  int SetFontGeneral(const QFont& newGeneralFont);
  int SetNewColors(const QModelIndexList& indexes, const QString& newColor);
  int ClearNewValues(const QModelIndexList& indexes);
  int RecoverNewValuesToDefault(const QModelIndexList& indexes);
  int RecoverNewValuesToBackup(const QModelIndexList& indexes);
  QVariantHash CollectItemsNeedSeeChange(const QModelIndexList& indexes) const;
  void onLivePreviewSwitchChanged(bool bLivePreview) { m_bLivePreviewSwitch = bLivePreview; }

signals:
  void requestSeeChanges(const QString& cfgKey, const QVariant& value);

private:
  void editCell(const QModelIndex& ind, bool bSucceed);
  bool editCellFailed(const QModelIndex& failedInd);
  bool editCellSucceed(const QModelIndex& okInd);
  bool editCellEraseIndex(const QModelIndex& okInd);
  QSet<QModelIndex> mEditFailedCells;

  bool m_bLivePreviewSwitch{false};
  mutable const StyleTreeNode* mFontGeneralFamilyNode{nullptr};
  mutable const StyleTreeNode* mFontGeneralSizeNode{nullptr};
  mutable const StyleTreeNode* mFontGeneralWeightNode{nullptr};
  mutable const StyleTreeNode* mFontGeneralStyleNode{nullptr};
};

#endif // STYLESHEETTREEMODEL_H
