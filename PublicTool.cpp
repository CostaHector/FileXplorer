#include "PublicTool.h"
#include "PublicVariable.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QSqlError>

void SetLayoutAlightment(QLayout* lay, const Qt::AlignmentFlag align) {
  for (int i = 0; i < lay->count(); ++i) {
    lay->itemAt(i)->setAlignment(align);
  }
  // Only QToolBar and QToolButton need to set alignment. (QWidget like QSeperator not need)
}

QString ChooseCopyDestination(QString defaultPath, QWidget* parent) {
  if (!QFileInfo(defaultPath).isDir()) {
    defaultPath = PreferenceSettings().value(MemoryKey::PATH_LAST_TIME_COPY_TO.name).toString();
  }
  const auto selectPath = QFileDialog::getExistingDirectory(parent, "Choose a destination", defaultPath);
  QFileInfo dstFi(selectPath);
  if (!dstFi.isDir()) {
    qDebug("selectPath[%s] is not a directory", qPrintable(selectPath));
    return "";
  }
  PreferenceSettings().setValue(MemoryKey::PATH_LAST_TIME_COPY_TO.name, dstFi.absoluteFilePath());
  return dstFi.absoluteFilePath();
}

QString MoveCopyToRearrangeActionsText(const QString& first_path, QActionGroup* oldAG) {
  if (oldAG == nullptr) {
    qCritical("oldAG is nullptr");
    return "";
  }
  QString i_1_path = first_path;        // first and (i-1) path
  for (auto* act : oldAG->actions()) {  // i path
    QString i_path = act->text();
    if (i_path == first_path) {
      act->setText(i_1_path);  // finish
      break;
    }
    act->setText(i_1_path);
    i_1_path = i_path;
  }
  QString actionsStr;
  for (auto* act : oldAG->actions()) {
    actionsStr += (act->text() + '\n');
  }
  if (!actionsStr.isEmpty()) {
    actionsStr.chop(1);
  }
  return actionsStr;
}

QSqlDatabase GetSqlVidsDB() {
  if (QSqlDatabase::connectionNames().contains("DBMOVIE_CONNECT")) {
    auto db = QSqlDatabase::database("DBMOVIE_CONNECT");
    db.open();
    return db;
  }
  auto db = QSqlDatabase::addDatabase("QSQLITE", "DBMOVIE_CONNECT");
  db.setDatabaseName(SystemPath::VIDS_DATABASE);
  db.open();
  return db;
}


void LoadCNLanguagePack(QTranslator& translator) {
  qDebug("Load Chinese pack");
  const QString baseName = "FileExplorerReadOnly_zh_CN";
  if (translator.load(":/i18n/" + baseName)) {
    qDebug("Load language pack succeed %s", qPrintable(baseName));
    QCoreApplication::installTranslator(&translator);
  }
}

void LoadSysLanaguagePack(QTranslator& translator) {
  qDebug("Load System Language pack");
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString& locale : uiLanguages) {
    const QString baseName = "FileExplorerReadOnly" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      qDebug("Load language pack succeed %s", qPrintable(baseName));
      QCoreApplication::installTranslator(&translator);
      break;
    } else {
      qDebug("No need to load language pack %s", qPrintable(baseName));
    }
  }
}
