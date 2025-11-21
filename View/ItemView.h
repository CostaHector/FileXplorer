#ifndef ITEMVIEW_H
#define ITEMVIEW_H
#include "CustomListView.h"
#include "FloatingModels.h"

class ItemView : public CustomListView {
 public:
  explicit ItemView(const QString& itemViewName, QWidget* parent = nullptr);
  void SetCurrentModel(FloatingModels* mdl) {
    setModel(mdl);
    mModels = mdl;
  }
  void subscribe();
  bool onCellDoubleClicked(const QModelIndex& clickedIndex) const;

 private:
  FloatingModels* mModels{nullptr};
  QAction* _PLAY_ITEM{nullptr};
};

#endif // ITEMVIEW_H
