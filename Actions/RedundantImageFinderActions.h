#ifndef REDUNDANTIMAGEFINDERACTIONS_H
#define REDUNDANTIMAGEFINDERACTIONS_H

#include <QAction>
#include <QActionGroup>

class RedundantImageFinderActions : public QObject {
 public:
  explicit RedundantImageFinderActions(QObject* parent = nullptr);

  QAction* FIND_DUPLICATE_IMGS_BY_LIBRARY{nullptr};
  QAction* FIND_DUPLICATE_IMGS_IN_A_PATH{nullptr};
  QActionGroup* FIND_DUPLICATE_IMGS_AG{nullptr};

  QAction* RECYLE_NOW{nullptr};
  QAction* ALSO_EMPTY_IMAGE{nullptr};
  QAction* OPEN_REDUNDANT_IMAGES_FOLDER{nullptr};
};

RedundantImageFinderActions& g_redunImgFinderAg();

#endif  // REDUNDANTIMAGEFINDERACTIONS_H
