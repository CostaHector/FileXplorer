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
  static CCMMode getModeFrom(const QMimeData* native);

  MimeDataCX(const MimeDataCX& rhs) noexcept;
  
  MimeDataCX(const QString& l_, const QStringList& lRels_, const CCMMode cutCopy_ = CCMMode::ERROR_OP)
      : QMimeData(), l(l_), lRels(lRels_), m_cutCopy(cutCopy_) {
    refillBaseMode(m_cutCopy);
  }
  
  MimeDataCX(const std::pair<QString, QStringList>& lAndRels, const CCMMode cutCopy_ = CCMMode::ERROR_OP)
      : MimeDataCX(lAndRels.first, lAndRels.second, cutCopy_) {}
  
  MimeDataCX(const QStringList& lAbsPathList, const CCMMode cutCopy_ = CCMMode::ERROR_OP) : MimeDataCX(PATHTOOL::GetLAndRels(lAbsPathList), cutCopy_) {}
  
  MimeDataCX(const QMimeData& parent, const CCMMode cutCopy_ = CCMMode::ERROR_OP);

  void determineMode(const CCMMode newMode) { m_cutCopy = newMode; }
  bool refillBaseMode(const CCMMode mode);

 public:
  const QString l;
  const QStringList lRels;
  CCMMode m_cutCopy;
};

#endif  // MIMEDATACX_H
