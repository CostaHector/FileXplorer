#ifndef MIMEDATACX_H
#define MIMEDATACX_H

#include <QMimeData>
#include <QUrl>
#include "public/PublicVariable.h"
#include "public/PathTool.h"

class MimeDataCX : public QMimeData {
  // Enhanced MimeDataCX, always keep folder structure
 public:
  static MimeDataCX fromPlainMimeData(const QMimeData* baseMimeData);
  static QStringList Urls2QStringList(const QMimeData& mimeData);
  static CCMMode::Mode getModeFrom(const QMimeData* native);

  MimeDataCX(const MimeDataCX& rhs) noexcept;
  
  MimeDataCX(const QString& l_, const QStringList& lRels_, const CCMMode::Mode cutCopy_ = CCMMode::ERROR_OP)
      : QMimeData(), l(l_), lRels(lRels_), m_cutCopy(cutCopy_) {
    refillBaseMode(m_cutCopy);
  }
  
  MimeDataCX(const std::pair<QString, QStringList>& lAndRels, const CCMMode::Mode cutCopy_ = CCMMode::ERROR_OP)
      : MimeDataCX(lAndRels.first, lAndRels.second, cutCopy_) {}
  
  MimeDataCX(const QStringList& lAbsPathList, const CCMMode::Mode cutCopy_ = CCMMode::ERROR_OP) : MimeDataCX(PathTool::GetLAndRels(lAbsPathList), cutCopy_) {}
  
  MimeDataCX(const QMimeData& parent, const CCMMode::Mode cutCopy_ = CCMMode::ERROR_OP);

  void determineMode(const CCMMode::Mode newMode) { m_cutCopy = newMode; }
  bool refillBaseMode(const CCMMode::Mode mode);

 public:
  const QString l;
  const QStringList lRels;
  CCMMode::Mode m_cutCopy;
};

#endif  // MIMEDATACX_H
