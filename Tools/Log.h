#ifndef LOG_H
#define LOG_H

#include <QColor>
struct Log {
 public:
  explicit Log(const QStringList& lst)
      : time{lst[0]}, level{lst[1][0].cell()}, msg{lst[2]}, fileName{lst[3]}, lineNo{lst[4].toInt()}, funcName{lst[5]} {}
  explicit Log(const QString& log) : Log(log.split('\t')) {}

  QString time;
  uchar level;
  QString msg;
  QString fileName;
  int lineNo;
  QString funcName;

  static inline QColor GetColor(char c) {
    if (not color_array_inited) {
      Log::COLOR_ARRAY['I' - 'A'] = QColor(0xd0, 0xe8, 0xc4);
      Log::COLOR_ARRAY['D' - 'A'] = QColor(0x94, 0xda, 0xe2);
      Log::COLOR_ARRAY['W' - 'A'] = QColor(0xf2, 0x89, 0x44);
      Log::COLOR_ARRAY['C' - 'A'] = QColor(0xcb, 0x37, 0x78);
      Log::COLOR_ARRAY['F' - 'A'] = QColor(0xc7, 0x0, 0xb);
      color_array_inited = true;
    }
    return COLOR_ARRAY[c - 'A'];
  }
  static const QStringList INDEX2COLUMNNAME;
  static QColor COLOR_ARRAY[26];
  static bool color_array_inited;
};
#endif  // LOG_H
