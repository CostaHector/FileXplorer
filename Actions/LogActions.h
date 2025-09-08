#ifndef LOGACTIONS_H
#define LOGACTIONS_H

#include <QObject>
#include <QAction>
#include <QActionGroup>
#include <QToolButton>

class LogActions : public QObject {
 public:
  explicit LogActions(QObject* parent = nullptr);

  QAction* _LOG_FILE{nullptr};
  QAction* _LOG_FOLDER{nullptr};
  QAction* _LOG_AGING{nullptr};

  QAction* _LOG_PRINT_LEVEL_DEBUG{nullptr};
  QAction* _LOG_PRINT_LEVEL_WARNING{nullptr};
  QActionGroup* _LOG_PRINT_LEVEL_AG{nullptr};

  QAction* _AUTO_FLUSH_IGNORE_LEVEL{nullptr};

  QList<QAction*> _DROPDOWN_LIST;

  QToolButton* GetLogPreviewerToolButton(QWidget* parent);
};

LogActions& g_LogActions();

#endif  // LOGACTIONS_H
