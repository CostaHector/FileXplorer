#ifndef ACTIONWITHPATH_H
#define ACTIONWITHPATH_H

#include <QAction>
#include "PublicVariable.h"

class ActionWithPath : public QAction {
 public:
  explicit ActionWithPath(const QString& path, QObject* parent = nullptr);
  explicit ActionWithPath(const QString& path, const QString& text, QObject* parent = nullptr);
  explicit ActionWithPath(const QString& path, const QIcon& icon, const QString& text, QObject* parent = nullptr);

  inline void setPath(const QString& path) { m_path = path; }
  inline QString getPath() const { return m_path; }
  bool on_PathActionTriggered();
  static void BindIntoNewPath(T_IntoNewPath IntoNewPath) { m_IntoNewPath = IntoNewPath; }

 private:
  void subscribe() { connect(this, &QAction::triggered, this, &ActionWithPath::on_PathActionTriggered); }

  static T_IntoNewPath m_IntoNewPath;

  QString m_path;
};

#endif  // ACTIONWITHPATH_H
