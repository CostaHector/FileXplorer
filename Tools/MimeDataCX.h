#ifndef MIMEDATACX_H
#define MIMEDATACX_H

#include <QMimeData>
#include <QUrl>
#include <QSet>
#include "public/PublicVariable.h"
#include "public/PathTool.h"

class MimeDataCX : public QMimeData {
 public:
  static MimeDataCX FromNativeMimeData(const QMimeData* baseMimeData);
  static QStringList Urls2QStringList(const QMimeData& mimeData);
  static CCMMode::Mode GetCutCopyModeFromNative(const QMimeData* native);

  // from native mimeData
  MimeDataCX(const QMimeData& parent,                           //
             const CCMMode::Mode cutCopy_ = CCMMode::ERROR_OP)  //
      : MimeDataCX{Urls2QStringList(parent), cutCopy_}          //
  {
    setHtml(parent.html());
    setText(parent.text());
    setUrls(parent.urls());
    setImageData(parent.imageData());
  }
  // from native mimeData selected urls to selected stringlist
  MimeDataCX(const QStringList& lAbsPathList,                        //
             const CCMMode::Mode cutCopy_ = CCMMode::ERROR_OP)       //
      : MimeDataCX{PathTool::GetLAndRels(lAbsPathList), cutCopy_} {  //
  }

  MimeDataCX(const MimeDataCX& rhs) noexcept  //
      : QMimeData{},                          //
        mRootPath{rhs.mRootPath},             //
        mRelSelections{rhs.mRelSelections},   //
        m_cutCopy{rhs.m_cutCopy}              //
  {
    setHtml(rhs.html());
    setText(rhs.text());
    setUrls(rhs.urls());
    setImageData(rhs.imageData());
  }

  // rootpath_, selection_
  MimeDataCX(const QString& rootpath_,                          //
             const QStringList& selections_,                    //
             const CCMMode::Mode cutCopy_ = CCMMode::ERROR_OP)  //
      : QMimeData{},                                            //
        mRootPath{rootpath_},                                   //
        mRelSelections{selections_},                            //
        m_cutCopy{cutCopy_} {                                   //
    refillBaseMode(m_cutCopy);
  }

  MimeDataCX(const PathTool::SelectionInfo& info,               //
             const CCMMode::Mode cutCopy_ = CCMMode::ERROR_OP)  //
      : QMimeData{},                                            //
        mRootPath{info.rootPath},                               //
        mRelSelections{info.relSelections},                     //
        mRootPaths{info.rootPaths},                             //
        mSelections{info.selections},                           //
        m_cutCopy{cutCopy_} {                                   //
    SetUrls();
    refillBaseMode(m_cutCopy);
  }

  MimeDataCX(const std::pair<QString, QStringList>& rootpathAndSelections,                 //
             const CCMMode::Mode cutCopy_ = CCMMode::ERROR_OP)                             //
      : MimeDataCX{rootpathAndSelections.first, rootpathAndSelections.second, cutCopy_} {  //
  }

  void determineMode(const CCMMode::Mode newMode) {  //
    m_cutCopy = newMode;
  }

  bool refillBaseMode(const CCMMode::Mode mode);

  bool IsFileSystemStuctureMatter() const {  //
    if (mRootPaths.isEmpty()) {              // from built in file system, stucture can be ignored
      return false;
    }
    return QSet<QString>{mRootPaths.cbegin(), mRootPaths.cend()} != QSet<QString>{mRootPath};
  }

 public:
  // used for keep file system structure, mSelections can be "file" or "path/to/file"
  // mRootPath + '/' + mRelSelection = absoluteFilePath
  const QString mRootPath;
  const QStringList mRelSelections;
  // used for flatten file system structure, mFiles only can be "file"
  // mRootPath + '/' + mSelection = absoluteFilePath
  QStringList mRootPaths;
  QStringList mSelections;

  CCMMode::Mode m_cutCopy;

 private:
  void SetUrls();
};

#endif  // MIMEDATACX_H
