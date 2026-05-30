#ifndef MULTIPAR2ACTIONS_H
#define MULTIPAR2ACTIONS_H

#include <QObject>
#include <QAction>

class MultiPar2Actions : public QObject {
  Q_OBJECT
public:
  static MultiPar2Actions& GetInst();
  explicit MultiPar2Actions(QObject* parent = nullptr);
  QWidget* GetToolBar(QWidget* parent);
  QAction* _VERIFY_IF_NEED_RECOVERY{nullptr};

signals:
  void createPar2Req(int rateOfRedundancy);

private:
  void subscribe();
  QAction* newCreateAction(int rateOfRedundancy);
  void EmitCreatePar2Req(const QAction* pCreatePar2Act);
  void EmitCreatePar2CustomReq();

  QAction                              //
      *_CREATE_PAR2_FILES_10{nullptr}, //
      *_CREATE_PAR2_FILES_20{nullptr}, //
      *_CREATE_PAR2_FILES_30{nullptr}, //
      *_CREATE_PAR2_FILES_40{nullptr}; //
  QActionGroup* CREATE_PAR2_AG{nullptr};

  QAction* _CREATE_PAR2_FILES_CUSTOM{nullptr};
};

#endif // MULTIPAR2ACTIONS_H
