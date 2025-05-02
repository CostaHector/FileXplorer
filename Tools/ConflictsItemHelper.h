#ifndef CONFLICTSITEMHELPER_H
#define CONFLICTSITEMHELPER_H

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QPair>
#include <QSet>
#include <QString>
#include <QStringList>
#include "public/PublicVariable.h"
#include "public/PathTool.h"

class Finder {
 public:
  explicit Finder(CCMMode::Mode mode_) : m_mode{mode_} {}

  QStringList FindAllItems(const QString& l, const QStringList& lRels) const;

  QStringList FindLLRelRCommon(const QString& l, const QStringList& lRels, const QString& r) const;

 private:
  bool isLink() const { return m_mode == CCMMode::LINK_OP; }
  bool isMove() const { return m_mode == CCMMode::MERGE_OP || m_mode == CCMMode::CUT_OP || m_mode == CCMMode::LINK_OP; }
  CCMMode::Mode m_mode;
};

class ConflictsItemHelper {
 public:
  QString l;
  QStringList lRels;
  QString r;
  CCMMode::Mode m_mode;
  Finder m_finder;
  QStringList commonList;
  QStringList m_fromPathItems;

  explicit ConflictsItemHelper(const QString& l_, const QString& r_, const QStringList& lRels_, const CCMMode::Mode mode)
      : l(l_), lRels(lRels_), r(r_), m_mode{mode}, m_finder{mode}, commonList(m_finder.FindLLRelRCommon(l_, lRels_, r_)) {
    saveLeftRelPathList();
  }

  explicit ConflictsItemHelper(const std::pair<QString, QStringList>& lAndRels, const QString& r_, const CCMMode::Mode mode)
      : l(lAndRels.first), lRels(lAndRels.second), r(r_), m_mode{mode}, m_finder{mode}, commonList(m_finder.FindLLRelRCommon(l, lRels, r_)) {
    saveLeftRelPathList();
  }

  explicit ConflictsItemHelper(const QString& l_, const QString& r_, const CCMMode::Mode mode)
      : ConflictsItemHelper(l_, r_, QDir(l_, "", QDir::SortFlag::NoSort, QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot).entryList(), mode) {
  }

  explicit ConflictsItemHelper(const QStringList& lAbsPathList, const QString& r_, const CCMMode::Mode mode)
      : ConflictsItemHelper(PATHTOOL::GetLAndRels(lAbsPathList), r_, mode) {}

  operator bool() const { return not commonList.isEmpty(); }
  void saveLeftRelPathList() {
    if (not bool(*this)) {
      // if no conflict and only move items, just rename/copy it
      m_fromPathItems = lRels;
      return;
    }
    m_fromPathItems = m_finder.FindAllItems(l, lRels);
  }
  QStringList GetLeftRelPathList() const { return m_fromPathItems; }
};
#endif  // CONFLICTSITEMHELPER_H
