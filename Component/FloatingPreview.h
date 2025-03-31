#ifndef FLOATINGPREVIEW_H
#define FLOATINGPREVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>
#include "Tools/QAbstractTableModelPub.h"

class ImgsModel : public QAbstractTableModelPub {
 public:
  explicit ImgsModel(QObject* object = nullptr)  //
      : QAbstractTableModelPub{object} {}
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return 1; }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return mImgsLst.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  void UpdateImgs(const QStringList& newImgsLst);
 private:
  QStringList mImgsLst;
};

class FloatingPreview: public QWidget {
 public:
  FloatingPreview(QWidget* parent=nullptr);
  static QPushButton* CreateBtn(const QString& tag, QWidget* parent);

  bool operator()(const QString& pth); // file system
  bool operator()(const QString& name, const QString& pth); // scene
  bool operator()(const QString& name, const QStringList& imgPthLst); // scene
  QSize sizeHint() const { return QSize{1080, 480};}
  bool NeedUpdate(const QString& lastName) const;

  void subscribe();

 private:
  bool onImgBtnClicked(bool checked);

  QVBoxLayout* mVLo{nullptr};
  QPushButton* mImgBtn{nullptr};
  QPushButton* mVidsBtn{nullptr};
  QPushButton* mOthersBtn{nullptr};
  QTableView* mImgTv{nullptr};
  ImgsModel* mImgModel{nullptr};

  QString mLastName;
};

#endif  // FLOATINGPREVIEW_H
