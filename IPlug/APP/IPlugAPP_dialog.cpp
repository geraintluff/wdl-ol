#include "IPlugAPP_host.h"
#include "config.h"
#include "resource.h"

#ifdef OS_WIN
#include "asio.h"
#endif

//// check the input and output devices, find matching srs
//void IPlugAPPHost::PopulateSampleRateList(HWND hwndDlg, RtAudio::DeviceInfo* inputDevInfo, RtAudio::DeviceInfo* outputDevInfo)
//{
//  WDL_String buf;
//
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_SR,CB_RESETCONTENT,0,0);
//
//  std::vector<int> matchedSRs;
//
//  if (inputDevInfo->probed && outputDevInfo->probed)
//  {
//    for (int i=0; i<inputDevInfo->sampleRates.size(); i++)
//    {
//      for (int j=0; j<outputDevInfo->sampleRates.size(); j++)
//      {
//        if(inputDevInfo->sampleRates[i] == outputDevInfo->sampleRates[j])
//          matchedSRs.push_back(inputDevInfo->sampleRates[i]);
//      }
//    }
//  }
//
//  for (int k=0; k<matchedSRs.size(); k++)
//  {
//    buf.SetFormatted(20, "%i", matchedSRs[k]);
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_SR,CB_ADDSTRING,0,(LPARAM)buf.Get());
//  }
//
//  LRESULT sridx = SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_SR, CB_FINDSTRINGEXACT, -1, (LPARAM)mState->mAudioSR);
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_SR,CB_SETCURSEL, sridx, 0);
//}
//
//void IPlugAPPHost::PopulateAudioInputList(HWND hwndDlg, RtAudio::DeviceInfo* info)
//{
//  if(!info->probed)
//    return;
//
//  WDL_String buf;
//
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_L,CB_RESETCONTENT,0,0);
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_R,CB_RESETCONTENT,0,0);
//
//  int i;
//
//  for (i=0; i<info->inputChannels -1; i++)
//  {
//    buf.SetFormatted(20, "%i", i+1);
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_L,CB_ADDSTRING,0,(LPARAM)buf.Get());
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_R,CB_ADDSTRING,0,(LPARAM)buf.Get());
//  }
//
//  // TEMP
//  buf.SetFormatted(20, "%i", i+1);
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_R,CB_ADDSTRING,0,(LPARAM)buf.Get());
//
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_L,CB_SETCURSEL, mState->mAudioInChanL - 1, 0);
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_R,CB_SETCURSEL, mState->mAudioInChanR - 1, 0);
//}
//
//void IPlugAPPHost::PopulateAudioOutputList(HWND hwndDlg, RtAudio::DeviceInfo* info)
//{
//  if(!info->probed)
//    return;
//
//  WDL_String buf;
//
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_L,CB_RESETCONTENT,0,0);
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_R,CB_RESETCONTENT,0,0);
//
//  int i;
//
//  //  for (int i=0; i<info.outputChannels; i++) {
//  for (i=0; i<info->outputChannels -1; i++)
//  {
//    buf.SetFormatted(20, "%i", i+1);
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_L,CB_ADDSTRING,0,(LPARAM)buf.Get());
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_R,CB_ADDSTRING,0,(LPARAM)buf.Get());
//  }
//
//  // TEMP
//  buf.SetFormatted(20, "%i", i+1);
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_R,CB_ADDSTRING,0,(LPARAM)buf.Get());
//
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_L,CB_SETCURSEL, mState->mAudioOutChanL - 1, 0);
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_R,CB_SETCURSEL, mState->mAudioOutChanR - 1, 0);
//}
//
//// This has to get called after any change to audio driver/in dev/out dev
//void IPlugAPPHost::PopulateDriverSpecificControls(HWND hwndDlg)
//{
//#ifdef OS_WIN
//  int driverType = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_DRIVER, CB_GETCURSEL, 0, 0);
//  if(driverType)   //ASIO
//  {
//    ComboBox_Enable(GetDlgItem(hwndDlg, IDC_COMBO_AUDIO_IN_DEV), FALSE);
//    Button_Enable(GetDlgItem(hwndDlg, IDC_BUTTON_ASIO), TRUE);
//  }
//  else
//  {
//    ComboBox_Enable(GetDlgItem(hwndDlg, IDC_COMBO_AUDIO_IN_DEV), TRUE);
//    Button_Enable(GetDlgItem(hwndDlg, IDC_BUTTON_ASIO), FALSE);
//  }
//#endif
//
//  int indevidx = 0;
//  int outdevidx = 0;
//
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_DEV,CB_RESETCONTENT,0,0);
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_DEV,CB_RESETCONTENT,0,0);
//
//  for (int i = 0; i<mAudioInputDevs.size(); i++)
//  {
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_DEV,CB_ADDSTRING,0,(LPARAM)GetAudioDeviceName(mAudioInputDevs[i]).c_str());
//
//    if(!strcmp(GetAudioDeviceName(mAudioInputDevs[i]).c_str(), mState->mAudioInDev))
//      indevidx = i;
//  }
//
//  for (int i = 0; i<mAudioOutputDevs.size(); i++)
//  {
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_DEV,CB_ADDSTRING,0,(LPARAM)GetAudioDeviceName(mAudioOutputDevs[i]).c_str());
//
//    if(!strcmp(GetAudioDeviceName(mAudioOutputDevs[i]).c_str(), mState->mAudioOutDev))
//      outdevidx = i;
//  }
//
//#ifdef OS_WIN
//  if(driverType)
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_DEV,CB_SETCURSEL, outdevidx, 0);
//  else
//#endif
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_DEV,CB_SETCURSEL, indevidx, 0);
//
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_DEV,CB_SETCURSEL, outdevidx, 0);
//
//  RtAudio::DeviceInfo inputDevInfo;
//  RtAudio::DeviceInfo outputDevInfo;
//
//  if (mAudioInputDevs.size())
//  {
//    inputDevInfo = mDAC->getDeviceInfo(mAudioInputDevs[indevidx]);
//    PopulateAudioInputList(hwndDlg, &inputDevInfo);
//  }
//
//  if (mAudioOutputDevs.size())
//  {
//    outputDevInfo = mDAC->getDeviceInfo(mAudioOutputDevs[outdevidx]);
//    PopulateAudioOutputList(hwndDlg, &outputDevInfo);
//  }
//
//  PopulateSampleRateList(hwndDlg, &inputDevInfo, &outputDevInfo);
//}
//
//void IPlugAPPHost::PopulateAudioDialogs(HWND hwndDlg)
//{
//  PopulateDriverSpecificControls(hwndDlg);
//
//  if (mState->mAudioInIsMono)
//  {
//    SendDlgItemMessage(hwndDlg,IDC_CB_MONO_INPUT,BM_SETCHECK, BST_CHECKED,0);
//  }
//  else
//  {
//    SendDlgItemMessage(hwndDlg,IDC_CB_MONO_INPUT,BM_SETCHECK, BST_UNCHECKED,0);
//  }
//
//  //Populate IOVS combobox
//  for (int i = 0; i< kNumIOVSOptions; i++)
//  {
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IOVS,CB_ADDSTRING,0,(LPARAM)kIOVSOptions[i].c_str());
//  }
//
//  LRESULT iovsidx = SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_IOVS, CB_FINDSTRINGEXACT, -1, (LPARAM)mState->mAudioIOVS);
//  SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_IOVS, CB_SETCURSEL, iovsidx, 0);
//
//  //Populate SIGVS combobox
//  for (int i = 0; i< kNumSIGVSOptions; i++)
//  {
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_SIGVS,CB_ADDSTRING,0,(LPARAM)kSIGVSOptions[i].c_str());
//  }
//
//  LRESULT sigvsidx = SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_SIGVS, CB_FINDSTRINGEXACT, -1, (LPARAM)mState->mAudioSigVS);
//  SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_SIGVS, CB_SETCURSEL, sigvsidx, 0);
//}
//
//bool IPlugAPPHost::PopulateMidiDialogs(HWND hwndDlg)
//{
//  if ( !mMidiIn || !mMidiOut )
//    return false;
//  else
//  {
//    for (int i=0; i<mMidiInputDevNames.size(); i++ )
//    {
//      SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN_DEV,CB_ADDSTRING,0,(LPARAM)mMidiInputDevNames[i].c_str());
//    }
//
//    LRESULT indevidx = SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN_DEV,CB_FINDSTRINGEXACT, -1, (LPARAM)mState->mMidiInDev);
//
//    // if the midi port name wasn't found update the ini file, and set to off
//    if(indevidx == -1)
//    {
//      strcpy(mState->mMidiInDev, "off");
//      UpdateINI();
//      indevidx = 0;
//    }
//
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN_DEV,CB_SETCURSEL, indevidx, 0);
//
//    for (int i=0; i<mMidiOutputDevNames.size(); i++ )
//    {
//      SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT_DEV,CB_ADDSTRING,0,(LPARAM)mMidiOutputDevNames[i].c_str());
//    }
//
//    LRESULT outdevidx = SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT_DEV,CB_FINDSTRINGEXACT, -1, (LPARAM)mState->mMidiOutDev);
//
//    // if the midi port name wasn't found update the ini file, and set to off
//    if(outdevidx == -1)
//    {
//      strcpy(mState->mMidiOutDev, "off");
//      UpdateINI();
//      outdevidx = 0;
//    }
//
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT_DEV,CB_SETCURSEL, outdevidx, 0);
//
//    // Populate MIDI channel dialogs
//
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN_CHAN,CB_ADDSTRING,0,(LPARAM)"all");
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT_CHAN,CB_ADDSTRING,0,(LPARAM)"all");
//
//    WDL_String buf;
//
//    for (int i=0; i<16; i++)
//    {
//      buf.SetFormatted(20, "%i", i+1);
//      SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN_CHAN,CB_ADDSTRING,0,(LPARAM)buf.Get());
//      SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT_CHAN,CB_ADDSTRING,0,(LPARAM)buf.Get());
//    }
//
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_IN_CHAN,CB_SETCURSEL, (LPARAM)mState->mMidiInChan, 0);
//    SendDlgItemMessage(hwndDlg,IDC_COMBO_MIDI_OUT_CHAN,CB_SETCURSEL, (LPARAM)mState->mMidiOutChan, 0);
//
//    return true;
//  }
//}
//
//#ifdef OS_WIN
//void IPlugAPPHost::PopulatePreferencesDialog(HWND hwndDlg)
//{
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_DRIVER,CB_ADDSTRING,0,(LPARAM)"DirectSound");
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_DRIVER,CB_ADDSTRING,0,(LPARAM)"ASIO");
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_DRIVER,CB_SETCURSEL, mState->mAudioDriverType, 0);
//
//  PopulateAudioDialogs(hwndDlg);
//  PopulateMidiDialogs(hwndDlg);
//}
//
//#else //OSX
//void IPlugAPPHost::PopulatePreferencesDialog(HWND hwndDlg)
//{
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_DRIVER,CB_ADDSTRING,0,(LPARAM)"CoreAudio");
//  //SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_DRIVER,CB_ADDSTRING,0,(LPARAM)"Jack");
//  SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_DRIVER,CB_SETCURSEL, mState->mAudioDriverType, 0);
//
//  PopulateAudioDialogs(hwndDlg);
//  PopulateMidiDialogs(hwndDlg);
//}
//#endif
//
//WDL_DLGRET IPlugAPPHost::PreferencesDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//  IPlugAPPHost* _this = sInstance;
//  AppState* mState = _this->mState;
//  AppState* mTempState = _this->mTempState;
//  AppState* mActiveState = _this->mActiveState;
//
//  int v = 0;
//  switch(uMsg)
//  {
//    case WM_INITDIALOG:
//
//      _this->PopulatePreferencesDialog(hwndDlg);
//      memcpy(mTempState, mState, sizeof(AppState)); // copy state to temp state
//
//      return TRUE;
//
//    case WM_COMMAND:
//      switch (LOWORD(wParam))
//      {
//        case IDOK:
//          if(memcmp(mActiveState, mState, sizeof(AppState)) != 0) // if state is different try to change audio
//          {
//            _this->TryToChangeAudio();
//          }
//          EndDialog(hwndDlg, IDOK); // INI file will be changed see MainDialogProc
//          break;
//        case IDAPPLY:
//          _this->TryToChangeAudio();
//          break;
//        case IDCANCEL:
//          EndDialog(hwndDlg, IDCANCEL);
//
//          // if state has been changed reset to previous state, INI file won't be changed
//          if (!_this->AudioSettingsInStateAreEqual(mState, mTempState)
//              || !_this->MIDISettingsInStateAreEqual(mState, mTempState))
//          {
//            memcpy(mState, mTempState, sizeof(AppState));
//
//            _this->TryToChangeAudioDriverType();
//            _this->ProbeAudioIO();
//            _this->TryToChangeAudio();
//          }
//
//          break;
//
//        case IDC_COMBO_AUDIO_DRIVER:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//          {
//
//            v = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_DRIVER, CB_GETCURSEL, 0, 0);
//
//            if(v != mState->mAudioDriverType)
//            {
//              mState->mAudioDriverType = v;
//
//              _this->TryToChangeAudioDriverType();
//              _this->ProbeAudioIO();
//
//              if (_this->mAudioInputDevs.size())
//                strcpy(mState->mAudioInDev,_this->GetAudioDeviceName(_this->mAudioInputDevs[0]).c_str());
//
//              if (_this->mAudioOutputDevs.size())
//                strcpy(mState->mAudioOutDev,_this->GetAudioDeviceName(_this->mAudioOutputDevs[0]).c_str());
//
//              // Reset IO
//              mState->mAudioOutChanL = 1;
//              mState->mAudioOutChanR = 2;
//
//              _this->PopulateAudioDialogs(hwndDlg);
//            }
//          }
//          break;
//
//        case IDC_COMBO_AUDIO_IN_DEV:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//          {
//            int idx = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_IN_DEV, CB_GETCURSEL, 0, 0);
//            SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_IN_DEV, CB_GETLBTEXT, idx, (LPARAM) mState->mAudioInDev);
//
//            // Reset IO
//            mState->mAudioInChanL = 1;
//            mState->mAudioInChanR = 2;
//
//            _this->PopulateDriverSpecificControls(hwndDlg);
//          }
//          break;
//
//        case IDC_COMBO_AUDIO_OUT_DEV:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//          {
//            int idx = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_OUT_DEV, CB_GETCURSEL, 0, 0);
//            SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_OUT_DEV, CB_GETLBTEXT, idx, (LPARAM) mState->mAudioOutDev);
//
//            // Reset IO
//            mState->mAudioOutChanL = 1;
//            mState->mAudioOutChanR = 2;
//
//            _this->PopulateDriverSpecificControls(hwndDlg);
//          }
//          break;
//
//        case IDC_COMBO_AUDIO_IN_L:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//          {
//            mState->mAudioInChanL = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_IN_L, CB_GETCURSEL, 0, 0) + 1;
//
//            //TEMP
//            mState->mAudioInChanR = mState->mAudioInChanL + 1;
//            SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_R,CB_SETCURSEL, mState->mAudioInChanR - 1, 0);
//            //
//          }
//          break;
//
//        case IDC_COMBO_AUDIO_IN_R:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//            SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_IN_R,CB_SETCURSEL, mState->mAudioInChanR - 1, 0);  // TEMP
//                mState->mAudioInChanR = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_IN_R, CB_GETCURSEL, 0, 0);
//          break;
//
//        case IDC_COMBO_AUDIO_OUT_L:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//          {
//            mState->mAudioOutChanL = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_OUT_L, CB_GETCURSEL, 0, 0) + 1;
//
//            //TEMP
//            mState->mAudioOutChanR = mState->mAudioOutChanL + 1;
//            SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_R,CB_SETCURSEL, mState->mAudioOutChanR - 1, 0);
//            //
//          }
//          break;
//
//        case IDC_COMBO_AUDIO_OUT_R:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//            SendDlgItemMessage(hwndDlg,IDC_COMBO_AUDIO_OUT_R,CB_SETCURSEL, mState->mAudioOutChanR - 1, 0);  // TEMP
//                mState->mAudioOutChanR = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_OUT_R, CB_GETCURSEL, 0, 0);
//          break;
//
//        case IDC_CB_MONO_INPUT:
//          if (SendDlgItemMessage(hwndDlg,IDC_CB_MONO_INPUT, BM_GETCHECK, 0, 0) == BST_CHECKED)
//            mState->mAudioInIsMono = 1;
//          else
//            mState->mAudioInIsMono = 0;
//          break;
//
//        case IDC_COMBO_AUDIO_IOVS: // follow through
//        case IDC_COMBO_AUDIO_SIGVS:
//          //TODO: FIX
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//          {
//            int iovsidx = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_IOVS, CB_GETCURSEL, 0, 0);
//            int sigvsidx = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_SIGVS, CB_GETCURSEL, 0, 0);
//
//            if (atoi(kIOVSOptions[iovsidx].c_str()) < atoi(kSIGVSOptions[sigvsidx].c_str()))   // if iovs < sigvs
//            {
//              SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_IOVS, CB_GETLBTEXT, iovsidx, (LPARAM) mState->mAudioIOVS);
//              strcpy(mState->mAudioSigVS, kSIGVSOptions[0].c_str()); // set sigvs to minimum
//              SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_SIGVS, CB_SETCURSEL, -1, 0);
//            }
//            else
//            {
//              SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_IOVS, CB_GETLBTEXT, iovsidx, (LPARAM) mState->mAudioIOVS);
//              SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_SIGVS, CB_GETLBTEXT, sigvsidx, (LPARAM) mState->mAudioSigVS);
//            }
//          }
//          break;
//        case IDC_COMBO_AUDIO_SR:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//          {
//            int idx = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_SR, CB_GETCURSEL, 0, 0);
//            SendDlgItemMessage(hwndDlg, IDC_COMBO_AUDIO_SR, CB_GETLBTEXT, idx, (LPARAM) mState->mAudioSR);
//          }
//          break;
//
//        case IDC_BUTTON_ASIO:
//          if (HIWORD(wParam) == BN_CLICKED)
//            #ifdef OS_WIN
//            if( (_this->mState->mAudioDriverType == DAC_ASIO) && (_this->mDAC->isStreamRunning() == true)) // TODO: still not right
//              ASIOControlPanel();
//            #else // OSX
//            system("open \"/Applications/Utilities/Audio MIDI Setup.app\"");
//            #endif
//          break;
//
//        case IDC_COMBO_MIDI_IN_DEV:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//          {
//            int idx = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_IN_DEV, CB_GETCURSEL, 0, 0);
//            SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_IN_DEV, CB_GETLBTEXT, idx, (LPARAM) mState->mMidiInDev);
//            _this->ChooseMidiInput( mState->mMidiInDev );
//          }
//          break;
//
//        case IDC_COMBO_MIDI_OUT_DEV:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//          {
//            int idx = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DEV, CB_GETCURSEL, 0, 0);
//            SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_DEV, CB_GETLBTEXT, idx, (LPARAM) mState->mMidiOutDev);
//            _this->ChooseMidiOutput( mState->mMidiOutDev );
//          }
//          break;
//
//        case IDC_COMBO_MIDI_IN_CHAN:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//            mState->mMidiInChan = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_IN_CHAN, CB_GETCURSEL, 0, 0);
//          break;
//
//        case IDC_COMBO_MIDI_OUT_CHAN:
//          if (HIWORD(wParam) == CBN_SELCHANGE)
//            mState->mMidiOutChan = (int) SendDlgItemMessage(hwndDlg, IDC_COMBO_MIDI_OUT_CHAN, CB_GETCURSEL, 0, 0);
//          break;
//
//        default:
//          break;
//      }
//      break;
//    default:
//      return FALSE;
//  }
//  return TRUE;
//}

//static
WDL_DLGRET MainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{  
  IPlugAPPHost* app = IPlugAPPHost::sInstance;

  switch (uMsg)
  {
    case WM_INITDIALOG:
      gHWND = hwndDlg;
//
//      if(!app->AttachGUI())
//        printf("couldn't attach gui\n"); //todo error
      
//      ClientResize(hwndDlg, PLUG_WIDTH, PLUG_HEIGHT);

      ShowWindow(hwndDlg,SW_SHOW);
      return 1;
    case WM_DESTROY:
      gHWND = NULL;

      #ifdef OS_WIN
      PostQuitMessage(0);
      #else
      SWELL_PostQuitMessage(hwndDlg);
      #endif

      return 0;
    case WM_CLOSE:
      DestroyWindow(hwndDlg);
      return 0;
//    case WM_GETDLGCODE: {
//        LPARAM lres;
//        lres = CallWindowProc(/*TODO GET PROC */, hWnd, WM_GETDLGCODE, wParam, lParam);
//        if (lParam && ((MSG*)lParam)->message == WM_KEYDOWN  &&  wParam == VK_LEFT) {
//          lres |= DLGC_WANTMESSAGE;
//        }
//        return lres;
//      }
    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case ID_QUIT:
          DestroyWindow(hwndDlg);
          return 0;
        case ID_ABOUT:
        {
          bool pluginOpensAboutBox = false;

          //TODO: open about box
          
          if (pluginOpensAboutBox == false) 
          {
            WDL_String info;
            info.Append(PLUG_COPYRIGHT_STR"\nBuilt on " __DATE__);
            MessageBox(hwndDlg, info.Get(), PLUG_NAME, MB_OK);
          }
          
          return 0;
        }
//        case ID_HELP:
//        {
//          WDL_String info;
//          info.Set("open help");
//          MessageBox(hwndDlg, info.Get(), PLUG_NAME, MB_OK);
//          
//          return 0;
//        }
//        case ID_PREFERENCES:
//        {
//          INT_PTR ret = DialogBox(gHINST, MAKEINTRESOURCE(IDD_DIALOG_PREF), hwndDlg, PreferencesDlgProc);
//
//          if(ret == IDOK)
//          {
//            sInst->UpdateINI();
//          }
//
//          return 0;
//        }
      }
      return 0;
  }
  return 0;
}
