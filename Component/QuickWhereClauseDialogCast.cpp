#include "QuickWhereClauseDialogCast.h"
#include "Notificator.h"
#include "CastPsonFileHelper.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "CastAkasManager.h"

void QuickWhereClauseDialogCast::CreatePrivateWidget() {
  using namespace PERFORMER_DB_HEADER_KEY;
  m_Name = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Name), this}; // shared
  CHECK_NULLPTR_RETURN_VOID(m_Name);
  m_Name->setToolTip(                     //
      "Henry Cavill&Chris Evans\n"        //
      "A: search 1 person\n"              //
      "A & B: search 2 person BOTH\n"     //
      "A1 | A2: search 2 person EITHER\n" //
      "(A1|A2)&B: search 2 person BOTH(One person also known as A2)");

  m_Rate = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Rate), this};
  CHECK_NULLPTR_RETURN_VOID(m_Rate);
  m_Rate->setToolTip(R"(>8)");

  m_Ori = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Ori), this};
  CHECK_NULLPTR_RETURN_VOID(m_Ori);
  m_Ori->setToolTip(R"(Orientations here)");

  m_Tags = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Tags), this};
  CHECK_NULLPTR_RETURN_VOID(m_Tags);
  m_Tags->setToolTip(R"(Superhero|Documentary)");

  m_Height = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Height), this};
  CHECK_NULLPTR_RETURN_VOID(m_Height);
  m_Height->setToolTip(R"(Height(cm) here)");

  m_Size = new (std::nothrow) ColumnFilterLineEdit{ENUM_2_STR(Size), this};
  CHECK_NULLPTR_RETURN_VOID(m_Size);
  m_Size->setToolTip(">1000000000&<1500000000");

  AppendColumnFilterLineEdit(m_Name);
  AppendColumnFilterLineEdit(m_Tags);
  AppendColumnFilterLineEdit(m_Rate);
  AppendColumnFilterLineEdit(m_Ori);
  AppendColumnFilterLineEdit(m_Height);
  AppendColumnFilterLineEdit(m_Size);

  m_Name->setFocus();
  setWindowTitle(QString{"SELECT * FROM `%1` WHERE ?"}.arg(DB_TABLE::PERFORMERS));
}
