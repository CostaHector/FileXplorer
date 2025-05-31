#ifndef MD5WINDOW_H
#define MD5WINDOW_H
#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QStringList>
#include <QToolBar>
#include <QMap>

class MD5Window : public QDialog {
 public:
  explicit MD5Window(const QString& root, const QStringList& items, QWidget* parent = nullptr);
  QSize sizeHint() const override { return QSize(600, 400); }
  void showEvent(QShowEvent *event) override;

  bool onIncrementalCalculateMD5();
  bool onFullCalculateMD5();
  bool onLoadFromMD5Files();
  bool onDumpIntoMD5Files();

 private:
  void subscribe();

  QString m_root;
  QStringList m_items;
  QString m_md5FileName;
  QPlainTextEdit* m_md5TextEdit;
  QDialogButtonBox* m_buttonBox;
  QAction* m_incrementRefresh;
  QAction* m_reloadFromFile;
  QAction* m_dumpMD5IntoFile;
  QAction* m_fullRefresh;
  QToolBar* m_md5InfoTB;

  QMap<QString, QString> m_fileMD5Map;
};

#endif  // MD5WINDOW_H
