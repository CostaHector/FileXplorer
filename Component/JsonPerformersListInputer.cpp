#include "JsonPerformersListInputer.h"
#include <QFile>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QCompleter>
#include <QStringListModel>
#include "Tools/Json/JsonHelper.h"
#include "Tools/NameTool.h"
#include "Tools/CastManager.h"
#include "public/PublicMacro.h"

JsonPerformersListInputer::JsonPerformersListInputer(QWidget* parent, Qt::WindowFlags f)  //
    : QDialog{parent, f},                                                                 //
      mPerfsCompleter{CastManager::getIns().m_performers.values()}                        //
{
  m_onePerf = new (std::nothrow) QLineEdit;
  CHECK_NULLPTR_RETURN_VOID(m_onePerf);
  m_perfsList = new (std::nothrow) QLineEdit;
  CHECK_NULLPTR_RETURN_VOID(m_perfsList);
  buttonBox = new (std::nothrow) QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  CHECK_NULLPTR_RETURN_VOID(buttonBox);

  mPerfsCompleter.setCaseSensitivity(Qt::CaseInsensitive);
  mPerfsCompleter.setCompletionMode(QCompleter::CompletionMode::PopupCompletion);

  m_onePerf->setCompleter(&mPerfsCompleter);
  m_onePerf->addAction(QIcon(":img/RENAME_PERFORMERS"), QLineEdit::LeadingPosition);
  m_onePerf->setClearButtonEnabled(true);
  m_perfsList->setClearButtonEnabled(true);

  lo = new QFormLayout;
  lo->addRow("performer", m_onePerf);
  lo->addRow("list", m_perfsList);
  lo->addWidget(buttonBox);
  setLayout(lo);

  subscribe();
  setWindowIcon(QIcon(":img/RENAME_PERFORMERS"));
  setWindowTitle("Mod performers");
}

QSize JsonPerformersListInputer::sizeHint() const {  //
  return QSize(600, 100);
}

bool JsonPerformersListInputer::appendAPerformer() {
  const QString& perf = m_onePerf->text().trimmed();
  if (perf.isEmpty()) {
    return true;
  }
  QString perfStr = text();
  perfStr += NameTool::CSV_COMMA;
  perfStr += perf;
  m_perfsList->setText(NameTool::CastTagSentenceParse2Str(perfStr, true));
  return true;
}

void JsonPerformersListInputer::uniquePerformers() {
  const QString newCastStr = NameTool::CastTagSentenceParse2Str(text(), true);
  m_perfsList->setText(newCastStr);
}

bool JsonPerformersListInputer::submitPerformersListToJsonFile() {
  const QString& jsonFilePath = windowFilePath();
  if (p_dict == nullptr) {
    qDebug("Cannot submit. dict is nullptr");
    return false;
  }
  QVariantHash& dict = *p_dict;
  if (!dict.contains(ENUM_2_STR(Cast))) {
    return false;
  }
  const QString& perfs = text();
  dict[ENUM_2_STR(Cast)] = NameTool()(perfs);
  return JsonHelper::DumpJsonDict(dict, jsonFilePath);
}

bool JsonPerformersListInputer::reloadPerformersFromJsonFile(const QString& jsonFilePath, QVariantHash& dict) {
  p_dict = &dict;
  if (!QFile::exists(jsonFilePath)) {
    setWindowFilePath("");
    qWarning("Error path[%s] not exist", qPrintable(jsonFilePath));
    return false;
  }
  setWindowFilePath(jsonFilePath);
  auto castIt = dict.constFind(ENUM_2_STR(Cast));
  if (castIt == dict.cend()) {
    qDebug("No cast key in json[%s]", qPrintable(jsonFilePath));
    return false;
  }
  const QStringList& casts = castIt.value().toStringList();
  m_perfsList->setText(casts.join(NameTool::CSV_COMMA));
  return true;
}

void JsonPerformersListInputer::subscribe() {
  connect(m_onePerf, &QLineEdit::editingFinished, this, &JsonPerformersListInputer::appendAPerformer);
  connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [this]() {
    bool ret = JsonPerformersListInputer::submitPerformersListToJsonFile();
    qDebug("Save mod performers result: %d", ret);
    hide();
  });
  connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &JsonPerformersListInputer::hide);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  PerformersListInputer pw;
  pw.show();
  a.exec();
  return 0;
}
#endif
