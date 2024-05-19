#include "DuplicateVideosFinderActions.h"

#include <QLineEdit>

#include "PublicVariable.h"

DuplicateVideosFinderActions& g_dupVidFinderAg() {
  static DuplicateVideosFinderActions ins;
  return ins;
}

QToolBar* DuplicateVideosFinderActions::getToolBar(QWidget* parent) {
  QToolBar* m_tb{new QToolBar{"Duplicator finder toolbar", parent}};
  m_tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  m_tb->addAction(APPEND_A_PATH);
  m_tb->addSeparator();
  m_tb->addAction(DIFFER_BY_DURATION);
  if (durationDevLE == nullptr) {
    int durDev =
        PreferenceSettings().value(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name, MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.v).toInt();
    durationDevLE = new QLineEdit{QString::number(durDev), m_tb};
  }
  m_tb->addWidget(durationDevLE);
  m_tb->addSeparator();
  m_tb->addAction(DIFFER_BY_SIZE);
  if (sizeDevLE == nullptr) {
    int szDev =
        PreferenceSettings().value(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.v).toInt();
    sizeDevLE = new QLineEdit{QString::number(szDev), m_tb};
  }
  m_tb->addWidget(sizeDevLE);
  m_tb->addSeparator();
  m_tb->addAction(RECYCLE_ONE_FILE);
  m_tb->addSeparator();
  return m_tb;
}
