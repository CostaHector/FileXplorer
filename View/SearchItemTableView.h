#ifndef SEARCHITEMTABLEVIEW_H
#define SEARCHITEMTABLEVIEW_H

#include "PublicVariable.h"
#include "SearchItemModel.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QTableView>
#include <QToolBar>

#include <QSortFilterProxyModel>

class MySortFilterProxyModel : public QSortFilterProxyModel {
 public:
  explicit MySortFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
    return sourceModel()->data(index0, Qt::ItemDataRole::DisplayRole).toString().contains(filterRegExp());
  }
};

class SearchItemTableView : public QWidget {
 public:
  explicit SearchItemTableView(const QString& rootPath = "./", QWidget* parent = nullptr);

  QSize sizeHint() const override { return QSize(1024, 768); }

  void onRootPathChanged() {
    PreferenceSettings().setValue(MemoryKey::SEARCH_MODEL_ROOT_PATH.name, m_rootPathLE->text());
    m_searchModel->ChangeRootPathOrFilter(m_rootPathLE->text(), String2DirFilters(m_dirFilterLE->text()), m_typeFilterStr.split('|'), false);
  }

  void onNameFilterChanged() {
    m_proxyModel->setFilterRegExp(QRegExp(m_nameFilterLE->text(), Qt::CaseInsensitive));
    PreferenceSettings().setValue(MemoryKey::SEARCH_MODEL_NAME_FILTER.name, m_nameFilterLE->text());
  }

  void onRowDoubleClicked(QModelIndex index);

  void onDirFilterChanged(QAction* dirFilterActTriggered);

  void onTypeFilterChanged();

  void UpdateActionChecked(const QString& dirFilterStr) {
    _FILES->setChecked(dirFilterStr.contains("Files"));
    _DIRS->setChecked(dirFilterStr.contains("Dirs"));
  }

 private:
  QString m_rootPath;

  QTableView* m_tv;

  QAction* _FILES;
  QAction* _DIRS;
  QActionGroup* DIR_FILTER_AG;

  QAction* _VID;
  QAction* _IMG;
  QAction* _DOC;
  QAction* _PLAIN_TEXT;
  QAction* _CODE;
  QActionGroup* FREQ_TYPE_FILTER;
  QToolBar* m_filterTB;

  QComboBox* m_rootPathCB;
  QLineEdit* m_rootPathLE;

  QComboBox* m_dirFilterCB;
  QLineEdit* m_dirFilterLE;

  QComboBox* m_typeFilterCB;
  QLineEdit* m_typeFilterLE;

  QComboBox* m_nameFilterCB;
  QLineEdit* m_nameFilterLE;

  QString m_dirFilterStr;
  QString m_typeFilterStr;
  QString m_nameFilterStr;

  QSortFilterProxyModel* m_proxyModel;
  SearchItemModel* m_searchModel;
};

#endif  // SEARCHITEMTABLEVIEW_H
