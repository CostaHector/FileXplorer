#include "WindowsSystemVolumeMixer.h"

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>

// 设置当前进程的系统音频会话静音状态
// 返回true表示设置成功，false表示未找到音频会话（通常是还没播放过音频）
bool setSystemAppMute(bool mute)
{
  // 初始化COM环境，重复初始化也不会报错
  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  bool coInited = (hr == S_OK);
  if (hr != S_OK && hr != RPC_E_CHANGED_MODE) {
    return false;
  }

  IMMDeviceEnumerator* pEnum = nullptr;
  IMMDevice* pDevice = nullptr;
  IAudioSessionManager2* pSessMgr = nullptr;
  IAudioSessionEnumerator* pSessEnum = nullptr;
  bool success = false;
  DWORD curPid = GetCurrentProcessId();

  auto cleanUpLambda = [&pSessEnum, &pSessMgr, &pDevice, &pEnum, &coInited, &success]() -> bool {
    if (pSessEnum) pSessEnum->Release();
    if (pSessMgr) pSessMgr->Release();
    if (pDevice) pDevice->Release();
    if (pEnum) pEnum->Release();
    if (coInited) CoUninitialize();
    return success;
  };

  // 1. 创建设备枚举器
  hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr,
                        CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                        (void**)&pEnum);
  if (FAILED(hr)) return cleanUpLambda();

  // 2. 获取默认音频输出设备
  hr = pEnum->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
  if (FAILED(hr)) return cleanUpLambda();

  // 3. 激活音频会话管理器
  hr = pDevice->Activate(__uuidof(IAudioSessionManager2),
                         CLSCTX_ALL, nullptr, (void**)&pSessMgr);
  if (FAILED(hr)) return cleanUpLambda();

  // 4. 枚举所有音频会话
  hr = pSessMgr->GetSessionEnumerator(&pSessEnum);
  if (FAILED(hr)) return cleanUpLambda();

  int count = 0;
  pSessEnum->GetCount(&count);

  // 5. 找到当前进程的会话，设置静音
  for (int i = 0; i < count; ++i) {
    IAudioSessionControl* pCtrl = nullptr;
    IAudioSessionControl2* pCtrl2 = nullptr;
    ISimpleAudioVolume* pVol = nullptr;

    if (FAILED(pSessEnum->GetSession(i, &pCtrl))) continue;
    if (FAILED(pCtrl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pCtrl2))) {
      pCtrl->Release();
      continue;
    }
    pCtrl->Release();

    DWORD sessPid = 0;
    pCtrl2->GetProcessId(&sessPid);
    if (sessPid != curPid) {
      pCtrl2->Release();
      continue;
    }

    // 匹配到当前进程，执行静音设置
    if (SUCCEEDED(pCtrl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pVol))) {
      hr = pVol->SetMute(mute ? TRUE : FALSE, nullptr);
      success = SUCCEEDED(hr);
      pVol->Release();
    }
    pCtrl2->Release();
    if (success) break;
  }
  return cleanUpLambda();
}
#else
bool setSystemAppMute(bool mute) { return true; }
#endif