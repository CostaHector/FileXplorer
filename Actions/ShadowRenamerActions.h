#ifndef SHADOWRENAMERACTIONS_H
#define SHADOWRENAMERACTIONS_H

#include <QObject>
#include <QAction>

class ShadowRenamerActions : public QObject {
  Q_OBJECT
public:
  static ShadowRenamerActions& GetInst();
  QWidget* GetShadowFilesToolBar(QWidget* notNullParent) const;
  QWidget* GetShadowFilesRecycleToolBar(QWidget* notNullParent) const;

  QAction* CREATE_STAGING_FILES{nullptr}, *SYNC_STAGING_TO_VIDEO{nullptr};
  QAction* SHOW_STAGING_STATISTICS{nullptr};
  QAction* RECYCLE_SYNCED_STAGING_FILE{nullptr}, *RECYCLE_NO_NEED_SYNC_FILE{nullptr};

  bool isOpenSourceOnDoubleClick() const { return m_bOpenSourceOnDoubleClick; }
private:
  explicit ShadowRenamerActions(QObject* parent = nullptr);
  QAction* OPEN_SOURCE_ON_DOUBLE_CLICK{nullptr};
  bool m_bOpenSourceOnDoubleClick;
};

#endif // SHADOWRENAMERACTIONS_H
