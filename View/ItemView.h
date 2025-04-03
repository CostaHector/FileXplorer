#ifndef ITEMVIEW_H
#define ITEMVIEW_H
#include "CustomListView.h"
#include "Model/FloatingModels.h"

class ItemView : public CustomListView {
 public:
  explicit ItemView(const QString& itemViewName, QWidget* parent = nullptr);
  void SetCurrentModel(FloatingModels* mdl) {
    setModel(mdl);
    mModels = mdl;
  }
  void subscribe();
  void onCellDoubleClicked(const QModelIndex& clickedIndex) const;
  void onOrientationChange(const QAction* pOrientation);

 private:
  FloatingModels* mModels{nullptr};
  QAction* _PLAY_ITEM{nullptr};
  QAction *_ORIENTATION_LEFT_TO_RIGHT{nullptr}, *_ORIENTATION_TOP_TO_BOTTOM{nullptr};
  QActionGroup* _ORIENTATION_GRP{nullptr};
  QMenu* mItemMenu{nullptr};
};

#endif // ITEMVIEW_H
