#ifndef CLIPBOARDGUARD_H
#define CLIPBOARDGUARD_H
#include <memory>
#include "PublicMacro.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

class ClipboardGuard {
 public:
  ClipboardGuard() {
    mClipboard = QApplication::clipboard();
    CHECK_NULLPTR_RETURN_VOID(mClipboard);
    const QMimeData* original = mClipboard->mimeData();
    CHECK_NULLPTR_RETURN_VOID(original);

    auto* pTempEmptyMimeData = new (std::nothrow) QMimeData;
    CHECK_NULLPTR_RETURN_VOID(pTempEmptyMimeData);
    savedMimeData.reset(pTempEmptyMimeData);
    for (const QString& format : original->formats()) {
      savedMimeData->setData(format, original->data(format));
    }
  }

  operator bool() const {
    return mClipboard != nullptr && savedMimeData != nullptr;
  }

  ~ClipboardGuard() {
    if (mClipboard != nullptr && savedMimeData != nullptr) {
      mClipboard->setMimeData(savedMimeData.release());
    }
  }

  QClipboard* clipBoard() {
    return mClipboard;
  }

 private:
  QClipboard* mClipboard {nullptr};
  std::unique_ptr<QMimeData> savedMimeData {nullptr};
};

#endif  // CLIPBOARDGUARD_H
