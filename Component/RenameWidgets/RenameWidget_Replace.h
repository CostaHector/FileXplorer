#ifndef RENAMEWIDGET_REPLACE_H
#define RENAMEWIDGET_REPLACE_H
#include "AdvanceRenamer.h"
#include <QComboBox>

class RenameWidget_Replace : public AdvanceRenamer {
  Q_OBJECT
 public:
  using AdvanceRenamer::AdvanceRenamer;
  virtual ~RenameWidget_Replace();

  void InitExtraCommonVariable() override;
  QToolBar* InitControlTB() override;
  void extraSubscribe() override;
  void InitExtraMemberWidget() override;

  QStringList RenameCore(const QStringList& replaceeList) override;
  QString GetNewName() const { return m_newStrCB->currentText(); }
  void setOldNameAndNewName(const QString& oldName, const QString& newName);
  void setOldLineEditDisabled(bool bDisabled);

 protected:
  QComboBox* m_newStrCB{nullptr};

 private:
  void onOldStrDefaultModeChanged(int oldStrDefaultMode);
  bool UpdateOldStrCbCurrentText();

  QComboBox* m_oldStrCB{nullptr};
  QComboBox* m_oldStrDefaultModeCB{nullptr};
  QCheckBox* m_regexCB{nullptr};
  bool m_oldStrInited = false;
};

class RenameWidget_Delete : public RenameWidget_Replace {
 public:
  explicit RenameWidget_Delete(QWidget* parent = nullptr);
  void initExclusiveSetting() override;
  void InitExtraCommonVariable() override;
};

#endif  // RENAMEWIDGET_REPLACE_H
