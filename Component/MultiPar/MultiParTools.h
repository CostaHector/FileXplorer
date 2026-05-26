#ifndef MULTIPARTOOLS_H
#define MULTIPARTOOLS_H

#include <QStringList>
#include "ParVerifyInfomation.h"

namespace MultiParTools {
int clampRedundancy(int rateOfRedundancy);

bool IsMultiPar2Available(const QString& multiParPath);
std::pair<bool, int> CreatePar2(const QStringList& filesAbsPath, int rateOfRedundancy = 10);
std::pair<bool, ParVerifyInfomationList> VerifyFiles(const QStringList& filesAbsPath);
ParVerifyInfomation VerifyAFile(const QString& multiParPath, const QString& parFileAbsPath);
enum class RepairResultE {
  CANNOT_REPAIR = -2,
  NEED_MORE_TO_REPAIR = -1,
  REPAIR_OK_BEGIN = 0,
  SUCCESS_REPAIRED = REPAIR_OK_BEGIN,
  NO_NEED_REPAIR,
};
inline bool isRepairedOk(RepairResultE result) { return (int)result >= (int)RepairResultE::REPAIR_OK_BEGIN; }
RepairResultE RepairAFile(const QString& multiParPath, const QString& parFileAbsPath, QByteArray* pRepairCliOutput = nullptr);
} // namespace MultiParTools

#endif // MULTIPARTOOLS_H
