#ifndef QTPROCESSGUARD_H
#define QTPROCESSGUARD_H

#include <QObject>

class QtProcessGuard : public QObject {
  Q_OBJECT
 public:
  explicit QtProcessGuard(int timeoutSec = 20, const QString& processPattern = "qt", QObject* parent = nullptr);
  ~QtProcessGuard();

 private:
  QString mStartGuard, mStopGuard;
  const int m_timeoutSec;
  static constexpr char PID_FILE_PATH[] {"/tmp/qt_program_guard.pid"};
};

#endif  // QTPROCESSGUARD_H
