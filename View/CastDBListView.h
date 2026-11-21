#ifndef CASTDBLISTVIEW_H
#define CASTDBLISTVIEW_H

#include "CustomListView.h"
#include "CastDbModel.h"

class CastCardDelegate;
class CastDBListView : public CustomListView {
public:
  explicit CastDBListView(QWidget* parent = nullptr);
  explicit CastDBListView(CastDbModel* castDbModel_, QWidget* parent = nullptr);

protected:
  void initExclusivePreferenceSetting() override;
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

private slots:
  void onIconSizeChanged(const QSize& iconSize);

private:
  void ReadSettings();

  CastCardDelegate* m_castCardDel{nullptr};
  bool m_geometryRestoredOnce{false};
};

#endif // CASTDBLISTVIEW_H
