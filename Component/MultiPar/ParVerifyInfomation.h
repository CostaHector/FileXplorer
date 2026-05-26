#ifndef PARVERIFYINFOMATION_H
#define PARVERIFYINFOMATION_H

#include "MultiParKey.h"
#include <QStringList>

struct ParVerifyInfomation {
  enum class Par2StatusE { ALL_FILES_COMPLETE = 0, READY_TO_REPAIR, READY_TO_RENAME, NEED_MORE_TO_REPAIR, NEED_VERIFY_AGAIN, CANNOT_VERIFY, PAR_STATUS_BUTT };

  ParVerifyInfomation() = default;
  explicit ParVerifyInfomation(Par2StatusE par2Status)
    : m_statusE{par2Status} {}
  explicit ParVerifyInfomation(QByteArray&& cliOutput) noexcept;
  explicit ParVerifyInfomation(const char* cliOutputCStr) noexcept
    : ParVerifyInfomation{QByteArray{cliOutputCStr}} {}
  bool updateCliOutput(QByteArray& newCliOutput);

  QString getFirstSrcFileAbsPath() const { return m_srcFileNames.isEmpty() ? "" : m_prePath + m_srcFileNames.front(); }
  QString getPar2FileAbsPath() const { return m_par2FileName.isEmpty() ? "" : m_prePath + m_par2FileName; }

  bool isTrustable() const noexcept { return (int) m_statusE < (int) Par2StatusE::NEED_VERIFY_AGAIN; }
  bool isNeedRepair() const { return m_statusE != Par2StatusE::ALL_FILES_COMPLETE; };
  bool isRepairable() const { return m_statusE == Par2StatusE::ALL_FILES_COMPLETE || m_statusE == Par2StatusE::READY_TO_REPAIR || m_statusE == Par2StatusE::READY_TO_RENAME;}
  bool isNeedReverify() const { return m_statusE == Par2StatusE::READY_TO_RENAME || m_statusE == Par2StatusE::NEED_VERIFY_AGAIN; }


#define MULTI_PAR_KEY_ITEM(enumName, enumValue, memberVariableDefValue, memberVariableName) decltype(memberVariableDefValue) m_##memberVariableName = memberVariableDefValue;
  MULTI_PAR_KEY_MAPPING
#undef MULTI_PAR_KEY_ITEM

  QByteArray m_verifyOutput;
  Par2StatusE m_statusE{Par2StatusE::NEED_VERIFY_AGAIN};
  // Need 854 more slice(s) to repair 1 file(s)
  // All Files Complete
  // Ready to repair 1 file(s)
  // Ready to rename 1 file(s)

  bool operator==(const ParVerifyInfomation& rhs) const noexcept {
    return
#define MULTI_PAR_KEY_ITEM(enumName, enumValue, memberVariableDefValue, memberVariableName) (m_##memberVariableName == rhs.m_##memberVariableDefValue) &&\
  MULTI_PAR_KEY_MAPPING
#undef MULTI_PAR_KEY_ITEM
        m_statusE == rhs.m_statusE;
  }

  void swap(ParVerifyInfomation& rhs) noexcept {
#define MULTI_PAR_KEY_ITEM(enumName, enumValue, memberVariableDefValue, memberVariableName) std::swap(m_##memberVariableName, rhs.m_##memberVariableName);
  MULTI_PAR_KEY_MAPPING
#undef MULTI_PAR_KEY_ITEM
    m_verifyOutput.swap(rhs.m_verifyOutput);
    std::swap(m_statusE, rhs.m_statusE);
  }

private:
  bool init() noexcept;
  void ParseCountInfo() noexcept;
  bool ParsePar2FilesList() noexcept;
  bool ParseStatusStr() noexcept;
};

using ParVerifyInfomationList = QList<ParVerifyInfomation>;
QByteArray GetLastLineFromFormattedCliByteArray(QByteArray& cliOutput);
#endif // PARVERIFYINFOMATION_H
