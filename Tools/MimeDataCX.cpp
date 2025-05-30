#include "MimeDataCX.h"

MimeDataCX::MimeDataCX(const MimeDataCX& rhs) noexcept : QMimeData(), l(rhs.l), lRels(rhs.lRels), m_cutCopy(rhs.m_cutCopy) {
  setHtml(rhs.html());
  setText(rhs.text());
  setUrls(rhs.urls());
  setImageData(rhs.imageData());
}

MimeDataCX::MimeDataCX(const QMimeData& parent, const CCMMode::Mode cutCopy_) : MimeDataCX(Urls2QStringList(parent), cutCopy_) {
  // from native QMimeData
  setHtml(parent.html());
  setText(parent.text());
  setUrls(parent.urls());
  setImageData(parent.imageData());
}

bool MimeDataCX::refillBaseMode(const CCMMode::Mode mode) {
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
  return true;
}

MimeDataCX MimeDataCX::fromPlainMimeData(const QMimeData* baseMimeData) {
  const auto* p2CX = dynamic_cast<const MimeDataCX*>(baseMimeData);
  if (p2CX != nullptr) {
    return *p2CX;
  }
  const QByteArray& ba = baseMimeData->data("Preferred DropEffect");
  qDebug("Preferred DropEffect %s", ba.toStdString().c_str());
  if (ba[0] == 0x2) {  // # 2 for cut and 5 for copy
    return {*baseMimeData, CCMMode::CUT_OP};
  } else if (ba[0] == 0x5) {
    return {*baseMimeData, CCMMode::COPY_OP};
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

CCMMode::Mode MimeDataCX::getModeFrom(const QMimeData* native) {
  const QByteArray& ba = native->data("Preferred DropEffect");
  if (ba[0] == 0x2) {  // # 2 for cut and 5 for copy
    return CCMMode::CUT_OP;
  } else if (ba[0] == 0x5) {
    return CCMMode::COPY_OP;
  }
  return CCMMode::ERROR_OP;
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
