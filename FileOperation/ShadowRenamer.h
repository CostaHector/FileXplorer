#ifndef SHADOWRENAMER_H
#define SHADOWRENAMER_H

#include <QString>
class QDir;

#define SHADOW_ID "_SHADOW"

namespace ShadowRenamer {
bool CheckPathValid(const QString& folderFullPathContainsVideosNeedRename, const QString& shadowPath);

enum class SHADOW_STATUS {
  BEGIN = 0x1,
  PENDING_SYNC = BEGIN,   // Transient, pending sync: user renamed it, but write-back has not succeeded yet, or retry is needed
  ALREADY_SYNCED = 0x2,     // Terminal, already synced: write-back succeeded; this staging file no longer takes effect
  NO_NEED_SYNC = 0x4,       // Transient, no sync needed: relative path to this staging file matches itself content; user has not modified it
  NO_CORRESPOND_FILE = 0x8, // Transient, no corresponding file: source video file is missing
  BUTT = 0x16,
};
inline bool isShadowStatusValid(const int status) {
  switch (status) {
    case (int)SHADOW_STATUS::PENDING_SYNC:
    case (int)SHADOW_STATUS::ALREADY_SYNCED:
    case (int)SHADOW_STATUS::NO_NEED_SYNC:
    case (int)SHADOW_STATUS::NO_CORRESPOND_FILE:
      return true;
    default:
      return false;
  }
}

inline QString JoinShadowFileContent(const QString& correspondingFileName, SHADOW_STATUS status) {
  return correspondingFileName + '\t' + QString::number((int)status);
}
std::pair<QString, SHADOW_STATUS> ParseShadowFileContent(const QString& shadowFileContent);
bool IsTreatAsShadowFile(const QString& shadowFileFullPath);
bool PrepareForParentFolder(QDir& dir, const QString& relative2File);
int CreateShadowFileUsingNameAsContent(const QString& folderFullPathContainsVideosNeedRename, const QString& shadowPath);

// return value: pair<bool, struct>{is Procedure finished normally, details};
struct SyncDetails {
  SyncDetails() = default;
  SyncDetails(int _shadowFilesCnt, int _pendingSyncCnt, int _alreadySyncedCnt, int _noNeedSyncCnt, int _noCorrespondingCnt, int _newSyncedCnt) :
    shadowFilesCnt{_shadowFilesCnt},
    pendingSyncCnt{_pendingSyncCnt}, alreadySyncedCnt{_alreadySyncedCnt}, noNeedSyncCnt{_noNeedSyncCnt}, noCorrespondingCnt{_noCorrespondingCnt},
    newSyncedCnt{_newSyncedCnt} { }
  bool operator==(const SyncDetails& rhs) const {
    return shadowFilesCnt == rhs.shadowFilesCnt
        && pendingSyncCnt == rhs.pendingSyncCnt
        && alreadySyncedCnt == rhs.alreadySyncedCnt
        && noNeedSyncCnt == rhs.noNeedSyncCnt
        && noCorrespondingCnt == rhs.noCorrespondingCnt
        && newSyncedCnt == rhs.newSyncedCnt;
  }
  int shadowFilesCnt{0}, pendingSyncCnt{0}, alreadySyncedCnt{0}, noNeedSyncCnt{0}, noCorrespondingCnt{0}, newSyncedCnt{0};
  bool isFinished() const { return pendingSyncCnt == 0 && noCorrespondingCnt == 0; }
  QString logStr() const {
    return QString::asprintf("ShadowCnt:%d(IsFinished:%d)(pending:%d,already:%d,noNeed:%d,noCorresponding:%d,new:%d)", shadowFilesCnt, isFinished(),
                             pendingSyncCnt, alreadySyncedCnt, noNeedSyncCnt, noCorrespondingCnt, newSyncedCnt);
  }
};
std::pair<bool, SyncDetails> SyncSourceFileByShadowFile(const QString& shadowPath, const QString& folderFullPathContainsVideosNeedRename);

inline QString GetStagingFileFolder(const QString& srcPath) {
  return srcPath + SHADOW_ID;
}
inline bool isStagingFileFolder(const QString& shadowPath) {
  return shadowPath.endsWith(SHADOW_ID, Qt::CaseSensitive);
}
inline QString GetVideoForPlayPath(QString vidFullPath) {
  if (!IsTreatAsShadowFile(vidFullPath)) {
    return vidFullPath;
  }
  const int lastIndex = vidFullPath.lastIndexOf(SHADOW_ID "/");
  if (lastIndex == -1) {
    return vidFullPath;
  }
  return vidFullPath.remove(lastIndex, sizeof(SHADOW_ID) - 1);
}
inline std::pair<bool, QString> ChopShadowPostFix(const QString& shadowPath) {
  if (!isStagingFileFolder(shadowPath)) {
    return {false, ""};
  }
  return {true, shadowPath.left(shadowPath.size() - (sizeof(SHADOW_ID) - 1))};
}

int onCreateStagingFile(const QString& srcPath);
SyncDetails onShowStagingStatistics(const QString& shadowPath);

inline bool IsStatusInRemoveBits(int removeStatusBits, SHADOW_STATUS status) {
  return (removeStatusBits & (int)status) != 0;
}
std::pair<bool, int> recycleStagingFileWithGivenStatus(const QString& shadowPath, int removeStatusBits);
std::pair<bool, int> onRecycleNoNeedSyncStagingFile(const QString& shadowPath);
std::pair<bool, int> onRecycleAlreadySyncedStagingFile(const QString& shadowPath);
};


#undef SHADOW_ID
#endif // SHADOWRENAMER_H
