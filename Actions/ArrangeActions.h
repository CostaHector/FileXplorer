#ifndef ARRANGEACTIONS_H
#define ARRANGEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class ArrangeActions : public QObject {
 public:
  explicit ArrangeActions(QObject* parent = nullptr);

  void subscribe();

  QAction* _STUDIO_INFORMATION{nullptr};
  QAction* _EDIT_STUDIOS{nullptr};
  QAction* _RELOAD_STUDIOS{nullptr};

  QAction* _PERFORMERS_INFORMATION{nullptr};
  QAction* _EDIT_PERFS{nullptr};
  QAction* _RELOAD_PERFS{nullptr};
  QAction* _AKA_PERFORMERS_INFORMATION{nullptr};
  QAction* _EDIT_PERF_AKA{nullptr};
  QAction* _RELOAD_PERF_AKA{nullptr};
};

ArrangeActions& g_ArrangeActions();

#endif  // ARRANGEACTIONS_H
