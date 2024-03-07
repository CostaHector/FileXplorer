#ifndef FILESYSTEMTYPEFILTER_H
#define FILESYSTEMTYPEFILTER_H

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QFileSystemModel>
#include <QMenu>
#include <QStyle>
#include <QToolButton>

#include "AdvanceSearchModel.h"
#include "Tools/SearchProxyModel.h"

class FileSystemTypeFilter : public QToolButton {
  Q_OBJECT
 public:
  FileSystemTypeFilter(QWidget* parent = nullptr);
  void BindFileSystemModel(QFileSystemModel* newModel);
  void BindFileSystemModel(AdvanceSearchModel* newModel, SearchProxyModel* newProxyModel);
  void onSwitchChanged(bool isOn);
  void onGrayOrHideChanged(bool isGray);
  void onTypeChecked(QAction* act);

  QDir::Filters filterAgent() const;
  void initFilterAgent(QDir::Filters filters);
  void setFilterAgent(QDir::Filters filters);

  void initNameFilterDisablesAgent(bool enable);
  void setNameFilterDisablesAgent(bool enable);

  void initSearchModelIteratorFlagAgent();
  void changeSearchModelIteratorFlagAgent(const bool including);

  static const QDir::Filters DEFAULT_FILTER_FLAG;

 private:
  QAction* FILTER_SWITCH = new QAction(QIcon(":/themes/FILE_SYSTEM_FILTER"), "Filter", this);

  QMenu* fileTypeFilterMenu = new QMenu(tr("Filter Menu"), this);
  QAction* FILES = new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_FileIcon), "Files", fileTypeFilterMenu);
  QAction* DIRS = new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DirIcon), "Dirs", fileTypeFilterMenu);
  QAction* DRIVES = new QAction(QApplication::style()->standardIcon(QStyle::StandardPixmap::SP_DriveHDIcon), "Drives", fileTypeFilterMenu);
  QAction* HIDDEN = new QAction(QIcon(":/themes/HIDDEN"), "Hidden", fileTypeFilterMenu);
  QAction* NO_DOT = new QAction(QIcon(":/themes/NO_DOT"), "NoDot", fileTypeFilterMenu);
  QAction* NO_DOT_DOT = new QAction(QIcon(":/themes/NO_DOT_DOT"), "NoDotDot", fileTypeFilterMenu);
  QAction* CASE_SENSITIVE = new QAction(QIcon(":/themes/CASE_SENSITIVE"), "Case Sensitive", fileTypeFilterMenu);

  //  QAction* _IMAGES = new QAction(QIcon(":/themes/IMAGE"), "image");
  //  QAction* _VIDEOS = new QAction(QIcon(":/themes/VIDEO"), "video");
  //  QAction* _PLAIN_TEXT = new QAction(QIcon(":/themes/PLAIN_TEXT"), "plain Text");
  //  QAction* _DOCUMENT = new QAction(QIcon(":/themes/DOCUMENT"), "document");
  //  QAction* _EXE = new QAction(QIcon(":/themes/EXE"), "executable");

  QAction* HIDE_ENTRIES_DONT_PASS_FILTER = new QAction(tr("hide/disable the entries don't pass name filter"), fileTypeFilterMenu);
  QAction* INCLUDING_SUBDIRECTORIES = new QAction(tr("Including Subdirs"), fileTypeFilterMenu);

  QActionGroup* m_FILTER_FLAG_AGS = new QActionGroup(this);

  QHash<QString, QDir::Filter> m_text2FilterFlag = {{FILES->text(), QDir::Files},
                                                    {DIRS->text(), QDir::Dirs},
                                                    {DRIVES->text(), QDir::Drives},
                                                    {HIDDEN->text(), QDir::Hidden},
                                                    {NO_DOT->text(), QDir::NoDot},
                                                    {NO_DOT_DOT->text(), QDir::NoDotDot},
                                                    {CASE_SENSITIVE->text(), QDir::CaseSensitive}};

  QFileSystemModel* _fsmModel{nullptr};
  AdvanceSearchModel* _searchSourceModel{nullptr};
  SearchProxyModel* _searchProxyModel{nullptr};

  enum MODEL_TYPE { INVALID_MODEL = 0, FILE_SYSTEM_MODEL = 1, SEARCH_MODEL = 2 };
  MODEL_TYPE m_modelType = MODEL_TYPE::INVALID_MODEL;
  QDir::Filters m_flagWhenFilterEnabled;

  bool m_isIncludingSubdirectory;
};

#endif  // FILESYSTEMTYPEFILTER_H
