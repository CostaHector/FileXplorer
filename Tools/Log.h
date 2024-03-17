#ifndef LOG_H
#define LOG_H

#include <QColor>
struct Log {
 public:
  static Log fromLogFilesLine(const QString& line);
  Log(const QString& time_, QChar level_, const QString& msg_, const QString& fileName_, int lineNo_, const QString& funcName_)
      : time{time_}, level{level_}, msg{msg_}, fileName{fileName_}, lineNo{lineNo_}, funcName{funcName_} {}
  Log() = default;

  operator bool() const { return not time.isEmpty(); };

  QString time;
  QChar level;
  QString msg;
  QString fileName;
  int lineNo;
  QString funcName;

  static inline QColor GetColor(QChar c) {
    if (not color_array_inited) {
      Log::COLOR_ARRAY['I' - 'A'] = QColor(0xd0, 0xe8, 0xc4);
      Log::COLOR_ARRAY['D' - 'A'] = QColor(0x94, 0xda, 0xe2);
      Log::COLOR_ARRAY['W' - 'A'] = QColor(0xf2, 0x89, 0x44);
      Log::COLOR_ARRAY['C' - 'A'] = QColor(0xcb, 0x37, 0x78);
      Log::COLOR_ARRAY['F' - 'A'] = QColor(0xc7, 0x0, 0xb);
      color_array_inited = true;
    }
    return COLOR_ARRAY[c.cell() - 'A'];
  }
  static const QStringList INDEX2COLUMNNAME;
  static QColor COLOR_ARRAY[26];
  static bool color_array_inited;
};
#endif  // LOG_H
