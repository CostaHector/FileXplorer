#ifndef MIMEDATACX_H
#define MIMEDATACX_H

#include <QMimeData>
#include <QUrl>
#include "PublicVariable.h"
#include "Tools/PathTool.h"

class MimeDataCX : public QMimeData {
 public:
  static MimeDataCX fromPlainMimeData(const QMimeData* baseMimeData);
  static QStringList Urls2QStringList(const QMimeData& mimeData) {
    QStringList lAbsPathList;
    for (const QUrl& url : mimeData.urls()) {
      lAbsPathList.append(url.toLocalFile());
    }
    return lAbsPathList;
  }

  MimeDataCX(const QMimeData& parent, const CCMMode cutCopy_ = CCMMode::ERROR) :
        MimeDataCX(Urls2QStringList(parent), cutCopy_) {
    setHtml(parent.html());
    setText(parent.text());
    setUrls(parent.urls());
    setImageData(parent.imageData());
  }

  MimeDataCX(const MimeDataCX& rhs) noexcept;

  MimeDataCX(const QString& l_, const QStringList& lRels_, const CCMMode cutCopy_ = CCMMode::ERROR)
      : QMimeData(), l(l_), lRels(lRels_), cutCopy(cutCopy_) {}

  MimeDataCX(const std::pair<QString, QStringList>& lAndRels, const CCMMode cutCopy_ = CCMMode::ERROR)
      : MimeDataCX(lAndRels.first, lAndRels.second, cutCopy_) {}

  MimeDataCX(const QStringList& lAbsPathList, const CCMMode cutCopy_ = CCMMode::ERROR) :
        MimeDataCX(PATHTOOL::GetLAndRels(lAbsPathList), cutCopy_) {
  }

  void setMode(const CCMMode newMode) { cutCopy = newMode; }

 public:
  const QString l;
  const QStringList lRels;
  CCMMode cutCopy;
};

#endif  // MIMEDATACX_H
