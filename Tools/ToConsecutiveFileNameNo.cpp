#include "ToConsecutiveFileNameNo.h"

// exception: nth should not seen as file no.
// scene n.jpg
// part n.jpg
// pt n.jpg
// sc n.jpg
// pt. n.jpg
// sc. n.jpg

const QRegularExpression ToConsecutiveFileNameNo::EXCEPTION_FILE_NAME_NO_PATTERN{"\\b(scene|part|sc\\.|sc|pt\\.|pt)\\s?\\d{1,2}\\..{1,4}$",
                                                                                 QRegularExpression::PatternOption::CaseInsensitiveOption};
const QRegularExpression ToConsecutiveFileNameNo::FILE_NAME_NO_PATTERN{"[ -](\\d{1,2})\\..{1,4}$"};

QStringList ToConsecutiveFileNameNo::operator()(const QStringList& names) {
  // names must sorted ascending and ignore case;
  QStringList newNames;
  newNames.reserve(names.size());
  int currentNameIndex = m_noStart;
  for (const QString& oldName : names) {
    QString newNameTemplate;
    if (!nameNeedToProcess(oldName, newNameTemplate, currentNameIndex)) {
      newNames.push_back(oldName);
      continue;
    }
    newNames.push_back(newNameTemplate);
    currentNameIndex++;
  }
  return newNames;
}

bool ToConsecutiveFileNameNo::nameNeedToProcess(const QString& s, QString& sTemplate, const int fileNo) {
  if (EXCEPTION_FILE_NAME_NO_PATTERN.match(s).hasMatch()) {
    return false;
  }
  QRegularExpressionMatch result = FILE_NAME_NO_PATTERN.match(s);
  if (not result.hasMatch()) {
    return false;
  }
  const int b = result.capturedStart(1), e = result.capturedEnd(1);
  const int len = e - b;
  sTemplate = s;
  sTemplate.replace(b, len, QString::number(fileNo));
  return true;
}
