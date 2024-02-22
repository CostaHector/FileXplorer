#ifndef PUBLICTOOL_H
#define PUBLICTOOL_H

#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QLayout>
#include <QString>
#include <QSqlDatabase>
#include <QTranslator>


class OSWalker_RETURN {
 public:
  QStringList relToNames;
  QStringList completeNames;
  QStringList suffixs;
  QList<bool> isFiles;
};
OSWalker_RETURN OSWalker(const QString& pre, const QStringList& rels, const bool includingSub = false, const bool includingSuffix = false);

class PublicTool {
 public:
  static bool copyDirectoryFiles(const QString& fromDir, const QString& toDir, bool coverFileIfExist = false);
};

auto FindQActionFromQActionGroupByActionName(const QString& actionName, QActionGroup* ag) -> QAction*;
void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align);

auto Walker(const QString& preUserInput, const QStringList& rels) -> QStringList;

auto ChooseCopyDestination(QString defaultPath, QWidget *parent=nullptr) -> QString;
auto MoveCopyToRearrangeActionsText(const QString& first_path, QActionGroup* oldAG) -> QString;


auto GetSqlVidsDB() -> QSqlDatabase;


void LoadCNLanguagePack(QTranslator& translator);

void LoadSysLanaguagePack(QTranslator& translator);

#endif  // PUBLICTOOL_H
