#ifndef ITEMVIEW_H
#define ITEMVIEW_H
#include "CustomListView.h"
#include "FloatingModels.h"

class ItemView : public CustomListView {
  Q_OBJECT
 public:
  explicit ItemView(const QString& itemViewName, QWidget* parent = nullptr);
  bool SetCurrentModel(FloatingModels* mdl);

 private slots:
  bool onCellDoubleClicked(const QModelIndex& clickedIndex) const;
  bool onPlayCurrentIndex() const;
  bool onRecycleSelections() const;
  int onRenameSelectedItems();

 private:
  void subscribe();
  void initExclusivePreferenceSetting();

  FloatingModels* mModels{nullptr};
  QAction* _PLAY_ITEM{nullptr};
  QAction* _RENAME_SCENE_RELATED_FILES_NUMERIZE{nullptr};
  QAction* _RECYCLE_ITEM{nullptr};
};

#endif // ITEMVIEW_H
