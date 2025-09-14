#ifndef TYPEFILTERBUTTON_H
#define TYPEFILTERBUTTON_H

#include <QToolButton>
#include "ModelFilterActions.h"

class TypeFilterButton : public QToolButton {
  Q_OBJECT
public:
  explicit TypeFilterButton(ModelFilterE modelE, QWidget* parent = nullptr);
  ~TypeFilterButton();
  void EmitFilterChanged(const QAction* pAct);
  void EmitFilterChangedWhenResetFilters();
  void EmitNameFilterDisablesChanged(bool bGrayUnpass);
  void EmitIncludingSubdirectoryChanged(const bool bInclude);

  QDir::Filters curDirFilters() const {return mFilterHelper.getCurDirFilters();}
  bool curGrayOrHideUnpassItem() const {return mFilterHelper.getCurGrayOrHideUpassItem();}
  QDirIterator::IteratorFlag curIteratorFlag() const {return mFilterHelper.getCurIteratorFlag();}

signals:
  void filterChanged(QDir::Filters filters);
  void nameFilterDisablesChanged(bool bGrayOrHide);
  void includingSubdirectoryChanged(QDirIterator::IteratorFlag iteratorFlg);

private:
  ModelFilterActions& mFilterHelper;
};

#endif  // TYPEFILTERBUTTON_H
