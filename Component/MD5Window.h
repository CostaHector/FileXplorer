#ifndef MD5WINDOW_H
#define MD5WINDOW_H
#include <QDialog>
#include <QToolBar>
#include <QPlainTextEdit>
#include <QStringList>
#include <QMap>
#include <QVBoxLayout>

class MD5Window : public QDialog {
 public:
  explicit MD5Window(QWidget* parent = nullptr);
  void ReadSetting();
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;
  int operator()(const QStringList& absPaths);

  void dropEvent(QDropEvent* event) override;
  void dragEnterEvent(QDragEnterEvent* event) override;

 private:
  QPlainTextEdit* m_md5TextEdit{nullptr};
  QAction* _ONLY_FIRST_BYTE{nullptr};
  QToolBar* m_md5InfoTB{nullptr};
  QVBoxLayout* mMainLayout{nullptr};

  QMap<QString, QString> m_fileMD5Map;
};

#endif  // MD5WINDOW_H
