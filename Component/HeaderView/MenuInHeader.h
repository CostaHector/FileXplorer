#ifndef MENUINHEADER_H
#define MENUINHEADER_H

#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include "EnumIntAction.h"

extern template struct EnumIntAction<QHeaderView::ResizeMode>;

class MenuInHeader : public QHeaderView {
  Q_OBJECT
public:
  explicit MenuInHeader(const QString& proName, Qt::Orientation orientation, QWidget* parent);
  ~MenuInHeader();

  const QString& GetName() const { return m_name; }
  void AddActionToMenu(QAction* act);
  void RestoreHeaderState();
  bool checkAndSetDefaultSectionSize(int newSectionSize);

protected:
  void contextMenuEvent(QContextMenuEvent* e) override;

  int GetClickedSection() const { return m_clickedColumn; }
  void InvalidateClickedSection() const { setClickedSection(INVALID_CLICKED_COLUMN); }

private:
  void setClickedSection(int newClickedSection) const { m_clickedColumn = newClickedSection; }

  bool onSetSectionResizeMode(const QAction* pResizeToContent);

  bool onSetDefaultSectionSize();

  bool onSetMaxSectionSize();

  int GetDefaultSectionSizeFirstTime() const;
  int GetMaxSectionSizeFirstTime() const;

  QAction* _SET_DEFAULT_SECTION_SIZE{nullptr};
  QAction* _SET_MAX_SECTION_SIZE{nullptr};
  QAction* _STRETCH_DETAIL_SECTION{nullptr};

  QAction* _RESIZE_MODE_INTERACTIVE{nullptr};
  QAction* _RESIZE_MODE_STRETCH{nullptr};
  QAction* _RESIZE_MODE_FIXED{nullptr};
  QAction* _RESIZE_MODE_RESIZE_TO_CONTENTS{nullptr};
  EnumIntAction<QHeaderView::ResizeMode> mResizeModeIntAction;
  QMenu* m_menu{nullptr};

  const QString m_name;
  const QString m_defaultSectionSizeKey;
  const QString m_maxSectionSizeKey;
  const QString m_stretchLastSectionKey;
  const QString m_resizeModeKey;
  const QString m_headerStateKey;

  mutable int m_clickedColumn{INVALID_CLICKED_COLUMN};

  static constexpr int INVALID_CLICKED_COLUMN = -1;
};

#endif
