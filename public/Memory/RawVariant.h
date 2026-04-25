#ifndef RAWVARIANT_H
#define RAWVARIANT_H

namespace RawVariant {
enum class VarType : unsigned char {
  Empty,
  Int,
  Bool,
  Float,
  Double,
  Char,
  LongLong,
  CStrLit // const char* 字符串字面量指针
};

union VarUnion {
  int i;
  bool b;
  float f;
  double d;
  char c;
  long long ll;
  const char* str;

  constexpr VarUnion()
    : i(0) {}
  constexpr VarUnion(int v)
    : i(v) {}
  constexpr VarUnion(bool v)
    : b(v) {}
  constexpr VarUnion(float v)
    : f(v) {}
  constexpr VarUnion(double v)
    : d(v) {}
  constexpr VarUnion(char v)
    : c(v) {}
  constexpr VarUnion(long long v)
    : ll(v) {}
  constexpr VarUnion(const char* s)
    : str(s) {}
};

struct Var {
  VarType type;
  VarUnion data;

  constexpr Var()
    : type(VarType::Empty)
    , data() {}

  constexpr Var(int v)
    : type(VarType::Int)
    , data(v) {}

  constexpr Var(bool v)
    : type(VarType::Bool)
    , data(v) {}

  constexpr Var(float v)
    : type(VarType::Float)
    , data(v) {}

  constexpr Var(double v)
    : type(VarType::Double)
    , data(v) {}

  constexpr Var(char v)
    : type(VarType::Char)
    , data(v) {}

  constexpr Var(long long v)
    : type(VarType::LongLong)
    , data(v) {}

  // 字符串字面量指针 —— 同样 constexpr
  constexpr Var(const char* s)
    : type(VarType::CStrLit)
    , data(s) {}

  // constexpr 类型判断接口
  constexpr bool isEmpty() const { return type == VarType::Empty; }
  constexpr bool isInt() const { return type == VarType::Int; }
  constexpr bool isBool() const { return type == VarType::Bool; }
  constexpr bool isFloat() const { return type == VarType::Float; }
  constexpr bool isDouble() const { return type == VarType::Double; }
  constexpr bool isChar() const { return type == VarType::Char; }
  constexpr bool isLongLong() const { return type == VarType::LongLong; }
  constexpr bool isCStr() const { return type == VarType::CStrLit; }
};

}

#endif // RAWVARIANT_H
