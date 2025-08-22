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

  void subscribe();
private:
  int GetBytesRange() const;
  void Recalculate();

  QAction* _ONLY_FIRST_8_BYTES{nullptr};
  QAction* _ONLY_FIRST_16_BYTES{nullptr};
  QAction* _ONLY_EVERY_BYTES{nullptr};
  QActionGroup* _CALC_BYTES_RANGE{nullptr};
  QToolBar* m_md5InfoTB{nullptr};

  QPlainTextEdit* m_md5TextEdit{nullptr};

  QVBoxLayout* mMainLayout{nullptr};

  QStringList mPathsList;
  QSet<QString> mPathsSet;
};

#endif  // MD5WINDOW_H
