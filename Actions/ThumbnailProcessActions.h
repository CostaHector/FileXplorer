#ifndef THUMBNAILPROCESSACTIONS_H
#define THUMBNAILPROCESSACTIONS_H

#include <QAction>
#include <QActionGroup>

class ThumbnailProcessActions : public QObject
{
 public:
  explicit ThumbnailProcessActions(QObject *parent = nullptr);

  QAction* _EXTRACT_1ST_IMG{nullptr};
  QAction* _EXTRACT_2ND_IMGS{nullptr};
  QAction* _EXTRACT_4TH_IMGS{nullptr};
  QAction* _CUSTOM_RANGE_IMGS{nullptr};
  QAction* _SKIP_IF_ALREADY_EXIST{nullptr};
};

ThumbnailProcessActions& g_ThumbnailProcessActions();


#endif // THUMBNAILPROCESSACTIONS_H
