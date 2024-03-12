#include "MimeDataCX.h"

MimeDataCX::MimeDataCX(const MimeDataCX& rhs) noexcept : l(rhs.l), lRels(rhs.lRels), m_cutCopy(rhs.m_cutCopy) {
  setHtml(rhs.html());
  setText(rhs.text());
  setUrls(rhs.urls());
  setImageData(rhs.imageData());
}

MimeDataCX::MimeDataCX(const QMimeData& parent, const CCMMode cutCopy_) : MimeDataCX(Urls2QStringList(parent), cutCopy_) {
  // from native QMimeData
  setHtml(parent.html());
  setText(parent.text());
  setUrls(parent.urls());
  setImageData(parent.imageData());
}

bool MimeDataCX::refillBaseMode(const CCMMode mode) {
  QByteArray preferred(4, 0x0);
  if (mode == CCMMode::CUT) {  // # 2 for cut and 5 for copy
    preferred[0] = 0x2;
  } else if (mode == CCMMode::COPY) {
    preferred[0] = 0x5;
  } else {
    qDebug("cannot refill base DropEffect");
    return false;
  }
  QMimeData::setData("Preferred DropEffect", preferred);
  return true;
}

MimeDataCX MimeDataCX::fromPlainMimeData(const QMimeData* baseMimeData) {
  const auto* p2CX = dynamic_cast<const MimeDataCX*>(baseMimeData);
  if (p2CX != nullptr) {
    return *p2CX;
  }
  const QByteArray& ba = baseMimeData->data("Preferred DropEffect");
  qDebug() << "Preferred DropEffect" << ba;
  if (ba[0] == 0x2) {  // # 2 for cut and 5 for copy
    return {*baseMimeData, CCMMode::CUT};
  } else if (ba[0] == 0x5) {
    return {*baseMimeData, CCMMode::COPY};
  }
  return *baseMimeData;
}

QStringList MimeDataCX::Urls2QStringList(const QMimeData& mimeData) {
  QStringList lAbsPathList;
  for (const QUrl& url : mimeData.urls()) {
    lAbsPathList.append(url.toLocalFile());
  }
  return lAbsPathList;
}

CCMMode MimeDataCX::getModeFrom(const QMimeData* native) {
  const QByteArray& ba = native->data("Preferred DropEffect");
  if (ba[0] == 0x2) {  // # 2 for cut and 5 for copy
    return CCMMode::CUT;
  } else if (ba[0] == 0x5) {
    return CCMMode::COPY;
  }
  return CCMMode::ERROR;
}

// MimeDataCX& MimeDataCX::operator==(const MimeDataCX& rhs) noexcept {
//   l = rhs.l;
//   lRels = rhs.lRels;
//   cutCopy = rhs.cutCopy;
//   setHtml(rhs.html());
//   setText(rhs.text());
//   setUrls(rhs.urls());
//   setImageData(rhs.imageData());
// }
