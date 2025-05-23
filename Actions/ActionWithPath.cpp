#include "ActionWithPath.h"

T_IntoNewPath ActionWithPath::m_IntoNewPath;

ActionWithPath::ActionWithPath(const QString& path, QObject* parent)  //
    : QAction{parent}, m_path{path} {
  setToolTip(m_path);
  subscribe();
}

ActionWithPath::ActionWithPath(const QString& path, const QString& text, QObject* parent)  //
    : QAction{text, parent}, m_path{path} {
  setToolTip(m_path);
  subscribe();
}

ActionWithPath::ActionWithPath(const QString& path, const QIcon& icon, const QString& text, QObject* parent)  //
    : QAction{icon, text, parent}, m_path{path} {
  setToolTip(m_path);
  subscribe();
}

bool ActionWithPath::on_PathActionTriggered() {
  if (m_IntoNewPath == nullptr) {
    qDebug("m_IntoNewPath is nullptr. Cannot into path [%s]", qPrintable(m_path));
    return false;
  }
  return m_IntoNewPath(getPath(), true, true);
}

void ActionWithPath::subscribe() {
  connect(this, &QAction::triggered, this, &ActionWithPath::on_PathActionTriggered);
}
