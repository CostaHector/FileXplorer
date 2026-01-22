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

 protected:
  void Initialize(const QString& path, const QString& blackPath="") {
    return static_cast<INST_T*>(this)->InitializeImpl(path, blackPath);
  }

 private:
  DATA_T mInternalData;
};

#endif  // SINGLETONMANAGER_H
