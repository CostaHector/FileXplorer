#ifndef RENAMEWIDGET_REPLACE_H
#define RENAMEWIDGET_REPLACE_H
#include "AdvanceRenamer.h"
#include <QComboBox>

class RenameWidget_Replace : public AdvanceRenamer {
 public:
  explicit RenameWidget_Replace(QWidget* parent = nullptr);

  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override;

  QStringList RenameCore(const QStringList& replaceeList) override;
  QString GetNewName() const { return m_newStrCB->currentText(); }
  void setOldNameAndNewName(const QString& oldName, const QString& newName);
  static std::pair<bool, QString> QueryAndConfirm(const QString& workPath,
                                                  const QStringList& selectedNames,  //
                                                  const QString& defOldName = "",
                                                  const QString& defNewName = "",  //
                                                  bool disableOldNameEdit = false);

 protected:
  QComboBox* m_newStrCB{nullptr};

 private:
  QComboBox* m_oldStrCB{nullptr};
  QCheckBox* m_regexCB{nullptr};
};

class RenameWidget_Delete : public RenameWidget_Replace {
 public:
  explicit RenameWidget_Delete(QWidget* parent = nullptr);
  void initExclusiveSetting() override;
  void InitExtraCommonVariable() override;
};

#endif  // RENAMEWIDGET_REPLACE_H
