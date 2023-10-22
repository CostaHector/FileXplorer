#ifndef MIMEDATACX_H
#define MIMEDATACX_H

#include <QMimeData>
#include <QUrl>
#include "PublicVariable.h"
#include "Tools/ConflictsItemHelper.h"

class MimeDataCX : public QMimeData {
 public:
  const QString l;
  const QStringList lRels;
  CCMMode cutCopy;

  static MimeDataCX fromPlainMimeData(const QMimeData* baseMimeData);

  MimeDataCX(const MimeDataCX& rhs) noexcept;
//  MimeDataCX& operator==(const MimeDataCX& rhs) noexcept;

  MimeDataCX(const QString& l_, const QStringList& lRels_, const CCMMode cutCopy_ = CCMMode::ERROR)
      : QMimeData(), l(l_), lRels(lRels_), cutCopy(cutCopy_) {}

  MimeDataCX(const QPair<QString, QStringList>& lAndRels, const CCMMode cutCopy_ = CCMMode::ERROR)
      : MimeDataCX(lAndRels.first, lAndRels.second, cutCopy_) {}

  MimeDataCX(const QStringList& lAbsPathList, const CCMMode cutCopy_ = CCMMode::ERROR) : MimeDataCX(GetLAndRels(lAbsPathList), cutCopy_) {}

  MimeDataCX(const QMimeData& parent, const CCMMode cutCopy_ = CCMMode::ERROR) : MimeDataCX(Urls2QStringList(parent), cutCopy_) {
    setHtml(parent.html());
    setText(parent.text());
    setUrls(parent.urls());
    setImageData(parent.imageData());
  }

  QStringList Urls2QStringList(const QMimeData& mimeData) {
    QStringList lAbsPathList;
    for (const QUrl& url : mimeData.urls()) {
      lAbsPathList.append(url.toLocalFile());
    }
    return lAbsPathList;
  }

  void setMode(const CCMMode newMode) { cutCopy = newMode; }
};

#endif  // MIMEDATACX_H
