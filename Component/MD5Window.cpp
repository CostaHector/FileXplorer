#include "MD5Window.h"
#include "Notificator.h"
#include "MD5Calculator.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QIcon>
#include <QMimeData>

MD5Window::MD5Window(QWidget* parent)
    : QDialog{parent}  //
{
  _ONLY_FIRST_BYTE = new (std::nothrow) QAction{"Only First Byte", this};
  _ONLY_FIRST_BYTE->setCheckable(true);
  _ONLY_FIRST_BYTE->setChecked(false);

  m_md5InfoTB = new (std::nothrow) QToolBar{"Extra Info", this};
  m_md5InfoTB->addAction(_ONLY_FIRST_BYTE);

  m_md5TextEdit = new (std::nothrow) QPlainTextEdit{this};

  mMainLayout = new (std::nothrow) QVBoxLayout(this);
  mMainLayout->addWidget(m_md5InfoTB);
  mMainLayout->addWidget(m_md5TextEdit);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
  setWindowTitle("MD5 Window");
  setWindowIcon(QIcon(":img/MD5_FILE_IDENTIFIER_PATH"));

  m_md5TextEdit->setAcceptDrops(false);
  //  m_md5TextEdit->installEventFilter(this);
  setAcceptDrops(true);

  ReadSetting();
}

void MD5Window::ReadSetting() {
  if (PreferenceSettings().contains("MD5WindowGeometry")) {
    restoreGeometry(PreferenceSettings().value("MD5WindowGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  m_md5TextEdit->setFont(StyleSheet::TEXT_EDIT_FONT);
}

void MD5Window::showEvent(QShowEvent* event) {
  QDialog::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void MD5Window::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("MD5WindowGeometry", saveGeometry());
  QDialog::closeEvent(event);
}

int MD5Window::operator()(const QStringList& absPaths) {
  const int firstByte{_ONLY_FIRST_BYTE->isChecked() ? 8 : 0};
  for (const QString& absPath : absPaths) {
    if (m_fileMD5Map.contains(absPath)) {
      continue;
    }
    const QFileInfo fi{absPath};
    if (!fi.isFile()) {
      continue;
    }
    const QString md5{MD5Calculator::GetFileMD5(absPath, firstByte)};
    m_fileMD5Map[absPath] = md5;
    m_md5TextEdit->appendPlainText(md5 + ':' + absPath);
  }
  setWindowTitle(QString("MD5 | %1 file(s)").arg(m_fileMD5Map.size()));
  return m_fileMD5Map.size();
}

void MD5Window::dropEvent(QDropEvent* event) {
  const QMimeData* pMimedata = event->mimeData();
  if (pMimedata == nullptr) {
    event->ignore();
    return;
  }
  if (!pMimedata->hasUrls()) {
    QDialog::dropEvent(event);
    return;
  }
  const QList<QUrl>& urls = pMimedata->urls();
  QStringList absPaths;
  absPaths.reserve(urls.size());
  for (const QUrl& url : urls) {
    absPaths.append(url.toLocalFile());
  }
  const int filesCnt = operator()(absPaths);
  LOG_GOOD("MD5 files count calculated", QString::number(filesCnt));
  event->accept();
}

void MD5Window::dragEnterEvent(QDragEnterEvent* event) {
  const QMimeData* pMimedata = event->mimeData();
  if (pMimedata == nullptr) {
    event->ignore();
    return;
  }
  if (!pMimedata->hasUrls()) {
    QDialog::dragEnterEvent(event);
    return;
  }
  event->accept();
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  MD5Window md5Window;
  md5Window.show();
  md5Window({__FILE__});
  a.exec();
  return 0;
}
#endif
