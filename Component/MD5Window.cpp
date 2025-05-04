#include "MD5Window.h"
#include <QDir>
#include <QFileInfo>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>

#include "Tools/FileSystemItemFilter.h"
#include "Tools/MD5Calculator.h"

MD5Window::MD5Window(const QString& root, const QStringList& items, QWidget* parent)
    : QDialog(parent),
      m_root(root),
      m_items(items),
      m_md5FileName(QFileInfo(m_root).fileName() + ".md5"),
      m_md5TextEdit(new QPlainTextEdit(this)),
      m_buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Orientation::Horizontal, this)),
      m_incrementRefresh(new (std::nothrow) QAction(QIcon(":img/INCREMENTAL_CALCULATE"), tr("Incremental Calc"), this)),
      m_reloadFromFile(new (std::nothrow) QAction(tr("Reload"), this)),
      m_dumpMD5IntoFile(new (std::nothrow) QAction(tr("Dump"), this)),
      m_fullRefresh(new (std::nothrow) QAction(QIcon(":img/FULL_CALCULATE"), tr("Full Calc"), this)),
      m_md5InfoTB(new QToolBar("Extra Info", this)) {
  m_md5TextEdit->setReadOnly(true);
  m_md5TextEdit->setFont(QFont("Consolas"));

  m_md5InfoTB->setToolTip("Here you can choose to save/load the MD5 results into/from files. Or Recalculate MD5 Fully/Incrementally ");
  m_incrementRefresh->setToolTip("Only Calculate modified file or new added file MD5 since the last calculate");
  m_fullRefresh->setToolTip("Calculate the entire files MD5 regardless of whether any changes were made to the files");
  m_reloadFromFile->setToolTip("Reload MD5 from directly from *.md5");
  m_dumpMD5IntoFile->setToolTip("Dump into *.md5");

  m_md5InfoTB->addAction(m_incrementRefresh);
  m_md5InfoTB->addSeparator();
  m_md5InfoTB->addActions({m_reloadFromFile, m_dumpMD5IntoFile});
  m_md5InfoTB->addSeparator();
  m_md5InfoTB->addAction(m_fullRefresh);
  m_md5InfoTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  m_md5InfoTB->setOrientation(Qt::Orientation::Horizontal);

  auto* lo = new QVBoxLayout(this);
  lo->addWidget(m_md5InfoTB);
  lo->addWidget(m_md5TextEdit);
  lo->addWidget(m_buttonBox);
  setLayout(lo);

  subscribe();
  setWindowTitle(QString("MD5 | Direct: %1 item(s) | %2").arg(items.size()).arg(m_root));
  setWindowIcon(QIcon(":img/MD5_FILE_IDENTIFIER_PATH"));

  onLoadFromMD5Files();
}

bool MD5Window::onIncrementalCalculateMD5() {
  const auto& fileLst = FileSystemItemFilter::FilesOut(m_items);
  const int ROOT_PATH_LEN = m_root.size();
  QStringList toUpdateFileLst;
  for (int i = 0; i < fileLst.size(); ++i) {
    if (not m_fileMD5Map.contains(fileLst[i].mid(ROOT_PATH_LEN + 1))) {
      toUpdateFileLst.append(fileLst[i]);
    }
  }
  // erase not exists files
  QStringList toEraseFileList;
  QDir dir(m_root);
  for (const QString& relPath : m_fileMD5Map.keys()) {
    if (not dir.exists(relPath)) {
      toEraseFileList << relPath;
    }
  }
  for (const auto& eraseKey : toEraseFileList) {
    m_fileMD5Map.remove(eraseKey);
  }

  const auto& md5Lst = MD5Calculator::GetBatchFileMD5(toUpdateFileLst);
  setWindowTitle(
      QString("MD5 | +%1-%2/%3 file(s) update | %4").arg(toUpdateFileLst.size()).arg(toEraseFileList.size()).arg(fileLst.size()).arg(m_root));
  // add new files
  for (int i = 0; i < toUpdateFileLst.size(); ++i) {
    m_fileMD5Map[toUpdateFileLst[i].mid(ROOT_PATH_LEN + 1)] = md5Lst[i];
  }

  m_md5TextEdit->setPlainText("");
  for (auto it = m_fileMD5Map.cbegin(); it != m_fileMD5Map.cend(); ++it) {
    m_md5TextEdit->appendPlainText(it.value() + ':' + it.key());
  }
  return true;
}

bool MD5Window::onFullCalculateMD5() {
  const auto& fileLst = FileSystemItemFilter::FilesOut(m_items);
  setWindowTitle(QString("MD5 | +%1-0/%2 file(s) update | %3").arg(fileLst.size()).arg(fileLst.size()).arg(m_root));

  const auto& md5Lst = MD5Calculator::GetBatchFileMD5(fileLst);
  const int ROOT_PATH_LEN = m_root.size();
  decltype(m_fileMD5Map) tempForRelease;
  m_fileMD5Map.swap(tempForRelease);
  m_md5TextEdit->setPlainText("");
  for (int i = 0; i < fileLst.size(); ++i) {
    m_fileMD5Map[fileLst[i].mid(ROOT_PATH_LEN + 1)] = md5Lst[i];
    m_md5TextEdit->appendPlainText(md5Lst[i] + ':' + fileLst[i].mid(ROOT_PATH_LEN + 1));
  }
  return true;
}

bool MD5Window::onLoadFromMD5Files() {
  QFile file(m_root + '/' + m_md5FileName);
  if (!file.exists()) {
    qWarning("File[%s] not exists", qPrintable(m_md5FileName));
    return false;
  }
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning("File[%s] open for read failed", qPrintable(m_md5FileName));
    return false;
  }
  QTextStream stream(&file);
  stream.setCodec("UTF-8");
  static constexpr int MD5_LEN = 32;
  QString contents;
  while (!stream.atEnd()) {
    const QString& line = stream.readLine();
    contents += line;
    m_fileMD5Map[line.mid(MD5_LEN + 1)] = line.left(MD5_LEN);
  }
  file.close();
  m_md5TextEdit->setPlainText(contents);
  return true;
}

bool MD5Window::onDumpIntoMD5Files() {
  QFile file(m_root + '/' + m_md5FileName);
  if (not file.exists()) {
    qInfo("File[%s] not exists, will create it.", qPrintable(m_md5FileName));
  }
  if (not file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning("File[%s] open for read failed", qPrintable(m_md5FileName));
    return false;
  }
  QTextStream stream(&file);
  stream << m_md5TextEdit->toPlainText();
  file.close();
  return true;
}

void MD5Window::subscribe() {
  connect(m_reloadFromFile, &QAction::triggered, this, &MD5Window::onLoadFromMD5Files);
  connect(m_dumpMD5IntoFile, &QAction::triggered, this, &MD5Window::onDumpIntoMD5Files);

  connect(m_incrementRefresh, &QAction::triggered, this, &MD5Window::onIncrementalCalculateMD5);
  connect(m_fullRefresh, &QAction::triggered, this, &MD5Window::onFullCalculateMD5);

  connect(m_buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &QDialog::accept);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  // MD5Window md5Window({__FILE__});
  const QString& root = QFileInfo(__FILE__).absolutePath();
  MD5Window md5Window(root, {root});
  md5Window.show();
  a.exec();
  return 0;
}
#endif
