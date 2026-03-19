#ifndef ITEMVIEW_H
#define ITEMVIEW_H
#include "CustomListView.h"
#include "FloatingModels.h"

class ItemView : public CustomListView {
  Q_OBJECT
 public:
  explicit ItemView(const QString& itemViewName, QWidget* parent = nullptr);
  bool SetCurrentModel(FloatingModels* mdl);
  void subscribe();
  bool onCellDoubleClicked(const QModelIndex& clickedIndex) const;
  bool onPlayCurrentIndex() const;
  bool onRecycleSelections() const;

 private:
  FloatingModels* mModels{nullptr};
  QAction* _PLAY_ITEM{nullptr};
  QAction* _RECYCLE_ITEM{nullptr};
};

#endif // ITEMVIEW_H
