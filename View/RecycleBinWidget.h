#ifndef RECYCLEBINWIDGET_H
#define RECYCLEBINWIDGET_H

#include <QAction>
#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include "View/CustomTableView.h"

#include <QMessageBox>

#include <QDir>
#include <QFileInfo>

// #include <fileapi.h>
#include <QFileIconProvider>
#include <QModelIndexList>
#include <QSqlError>
#include <QSqlTableModel>

class TrashbinModel : public QSqlTableModel {
 public:
  explicit TrashbinModel(QObject* parent = nullptr, QSqlDatabase con = QSqlDatabase()) : QSqlTableModel{parent, con} {
    setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
  }

  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Vertical) {
      if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
      }
    }
    return QSqlTableModel::headerData(section, orientation, role);
  }

  QString fileName(const QModelIndex& curIndex) const;
  QString oldLocationFolder(const QModelIndex& curIndex) const;
  QString tempLocationFolder(const QModelIndex& curIndex) const;
  QString fullPath(const QModelIndex& curIndex) const;

  void refresh();
 private:
  QFileIconProvider m_iconProvider;
};

class RecycleBinWidget : public QMainWindow {
 public:
  RecycleBinWidget(QWidget* parent = nullptr);

  void ChangeWindowIcon();

  void closeEvent(QCloseEvent* event) override;

 protected:
  void onSubmitAndRefresh();
  bool onDeleteAll();
  bool onRestoreAll();
  bool onDeleteSelection();
  bool onRestoreSelection();

 private:
  QModelIndexList GetAllRows() const {
    const int rowCount = m_model->rowCount();
    QModelIndexList indexes;
    indexes.reserve(rowCount);
    for (int i = 0; i < rowCount; ++i) {
      indexes.append(m_model->index(i, 0));
    }
    return indexes;
  }

  void subscribe();
  bool queryForConfirm(const QString& confirmTitle, const QString& confirmMsg) {
    return QMessageBox::question(this, confirmTitle, confirmMsg, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Yes,
                                 QMessageBox::StandardButton::No) == QMessageBox::StandardButton::Yes;
  }
  bool deleteItems(const QModelIndexList& indexes);
  bool restoreItems(const QModelIndexList& indexes);

  struct SubdirItemsFirst {
    QStringList names;
    QStringList oldPaths;
    QStringList tempPaths;
  };

  static SubdirItemsFirst SubdirItemsFirstSort(const QStringList& names, const QStringList& tmpPaths, const QStringList& oldpaths);
  static SubdirItemsFirst SubdirItemsFirstSort(const TrashbinModel* model, const QModelIndexList& indexes);

  bool createRecycleTable();


  QMenu* m_recycleBinMenu;
  QToolBar* m_recycleBinToolBar;

  TrashbinModel* m_model{nullptr};
  CustomTableView* m_view{nullptr};
};

#endif  // RECYCLEBINWIDGET_H
