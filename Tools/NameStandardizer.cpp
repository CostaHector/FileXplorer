#include "NameStandardizer.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include "PublicVariable.h"
#include "Tools/ProductionStudioManager.h"

const QRegExp stdCommaComp("\\s+,");
const QRegExp stdExclamationComp("\\s+!");

using namespace JSON_RENAME_REGEX;
auto NameStandardizer::operator()(QString aFileName) -> QString {
  // non-standard character
  QString& noInvalidChar = aFileName.replace(invalidCharPat, " ");
  QString& noExtraSpaceComma = noInvalidChar.replace(stdCommaComp, ", ");
  QString& noExtraExclamationComma = noExtraSpaceComma.replace(stdExclamationComp, "! ");
  QString& noContiousSpace = noExtraExclamationComma.replace(continuousSpaceComp, " ");
  QString& noInvalidQuote = noContiousSpace.replace(invalidQuotePat, "'");

  QString& noLeadingStr = noInvalidQuote.remove(leadingStrComp);
  QString noLeadingBracket = noLeadingStr.trimmed().remove(leadingOpenBracketComp);

  QString& noBracket = noLeadingBracket.replace(nonLeadingBracketComp, "-");
  QString& standardStr = noBracket.replace(spaceBarSpaceComp, "-");
  QString& noContinousHypen = standardStr.replace(continousHypenComp, "-");

  QString& isloatedDot = noContinousHypen.replace(hypenOrSpaceFollowedWithDotPat, ".");
  QString& noHypenEnds = isloatedDot.remove(trailingHypenComp);
  QString fileName = noHypenEnds.replace('-', " - ").trimmed();

  // Get standard Name
  const int barIndex = fileName.indexOf('-');
  if (barIndex == -1 or barIndex == 0) {
    return fileName;
  }
  static auto& psm = ProductionStudioManager::getIns();
  const QString& studioName = fileName.left(barIndex - 1);
  return psm[studioName] + fileName.mid(barIndex - 1);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  const auto& mp = NameStandardizer::jsonLoader();
  qDebug("STANDARD_STUDIO_NAME size=%d", mp.size());
}
#endif
