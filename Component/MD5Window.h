#ifndef MD5WINDOW_H
#define MD5WINDOW_H
#include <QDialog>
#include <QToolBar>
#include <QToolButton>
#include <QPlainTextEdit>
#include <QStringList>
#include <QMap>
#include <QVBoxLayout>

#include "EnumIntAction.h"
#include "BytesRangeTool.h"
#include <QCryptographicHash>

struct AbsFilePathBytesRangeHashAlgorithmKey {
  QString absFilePath;
  int firstByte;
  QCryptographicHash::Algorithm alg;
  bool operator< (const AbsFilePathBytesRangeHashAlgorithmKey& rhs) const {
    if (absFilePath != rhs.absFilePath) {
      return absFilePath < rhs.absFilePath;
    }
    if (firstByte != rhs.firstByte) {
      return firstByte < rhs.firstByte;
    }
    return alg < rhs.alg;
  }
};

extern template struct EnumIntAction<BytesRangeTool::BytesRangeE>;
extern template struct EnumIntAction<QCryptographicHash::Algorithm>;

class MD5Window : public QDialog {
public:
  explicit MD5Window(QWidget* parent = nullptr);
  int operator()(const QStringList& absPaths);

  void dragEnterEvent(QDragEnterEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void ReadSetting();
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

  void subscribe();
private:
  int GetBytesRange() const;
  void Recalculate();

  QAction* _ONLY_FIRST_8_BYTES{nullptr};
  QAction* _ONLY_FIRST_16_BYTES{nullptr};
  QAction* _ONLY_ENTIRE_FILE_BYTES{nullptr};
  EnumIntAction<BytesRangeTool::BytesRangeE> mBytesRangeIntAct;

  QAction* _MD5 {nullptr};
  QAction* _SHA1 {nullptr};
  QAction* _SHA256 {nullptr};
  QAction* _SHA512 {nullptr};
  QToolButton* _HASH_ALGORITHM_TOOLBUTTON{nullptr};
  EnumIntAction<QCryptographicHash::Algorithm> mHashAlgIntAct;

  QToolBar* m_md5InfoTB{nullptr};

  QPlainTextEdit* m_md5TextEdit{nullptr};

  QVBoxLayout* mMainLayout{nullptr};

  QStringList mPathsList;
  QMap<AbsFilePathBytesRangeHashAlgorithmKey, QString> mAlreadyCalculatedHashMap;
};

#endif  // MD5WINDOW_H
