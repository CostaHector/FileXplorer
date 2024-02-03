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

using namespace JSON_RENAME_REGEX;
auto NameStandardizer::operator()(QString aFileName) -> QString{
  auto noInvalidChar = aFileName.replace(invalidCharPat, " ");
  auto noContiousSpace = noInvalidChar.replace(continuousSpaceComp, " ");
  auto noInvalidQuote = noContiousSpace.replace(invalidQuotePat, "'");

          //non-standard character

  auto noLeadingStr = noInvalidQuote.remove(leadingStrComp);

  auto noLeadingBracket = noLeadingStr.trimmed().remove(leadingOpenBracketComp);

  auto noBracket = noLeadingBracket.replace(nonLeadingBracketComp, "-");
  auto standardStr = noBracket.replace(spaceBarSpaceComp, "-");
  auto noContinousHypen = standardStr.replace(continousHypenComp, "-");

  auto isloatedDot = noContinousHypen.replace(hypenOrSpaceFollowedWithDotPat, ".");
  auto noHypenEnds = isloatedDot.remove(trailingHypenComp);
  auto fileName = noHypenEnds.replace('-', " - ").trimmed();

          //Get standard Name
  auto barIndex = fileName.indexOf('-');
  if (barIndex == -1 or barIndex == 0){
    return fileName;
  }
  static auto& psm = ProductionStudioManager::getIns();
  const QString& studioName = fileName.left(barIndex - 1);
  return psm[studioName] + fileName.mid(barIndex - 1);
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  const auto& mp = NameStandardizer::jsonLoader();
  qDebug("STANDARD_STUDIO_NAME size=%d", mp.size());
}
#endif
