#ifndef CASTDBLISTVIEW_H
#define CASTDBLISTVIEW_H

#include "CustomListView.h"
#include "CastDbModel.h"

class CastCardDelegate;
class CastDBListView : public CustomListView {
public:
  explicit CastDBListView(CastDbModel* castDbModel_, QItemSelectionModel* selectionModel_, QWidget* parent = nullptr);
#ifdef RUNNING_UNIT_TESTS
  CastCardDelegate* GetCardDelegate() { return m_castCardDel; }
#endif

protected:
  void initExclusivePreferenceSetting() override;

private slots:
  void onIconSizeChanged(const QSize& iconSize);

private:
  CastCardDelegate* m_castCardDel{nullptr};
};

#endif // CASTDBLISTVIEW_H
