#include "MimeDataCX.h"

bool MimeDataCX::refillBaseMode(const CCMMode::Mode mode) {
#ifdef _WIN32
  QByteArray preferred(4, 0x0);
  if (mode == CCMMode::CUT_OP) {  // # 2 for cut and 5 for copy
    preferred[0] = 0x2;
  } else if (mode == CCMMode::COPY_OP) {
    preferred[0] = 0x5;
  } else {
    qDebug("cannot refill base DropEffect");
    return false;
  }
  QMimeData::setData("Preferred DropEffect", preferred);
#else
  if (mode == CCMMode::CUT_OP) {
    QMimeData::setData("XdndAction", "XdndActionMove");  // 剪切操作标识符:ml-citation{ref="10" data="citationList"}
  } else if (mode == CCMMode::COPY_OP) {
    QMimeData::setData("XdndAction", "XdndActionCopy");  // 复制操作标识符:ml-citation{ref="10" data="citationList"}
  } else {
    qDebug("cannot refill base DropEffect");
    return false;
  }
#endif
  return true;
}

void MimeDataCX::SetUrls() {
  QList<QUrl> urls;
  for (const QString& selection : mRelSelections) {
    urls.append(QUrl::fromLocalFile(mRootPath + '/' + selection));
  }
  setUrls(urls);
  setText(mRelSelections.join('\n'));
}

MimeDataCX MimeDataCX::FromNativeMimeData(const QMimeData* baseMimeData) {
  const auto* p2CX = dynamic_cast<const MimeDataCX*>(baseMimeData);
  if (p2CX != nullptr) {
    qDebug("No need to get mode from native");
    return *p2CX;
  }
  CCMMode::Mode mode = GetCutCopyModeFromNative(baseMimeData);
  if (mode != CCMMode::Mode::ERROR_OP) {
    return {*baseMimeData, mode};
  }
  return *baseMimeData;
}

QStringList MimeDataCX::Urls2QStringList(const QMimeData& mimeData) {
  if (!mimeData.hasUrls()) {
    return {};
  }
  QStringList lAbsPathList;
  lAbsPathList.reserve(mimeData.urls().size());
  for (const QUrl& url : mimeData.urls()) {
    lAbsPathList.append(url.toLocalFile());
  }
  return lAbsPathList;
}

CCMMode::Mode MimeDataCX::GetCutCopyModeFromNative(const QMimeData* native) {
  if (native == nullptr) {
    qCritical("native is nullptr");
    return CCMMode::ERROR_OP;
  }
#ifdef _WIN32
  if (native->hasFormat("Preferred DropEffect")) {
    const QByteArray& ba = native->data("Preferred DropEffect");
    if (ba[0] == 0x2) {  // # 2 for cut and 5 for copy
      return CCMMode::CUT_OP;
    } else if (ba[0] == 0x5) {
      return CCMMode::COPY_OP;
    }
    qWarning("Preferred DropEffect value[%d] invalid", (int)ba[0]);
  }
#else
  if (native->hasFormat("XdndAction")) {
    QByteArray ba = native->data("XdndAction");
    const QString cutOrCopyAction = QString::fromUtf8(ba);
    if (cutOrCopyAction == "XdndActionMove") {  // 0xx
      return CCMMode::CUT_OP;
    } else if (cutOrCopyAction == "XdndActionCopy") {
      return CCMMode::COPY_OP;  // 0x1
    }
    qWarning("XdndAction value[%s] invalid", qPrintable(cutOrCopyAction));
  }
#endif
  return CCMMode::ERROR_OP;
}
