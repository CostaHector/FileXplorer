#ifndef SEARCHPROXYMODEL_H
#define SEARCHPROXYMODEL_H

#include <QComboBox>
#include <QDebug>
#include <QSortFilterProxyModel>

class SearchProxyModel : public QSortFilterProxyModel {
 public:
  SearchProxyModel(QObject* parent = nullptr);

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override{ return sourceModel()->headerData(section, orientation, role); }

  auto onSearchModeChanged(const QString& searchMode) -> void;
  auto startFilter(const QString& searchText) -> void;

  /*
        # def filterAcceptsRow(self, source_row: int, source_parent: QModelIndex) -> bool:
        #     expr: QRegularExpression = self.filterRegularExpression()
        #     txt = self.sourceModel().data(self.sourceModel().index(source_row, 0, source_parent), Qt::DisplayRole)
        #     return expr.match(txt).hasMatch()
        #     if not self.m_ser:
        #         return True
        #     return self.m_ser in self.sourceModel().data(self.sourceModel().index(source_row, 0, source_parent), Qt::DisplayRole)
        # def setSearchString(self, ser: str):
        #     self.m_ser = ser
        #     self.invalidateFilter()
  */

 private:
  QString m_lastTimeFilterStr;
  QString m_searchMode;
};
#endif  // SEARCHPROXYMODEL_H
