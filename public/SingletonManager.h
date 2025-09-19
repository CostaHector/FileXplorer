#ifndef SINGLETONMANAGER_H
#define SINGLETONMANAGER_H
#include <QString>

template <typename INST_T, typename DATA_T>
class SingletonManager {
 public:
  static INST_T& getInst() {
    static INST_T instance;
    return instance;
  }
  inline int count() const { return data().size(); }
  DATA_T& data() { return mInternalData; }
  const DATA_T& data() const { return mInternalData; }

  int ForceReload() {
    return static_cast<INST_T*>(this)->ForceReloadImpl();
  }

#ifdef RUNNING_UNIT_TESTS
  int ResetStateForTest(const QString& path) {
    return static_cast<INST_T*>(this)->ResetStateForTestImpl(path);
  }
#endif
 protected:
  void Initialize(const QString& path) {
    return static_cast<INST_T*>(this)->InitializeImpl(path);
  }

 private:
  DATA_T mInternalData;
};

#endif  // SINGLETONMANAGER_H
