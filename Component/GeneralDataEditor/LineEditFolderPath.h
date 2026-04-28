#ifndef LINEEDITFOLDERPATH_H
#define LINEEDITFOLDERPATH_H

#include "LineEditPathRelated.h"

class LineEditFolderPath : public LineEditPathRelated {
public:
  using LineEditPathRelated::LineEditPathRelated;

protected:
  void onActionTriggered() override;
};

#endif // LINEEDITFOLDERPATH_H
