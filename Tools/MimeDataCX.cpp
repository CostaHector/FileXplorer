#include "MimeDataCX.h"

MimeDataCX::MimeDataCX(const MimeDataCX& rhs) noexcept : l(rhs.l), lRels(rhs.lRels), cutCopy(rhs.cutCopy) {
  setHtml(rhs.html());
  setText(rhs.text());
  setUrls(rhs.urls());
  setImageData(rhs.imageData());
}

MimeDataCX MimeDataCX::fromPlainMimeData(const QMimeData* baseMimeData) {
    const auto* p2CX = dynamic_cast<const MimeDataCX*>(baseMimeData);
    if (p2CX != nullptr){
      return *p2CX;
    }else{
      return *baseMimeData;
    }
    /* boolean expression will not call Copy Constructor. cause the cutCopy to default ERROR */
    // return p2CX != nullptr ? *p2CX : *baseMimeData;
}

//MimeDataCX& MimeDataCX::operator==(const MimeDataCX& rhs) noexcept {
//  l = rhs.l;
//  lRels = rhs.lRels;
//  cutCopy = rhs.cutCopy;
//  setHtml(rhs.html());
//  setText(rhs.text());
//  setUrls(rhs.urls());
//  setImageData(rhs.imageData());
//}
