#include "PerformersAkaManager.h"
#include "Tools/PerformerJsonFileHelper.h"
#include "Tools/FileDescriptor/TableFields.h"
#include "public/MemoryKey.h"
#include "public/PublicVariable.h"
#include "public/PublicMacro.h"
#include <QTextStream>

const QHash<QChar, QString> PerformersAkaManager::op2Str = {{'&', "AND"}, {'|', "OR"}};
constexpr char PerformersAkaManager::LOGIC_OR_CHAR;
constexpr char PerformersAkaManager::LOGIC_AND_CHAR;
const QString PerformersAkaManager::FUZZY_LIKE{R"(%1 like "%%2%")"};

PerformersAkaManager& PerformersAkaManager::getIns() {
  static PerformersAkaManager ins;
  return ins;
}

QString PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(const QString& keyName,          //
                                                               const QString& tokens,           //
                                                               const QString& binaryCondition,  //
                                                               const QHash<QString, QString>& ALSO_DICT) {
  if (keyName.isEmpty() || tokens.isEmpty()) {
    return "";
  }
  static const QSet<QChar> CONTROL_CHAR = {'(', ')', '&', '|'};
  static const auto isdigit = [](QChar c) -> bool { return not CONTROL_CHAR.contains(c); };
  static const auto precedence = [](QChar c) -> int {
    if (c == '&') {
      return 2;
    }
    if (c == '|') {
      return 1;
    }
    return 0;  // like open bracket '('
  };

  QStack<QString> values;  // reverse poland expr
  QStack<QChar> ops;

  for (int i = 0; i < tokens.size(); ++i) {
    if (tokens[i] == '(') {
      ops << tokens[i];
    } else if (tokens[i] == ')') {
      while (!ops.empty() && ops.top() != '(') {
        OperatorJoinOperands(values, ops);
        // pop opening brace.
      }
      if (!ops.empty()) {
        ops.pop();
      }
    } else if (isdigit(tokens[i])) {
      int startIndex = i;
      // There may be more than one
      // digits in number.
      while (i < tokens.length() && isdigit(tokens[i])) {
        i++;
      }
      // autoCompleteAka
      const QString& perf = tokens.mid(startIndex, i - startIndex);
      values << binaryCondition.arg(keyName, ALSO_DICT.value(perf, perf));

      // right now the i points to
      // the character next to the digit,
      // since the for loop also increases
      // the i, we would skip one
      //  token position; we need to
      // decrease the value of i by 1 to
      // correct the offset.
      i--;
    } else {
      // While top of 'ops' has same or greater
      // precedence to current token, which
      // is an operator. Apply operator on top
      // of 'ops' to top two elements in values stack.
      while (!ops.empty() && precedence(ops.top()) >= precedence(tokens[i])) {
        OperatorJoinOperands(values, ops);
      }

      // Push current token to 'ops'.
      ops.push(tokens[i]);
    }
  }
  while (!ops.empty()) {
    OperatorJoinOperands(values, ops);
  }
  return values.top();
}

PerformersAkaManager::PerformersAkaManager() : m_akaPerf(ReadOutAkaName()) {}

QHash<QString, QString> PerformersAkaManager::ReadOutAkaName() {
#ifdef _WIN32
  const QString akaPerfFilePath =  //
      PreferenceSettings().value(MemoryKey::WIN32_AKA_PERFORMERS.name, MemoryKey::WIN32_AKA_PERFORMERS.v).toString();
#else
  const QString akaPerfFilePath =  //
      PreferenceSettings().value(MemoryKey::LINUX_AKA_PERFORMERS.name, MemoryKey::LINUX_AKA_PERFORMERS.v).toString();
#endif
  QFile file(akaPerfFilePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("File not found: %s.", qPrintable(file.fileName()));
    return {};
  }

  QHash<QString, QString> akaDict;
  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  static const QRegularExpression PERF_SPLIT("\\s*,\\s*");
  while (!stream.atEnd()) {
    QString line = stream.readLine();
    line.replace(PERF_SPLIT, "|");
    for (const QString& perf : line.split('|')) {
      akaDict.insert(perf, line);
    }
  }
  file.close();
  qDebug("%d aka name(s) read out", akaDict.size());
  return akaDict;
}

int PerformersAkaManager::ForceReloadAkaName() {
  int beforeAkaNameCnt = m_akaPerf.size();
  m_akaPerf = PerformersAkaManager::ReadOutAkaName();
  int afterAkaNameCnt = m_akaPerf.size();
  qDebug("%d aka names added/removed", afterAkaNameCnt - beforeAkaNameCnt);
  return afterAkaNameCnt - beforeAkaNameCnt;
}

void PerformersAkaManager::OperatorJoinOperands(QStack<QString>& values, QStack<QChar>& ops) {
  QString val2 = values.top();
  values.pop();

  QString val1 = values.top();
  values.pop();

  QChar op = ops.top();
  ops.pop();

  values << QString("(%1 %2 %3)").arg(val1).arg(op2Str[op]).arg(val2);
}

QString PerformersAkaManager::GetMovieTablePerformerSelectCommand(const QSqlRecord& record) const {
  QString perfs = record.field(PERFORMER_DB_HEADER_KEY::Name_INDEX).value().toString();
  QString akas = record.field(PERFORMER_DB_HEADER_KEY::AKA_INDEX).value().toString();
  if (!akas.isEmpty()) {
    perfs += LOGIC_OR_CHAR;
    perfs += akas.replace(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR, LOGIC_OR_CHAR);
  }
  const QString& whereClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Name), perfs);
  // movies table
  using namespace MOVIE_TABLE;
  static const QString SELECT_NAME_TEMPLATE           //
      {QString{"SELECT `%1`, `%2`, `%3` FROM "}       //
           .arg(ENUM_2_STR(PrePathLeft))   //
           .arg(ENUM_2_STR(PrePathRight))  //
           .arg(ENUM_2_STR(Name))          //
       + "`%1` "                                      //
       + QString{"WHERE "}};
  return SELECT_NAME_TEMPLATE.arg(DB_TABLE::MOVIES) + whereClause;
}
