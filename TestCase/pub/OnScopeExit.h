#ifndef ONSCOPEEXIT_H
#define ONSCOPEEXIT_H

#define LAMBDA_OBJECT_NAME(name, lineNum) name##lineNum

template <typename Func>
class OnScopeExit {
 public:
  OnScopeExit() = default;
  ~OnScopeExit() {
     m_func();
  }
  Func m_func;
};

enum class PLACEHOLDER_E {};

template <typename Func>
OnScopeExit<Func> operator+(PLACEHOLDER_E, Func func) {
  return OnScopeExit<Func>{func};
}

#define ON_SCOPE_EXIT auto LAMBDA_OBJECT_NAME(exitor, __LINE__) = PLACEHOLDER_E{} + [&]()

#endif  // ONSCOPEEXIT_H
