#include "SplitterInsertIndexHelper.h"
#include <QDebug>
SplitterInsertIndexHelper::SplitterInsertIndexHelper(int maxCount) : m_maxCount{maxCount} {
  if (m_maxCount <= 0 || m_maxCount > 100) {
    qWarning("maxCount[%d] out of bound[0, 100)", m_maxCount);
    return;
  }
  m_occupied = new (std::nothrow) bool[m_maxCount]{false};
  if (m_occupied == nullptr) {
    qCritical("m_occupied is nullptr");
    return;
  }
}

SplitterInsertIndexHelper::~SplitterInsertIndexHelper() {
  if (m_occupied == nullptr) {
    return;
  }
  delete[] m_occupied;
  m_occupied = nullptr;
}

int SplitterInsertIndexHelper::operator()(int sequence) {
  if (m_occupied == nullptr) {
    qCritical("m_occupied is nullptr");
    return -1;
  }
  if (sequence < 0 && sequence >= m_maxCount) {
    qWarning("sequence[%d] out of bound[0, %d)", sequence, m_maxCount);
    return -1;
  }
  if (m_occupied[sequence]) {
    qWarning("sequence[%d] is already occupied", sequence);
    return -2;
  }
  // _____ _____
  int ansIndex = std::accumulate<bool*>(m_occupied, m_occupied + sequence, 0);
  m_occupied[sequence] = true;
  return ansIndex;
}

int SplitterInsertIndexHelper::GetOccupiedCnt() const {
  if (m_occupied == nullptr) {
    qCritical("m_occupied is nullptr");
    return -1;
  }
  return std::accumulate<bool*>(m_occupied, m_occupied + m_maxCount, 0);
}
