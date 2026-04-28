#ifndef LINEEDITCOLOR_H
#define LINEEDITCOLOR_H

#include "LineEditGeneral.h"

class LineEditColor : public LineEditGeneral {
public:
  using LineEditGeneral::LineEditGeneral;

protected:
  void onActionTriggered() override;
};

#endif // LINEEDITCOLOR_H
