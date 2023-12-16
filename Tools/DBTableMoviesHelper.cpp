#include "DBTableMoviesHelper.h"
#include "Tools/PerformerJsonFileHelper.h"

QHash<QString, QString> DBTableMoviesHelper::akaPerf;
const QHash<QChar, QString> DBTableMoviesHelper::op2Str = {{'&', "AND"}, {'|', "OR"}};
constexpr char DBTableMoviesHelper::LOGIC_OR_CHAR;
constexpr char DBTableMoviesHelper::LOGIC_AND_CHAR;

DBTableMoviesHelper::DBTableMoviesHelper() {}

void DBTableMoviesHelper::OperatorJoinOperands(QStack<QString>& values, QStack<QChar>& ops) {
  QString val2 = values.top();
  values.pop();

  QString val1 = values.top();
  values.pop();

  QChar op = ops.top();
  ops.pop();

  values << QString("(%1 %2 %3)").arg(val1).arg(op2Str[op]).arg(val2);
}

QString DBTableMoviesHelper::PlainLogicSentence2FuzzySqlWhere(const QString& tokens,
                                                              const QString& keyName,
                                                              const bool autoCompleteAka,
                                                              const QString& binaryCondition) {
  if (tokens.isEmpty()) {
    return "";
  }

  const bool isAutoReplaceAkaEnabled = autoCompleteAka and (keyName == DB_HEADER_KEY::ForSearch or keyName == DB_HEADER_KEY::Performers or
                                                            keyName == DB_HEADER_KEY::Prepath or keyName == DB_HEADER_KEY::Name);

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

  QStack<QString> values; // reverse poland expr
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
      const QString& searchName = isAutoReplaceAkaEnabled and akaPerf.contains(perf) ? akaPerf[perf] : perf;
      values << binaryCondition.arg(keyName, searchName);

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

int DBTableMoviesHelper::UpdateAKAHash(const bool isForce) {
  if (not(akaPerf.isEmpty() or isForce)) {  // update when empty or force
    return 0;
  }
  QFile file(SystemPath::AKA_PERFORMERS_TXT);
  if (not file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("File not found: %s.", file.fileName().toStdString().c_str());
    return -1;
  }

  int succeedCnt = 0;
  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  static const QRegExp PERF_SPLIT("\\s*,\\s*");
  while (not stream.atEnd()) {
    QString line = stream.readLine();
    line.replace(PERF_SPLIT, "|");
    for (const QString& perf : line.split('|')) {
      akaPerf.insert(perf, line);
    }
    ++succeedCnt;
  }
  file.close();
  return succeedCnt;
}

QString DBTableMoviesHelper::GetMovieTablePerformerSelectCommand(const QSqlRecord& record) {
  QString perfs = record.field(PERFORMER_DB_HEADER_KEY::Name_INDEX).value().toString();
  QString akas = record.field(PERFORMER_DB_HEADER_KEY::AKA_INDEX).value().toString();
  if (not akas.isEmpty()) {
    perfs += (LOGIC_OR_CHAR + akas.replace(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR, LOGIC_OR_CHAR));
  }
  const QString& whereClause = DBTableMoviesHelper::PlainLogicSentence2FuzzySqlWhere(perfs, DB_HEADER_KEY::ForSearch, false);
  // movies table
  return QString("SELECT `%1` from %2 where %3").arg(DB_HEADER_KEY::ForSearch, DB_TABLE::MOVIES, whereClause);
}
