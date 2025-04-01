#ifndef FLOATINGPREVIEW_H
#define FLOATINGPREVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QListView>
#include <QVBoxLayout>
#include "Tools/QAbstractListModelPub.h"

class ImgsModel : public QAbstractListModelPub {
 public:
  explicit ImgsModel(QObject* object = nullptr)  //
      : QAbstractListModelPub{object} {}
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mImgsLst.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  void UpdateImgs(const QStringList& newImgsLst);
  QString filePath(const QModelIndex& index) const;
 private:
  QStringList mImgsLst;
};

class FloatingPreview: public QWidget {
 public:
  FloatingPreview(QWidget* parent=nullptr);
  void ReadSettings();
  void SaveSettings();
  static QPushButton* CreateBtn(const QString& tag, QWidget* parent);

  bool operator()(const QString& pth); // file system
  bool operator()(const QString& name, const QString& pth); // scene
  bool operator()(const QString& name, const QStringList& imgPthLst); // scene
  bool NeedUpdate(const QString& lastName) const;

  void subscribe();

 private:
  bool onImgBtnClicked(bool checked);
  void on_cellDoubleClicked(const QModelIndex& clickedIndex) const;

  QVBoxLayout* mVLo{nullptr};
  QPushButton* mImgBtn{nullptr};
  QPushButton* mVidsBtn{nullptr};
  QPushButton* mOthersBtn{nullptr};
  QListView* mImgTv{nullptr};
  ImgsModel* mImgModel{nullptr};

  QString mLastName;
};

#endif  // FLOATINGPREVIEW_H
