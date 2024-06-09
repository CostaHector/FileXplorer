#ifndef REDUNDANTIMAGEFINDER_H
#define REDUNDANTIMAGEFINDER_H

#include <QAction>
#include <QFile>
#include <QMainWindow>
#include <QSet>
#include <QString>

class QWidget;
class RedundantImageModel;
class QListView;
class QTableView;
class QToolBar;

struct REDUNDANT_IMG_INFO {
  QString filePath;
  qint64 size;
  QString md5;
};

typedef QList<REDUNDANT_IMG_INFO> REDUNDANT_IMG_BUNCH;

class RedundantImageFinder : public QMainWindow {
 public:
  explicit RedundantImageFinder(QWidget* parent = nullptr);
  ~RedundantImageFinder();

  void ReadLocalCharacteristicLib(const QString& libPath);
  void LearnCommonImageCharacteristic(const QString& folderPath);

  void operator()(const QString& folderPath);

  void subscribe();

  void RecycleSelection();

 private:
  QString GetRedunPath() const;
  void UpdateDisplayWhenRecycled();

  static QSet<qint64> m_commonFileSizeSet;
  static QSet<QString> m_commonFileHash;

  REDUNDANT_IMG_BUNCH m_imgsBunch;

  QFile m_libFi;
  RedundantImageModel* m_imgModel;
  QTableView* m_list;

  QAction* RECYLE_NOW{new QAction{QIcon(":/themes/MOVE_TO_TRASH_BIN"), "Recycle selections", this}};
  QAction* RECYCLE_EMPTY_IMAGE{new QAction{"Also empty image", this}};
  QAction* OPEN_REDUNDANT_IMAGES_FOLDER{new QAction{"Open redun folder", this}};
  QToolBar* m_toolBar;
  static bool ALSO_RECYCLE_EMPTY_IMAGE;
};

#endif  // REDUNDANTIMAGEFINDER_H
