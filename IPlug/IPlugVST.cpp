#include <cstdio>
#include "IPlugVST.h"

const int VST_VERSION = 2400;

int VSTSpkrArrType(int nchan)
{
  if (!nchan) return kSpeakerArrEmpty;
  if (nchan == 1) return kSpeakerArrMono;
  if (nchan == 2) return kSpeakerArrStereo;
  return kSpeakerArrUserDefined;
}

IPlugVST::IPlugVST(IPlugInstanceInfo instanceInfo, IPlugConfig c)
  : IPLUG_BASE_CLASS(c, kAPIVST2)
  , IPlugProcessor<PLUG_SAMPLE_DST>(c, kAPIVST2)
  , IPlugPresetHandler(c, kAPIVST2)
  , mHostCallback(instanceInfo.mVSTHostCallback)
{
  Trace(TRACELOC, "%s", c.pluginName);

  AttachPresetHandler(this);

  mHasVSTExtensions = VSTEXT_NONE;

  int nInputs = MaxNChannels(ERoute::kInput), nOutputs = MaxNChannels(ERoute::kOutput);

  memset(&mAEffect, 0, sizeof(AEffect));
  mAEffect.object = this;
  mAEffect.magic = kEffectMagic;
  mAEffect.dispatcher = VSTDispatcher;
  mAEffect.getParameter = VSTGetParameter;
  mAEffect.setParameter = VSTSetParameter;
  mAEffect.numPrograms = c.nPresets;
  mAEffect.numParams = c.nParams;
  mAEffect.numInputs = nInputs;
  mAEffect.numOutputs = nOutputs;
  mAEffect.uniqueID = c.uniqueID;
  mAEffect.version = GetPluginVersion(true);
  mAEffect.__ioRatioDeprecated = 1.0f;
  mAEffect.__processDeprecated = VSTProcess;
  mAEffect.processReplacing = VSTProcessReplacing;
  mAEffect.processDoubleReplacing = VSTProcessDoubleReplacing;
  mAEffect.initialDelay = c.latency;
  mAEffect.flags = effFlagsCanReplacing | effFlagsCanDoubleReplacing;

  if (c.plugDoesChunks) { mAEffect.flags |= effFlagsProgramChunks; }
  if (LegalIO(1, -1)) { mAEffect.flags |= __effFlagsCanMonoDeprecated; }
  if (c.plugIsInstrument) { mAEffect.flags |= effFlagsIsSynth; }

  memset(&mEditRect, 0, sizeof(ERect));
  memset(&mInputSpkrArr, 0, sizeof(VstSpeakerArrangement));
  memset(&mOutputSpkrArr, 0, sizeof(VstSpeakerArrangement));
  mInputSpkrArr.numChannels = nInputs;
  mOutputSpkrArr.numChannels = nOutputs;
  mInputSpkrArr.type = VSTSpkrArrType(nInputs);
  mOutputSpkrArr.type = VSTSpkrArrType(nOutputs);

  // Default everything to connected, then disconnect pins if the host says to.
  _SetChannelConnections(ERoute::kInput, 0, nInputs, true);
  _SetChannelConnections(ERoute::kOutput, 0, nOutputs, true);

  _SetBlockSize(DEFAULT_BLOCK_SIZE);

  if(c.plugHasUI)
  {
    mAEffect.flags |= effFlagsHasEditor;
    mEditRect.left = mEditRect.top = 0;
    mEditRect.right = c.plugWidth;
    mEditRect.bottom = c.plugHeight;
  }
}

void IPlugVST::BeginInformHostOfParamChange(int idx)
{
  mHostCallback(&mAEffect, audioMasterBeginEdit, idx, 0, 0, 0.0f);
}

void IPlugVST::InformHostOfParamChange(int idx, double normalizedValue)
{
  mHostCallback(&mAEffect, audioMasterAutomate, idx, 0, 0, (float) normalizedValue);
}

void IPlugVST::EndInformHostOfParamChange(int idx)
{
  mHostCallback(&mAEffect, audioMasterEndEdit, idx, 0, 0, 0.0f);
}

void IPlugVST::InformHostOfProgramChange()
{
  mHostCallback(&mAEffect, audioMasterUpdateDisplay, 0, 0, 0, 0.0f);
}

EHost IPlugVST::GetHost()
{
  EHost host = IPLUG_BASE_CLASS::GetHost();

  if (host == kHostUninit)
  {
    char productStr[256];
    productStr[0] = '\0';
    int version = 0;
    mHostCallback(&mAEffect, audioMasterGetProductString, 0, 0, productStr, 0.0f);

    if (CSTR_NOT_EMPTY(productStr))
    {
      int decVer = (int) mHostCallback(&mAEffect, audioMasterGetVendorVersion, 0, 0, 0, 0.0f);
      int ver = decVer / 10000;
      int rmaj = (decVer - 10000 * ver) / 100;
      int rmin = (decVer - 10000 * ver - 100 * rmaj);
      version = (ver << 16) + (rmaj << 8) + rmin;
    }

    SetHost(productStr, version);
    host = IPLUG_BASE_CLASS::GetHost();
  }

  return host;
}

void IPlugVST::ResizeGraphics()
{
  if (HasUI())
  {
    mEditRect.left = mEditRect.top = 0;
    mEditRect.right = Width();
    mEditRect.bottom = Height();

    OnWindowResize();
  }
}

void IPlugVST::SetLatency(int samples)
{
  mAEffect.initialDelay = samples;
  IPlugProcessor::SetLatency(samples);
}

bool IPlugVST::SendVSTEvent(VstEvent& event)
{
  // It would be more efficient to bundle these and send at the end of a processed block,
  // but that would require writing OnBlockEnd and making sure it always gets called,
  // and who cares anyway, midi events aren't that dense.
  VstEvents events;
  memset(&events, 0, sizeof(VstEvents));
  events.numEvents = 1;
  events.events[0] = &event;
  return (mHostCallback(&mAEffect, audioMasterProcessEvents, 0, 0, &events, 0.0f) == 1);
}

bool IPlugVST::SendMidiMsg(const IMidiMsg& msg)
{
  VstMidiEvent midiEvent;
  memset(&midiEvent, 0, sizeof(VstMidiEvent));

  midiEvent.type = kVstMidiType;
  midiEvent.byteSize = sizeof(VstMidiEvent);  // Should this be smaller?
  midiEvent.deltaFrames = msg.mOffset;
  midiEvent.midiData[0] = msg.mStatus;
  midiEvent.midiData[1] = msg.mData1;
  midiEvent.midiData[2] = msg.mData2;

  return SendVSTEvent((VstEvent&) midiEvent);
}

bool IPlugVST::SendSysEx(ISysEx& msg)
{
  VstMidiSysexEvent sysexEvent;
  memset(&sysexEvent, 0, sizeof(VstMidiSysexEvent));

  sysexEvent.type = kVstSysExType;
  sysexEvent.byteSize = sizeof(VstMidiSysexEvent);
  sysexEvent.deltaFrames = msg.mOffset;
  sysexEvent.dumpBytes = msg.mSize;
  sysexEvent.sysexDump = (char*) msg.mData;

  return SendVSTEvent((VstEvent&) sysexEvent);
}

void IPlugVST::HostSpecificInit()
{
  if (!mHostSpecificInitDone)
  {
    mHostSpecificInitDone = true;
    EHost host = GetHost();
    switch (host)
    {
      case kHostAudition:
      case kHostOrion:
      case kHostForte:
      case kHostSAWStudio:
        LimitToStereoIO(); //TODO:  is this still necessary?
        break;
      default:
        break;
    }

    // This won't always solve a picky host problem -- for example Forte
    // looks at mAEffect IO count before identifying itself.
    mAEffect.numInputs = mInputSpkrArr.numChannels = MaxNChannels(ERoute::kInput);
    mAEffect.numOutputs = mOutputSpkrArr.numChannels = MaxNChannels(ERoute::kOutput);

    OnHostIdentified();
  }
}

VstIntPtr VSTCALLBACK IPlugVST::VSTDispatcher(AEffect *pEffect, VstInt32 opCode, VstInt32 idx, VstIntPtr value, void *ptr, float opt)
{
  // VSTDispatcher is an IPlugVST class member, we can access anything in IPlugVST from here.
  IPlugVST* _this = (IPlugVST*) pEffect->object;
  if (!_this)
  {
    return 0;
  }

  // Handle a couple of opcodes here to make debugging easier.
  switch (opCode)
  {
    case effEditIdle:
    case __effIdleDeprecated:
    #ifdef USE_IDLE_CALLS
    _this->OnIdle();
    #endif
    return 0;
  }

  Trace(TRACELOC, "%d(%s):%d:%d", opCode, VSTOpcodeStr(opCode), idx, (int) value);

  switch (opCode)
  {
    case effOpen:
    {
      _this->HostSpecificInit();
      _this->OnParamReset(kReset);
      return 0;
    }
    case effClose:
    {
      DELETE_NULL(_this);
      return 0;
    }
    case effGetParamLabel:
    {
      if (idx >= 0 && idx < _this->NParams())
      {
        ENTER_PARAMS_MUTEX_STATIC;
        strcpy((char*) ptr, _this->GetParam(idx)->GetLabelForHost());
        LEAVE_PARAMS_MUTEX_STATIC;
      }
      return 0;
    }
    case effGetParamDisplay:
    {
      if (idx >= 0 && idx < _this->NParams())
      {
        ENTER_PARAMS_MUTEX_STATIC;
        _this->GetParam(idx)->GetDisplayForHost(_this->mParamDisplayStr);
        LEAVE_PARAMS_MUTEX_STATIC;
        strcpy((char*) ptr, _this->mParamDisplayStr.Get());
      }
      return 0;
    }
    case effGetParamName:
    {
      if (idx >= 0 && idx < _this->NParams())
      {
        ENTER_PARAMS_MUTEX_STATIC;
        strcpy((char*) ptr, _this->GetParam(idx)->GetNameForHost());
        LEAVE_PARAMS_MUTEX_STATIC;
      }
      return 0;
    }
    case effGetParameterProperties:
    {
      if (idx >= 0 && idx < _this->NParams())
      {
        VstParameterProperties* props = (VstParameterProperties*) ptr;

        ENTER_PARAMS_MUTEX_STATIC;
        IParam* pParam = _this->GetParam(idx);
        switch (pParam->Type())
        {
          case IParam::kTypeInt:
          case IParam::kTypeEnum:
            props->flags = kVstParameterUsesIntStep | kVstParameterUsesIntegerMinMax;
            props->minInteger = (int) pParam->GetMin();
            props->maxInteger = (int) pParam->GetMax();
            props->stepInteger = props->largeStepInteger = 1;
            break;
          case IParam::kTypeBool:
            props->flags = kVstParameterIsSwitch;
            break;
          case IParam::kTypeDouble:
          default:
            props->flags = kVstParameterUsesFloatStep;
            props->largeStepFloat = props->smallStepFloat = props->stepFloat = pParam->GetStep();
            break;
        }

        strcpy(props->label, pParam->GetLabelForHost());
        LEAVE_PARAMS_MUTEX_STATIC;

        return 1;
      }
      return 0;
    }
    case effString2Parameter:
    {
      if (idx >= 0 && idx < _this->NParams())
      {
        if (ptr)
        {
          ENTER_PARAMS_MUTEX_STATIC;
          IParam* pParam = _this->GetParam(idx);
          const double v = pParam->StringToValue((const char *)ptr);
          pParam->Set(v);
          _this->SendParameterValueToUIFromAPI(idx, v, false);
          _this->OnParamChange(idx, kAutomation);
          LEAVE_PARAMS_MUTEX_STATIC;
        }
        return 1;
      }
      return 0;
    }
    case effSetSampleRate:
    {
      _this->_SetSampleRate(opt);
      _this->OnReset();
      return 0;
    }
    case effSetBlockSize:
    {
      _this->_SetBlockSize((int) value);
      _this->OnReset();
      return 0;
    }
    case effMainsChanged:
    {
      if (!value)
      {
        _this->OnActivate(false);
        _this->OnReset();
      }
      else
      {
        _this->OnActivate(true);
      }
      return 0;
    }
    case effEditGetRect:
    {
      if (ptr && _this->HasUI())
      {
        *(ERect**) ptr = &(_this->mEditRect);
        return 1;
      }
      ptr = 0;
      return 0;
    }
    case effEditOpen:
    {
#if defined(_WIN32) || defined(__LP64__) // __LP64__ = cocoa macOS
      if (_this->OpenWindow(ptr)) return 1;
#else   // OSX 32 bit, check if we are in a Cocoa VST host, otherwise tough luck
      bool iscocoa = (_this->mHasVSTExtensions&VSTEXT_COCOA);
      if (iscocoa && _this->OpenWindow(ptr)) return 1; // cocoa supported open cocoa
#endif
      return 0;
    }
    case effEditClose:
    {
      if (_this->HasUI())
      {
        _this->CloseWindow();
        return 1;
      }
      return 0;
    }
    case __effIdentifyDeprecated:
    {
      return 'NvEf';  // Random deprecated magic.
    }
    case effGetChunk:
    {
      uint8_t** ppData = (uint8_t**) ptr;
      if (ppData)
      {
        bool isBank = (!idx);
        IByteChunk& chunk = (isBank ? _this->mBankState : _this->mState);
        _this->InitChunkWithIPlugVer(chunk);
        bool savedOK = true;

        if (isBank)
        {
          _this->ModifyCurrentPreset();
          savedOK = _this->SerializePresets(chunk);
        }
        else
        {
          savedOK = _this->SerializeState(chunk);
        }

        if (savedOK && chunk.Size())
        {
          *ppData = chunk.GetBytes();
          return chunk.Size();
        }
      }
      return 0;
    }
    case effSetChunk:
    {
      if (ptr)
      {
        bool isBank = (!idx);
        IByteChunk& chunk = (isBank ? _this->mBankState : _this->mState);
        chunk.Resize((int) value);
        memcpy(chunk.GetBytes(), ptr, value);
        int pos = 0;
        int iplugVer = _this->GetIPlugVerFromChunk(chunk, pos);
        isBank &= (iplugVer >= 0x010000);

        if (isBank)
        {
          pos = _this->UnserializePresets(chunk, pos);
        }
        else
        {
          pos = _this->UnserializeState(chunk, pos);
          _this->ModifyCurrentPreset();
        }

        if (pos >= 0)
        {
          _this->OnRestoreState();
          return 1;
        }
      }
      return 0;
    }
    case effProcessEvents:
    {
      VstEvents* pEvents = (VstEvents*) ptr;
      if (pEvents)
      {
        for (int i = 0; i < pEvents->numEvents; ++i)
        {
          VstEvent* pEvent = pEvents->events[i];
          if (pEvent)
          {
            if (pEvent->type == kVstMidiType)
            {
              VstMidiEvent* pME = (VstMidiEvent*) pEvent;
              IMidiMsg msg(pME->deltaFrames, pME->midiData[0], pME->midiData[1], pME->midiData[2]);
              _this->ProcessMidiMsg(msg);
              //#ifdef TRACER_BUILD
              //  msg.LogMsg();
              //#endif
            }
            else if (pEvent->type == kVstSysExType)
            {
              VstMidiSysexEvent* pSE = (VstMidiSysexEvent*) pEvent;
              ISysEx sysex(pSE->deltaFrames, (const uint8_t*)pSE->sysexDump, pSE->dumpBytes);
              _this->ProcessSysEx(sysex);
            }
          }
        }
        return 1;
      }
      return 0;
    }
    case effCanBeAutomated:
    {
      if (idx >= 0 && idx < _this->NParams())
      {
        return _this->GetParam(idx)->GetCanAutomate();
      }
    }
    case effGetInputProperties:
    {
      if (ptr && idx >= 0 && idx < _this->MaxNChannels(ERoute::kInput))
      {
        VstPinProperties* pp = (VstPinProperties*) ptr;
        pp->flags = kVstPinIsActive;

        if (!(idx%2) && idx < _this->MaxNChannels(ERoute::kInput)-1)
          pp->flags |= kVstPinIsStereo;

        if (_this->_GetChannelLabel(ERoute::kInput, idx).GetLength())
          sprintf(pp->label, "%s", _this->_GetChannelLabel(ERoute::kInput, idx).Get());
        else
          sprintf(pp->label, "Input %d", idx + 1);

        return 1;
      }
      return 0;
    }
    case effGetOutputProperties:
    {
      if (ptr && idx >= 0 && idx < _this->MaxNChannels(ERoute::kOutput))
      {
        VstPinProperties* pp = (VstPinProperties*) ptr;
        pp->flags = kVstPinIsActive;

        if (!(idx%2) && idx < _this->MaxNChannels(ERoute::kOutput)-1)
          pp->flags |= kVstPinIsStereo;

        if (_this->_GetChannelLabel(ERoute::kOutput, idx).GetLength())
          sprintf(pp->label, "%s", _this->_GetChannelLabel(ERoute::kOutput, idx).Get());
        else
          sprintf(pp->label, "Output %d", idx + 1);

        return 1;
      }
      return 0;
    }
    case effGetPlugCategory:
    {
      if (_this->IsInstrument()) return kPlugCategSynth;
      return kPlugCategEffect;
    }
    case effProcessVarIo:
    {
      // VstVariableIo* pIO = (VstVariableIo*) ptr; // For offline processing (of audio files?)
      return 0;
    }
    case effSetSpeakerArrangement:
    {
      VstSpeakerArrangement* pInputArr = (VstSpeakerArrangement*) value;
      VstSpeakerArrangement* pOutputArr = (VstSpeakerArrangement*) ptr;
      if (pInputArr)
      {
        int n = pInputArr->numChannels;
        _this->_SetChannelConnections(ERoute::kInput, 0, n, true);
        _this->_SetChannelConnections(ERoute::kInput, n, _this->MaxNChannels(ERoute::kInput) - n, false);
      }
      if (pOutputArr)
      {
        int n = pOutputArr->numChannels;
        _this->_SetChannelConnections(ERoute::kOutput, 0, n, true);
        _this->_SetChannelConnections(ERoute::kOutput, n, _this->MaxNChannels(ERoute::kOutput) - n, false);
      }
      return 1;
    }
    case effGetSpeakerArrangement:
    {
      VstSpeakerArrangement** ppInputArr = (VstSpeakerArrangement**) value;
      VstSpeakerArrangement** ppOutputArr = (VstSpeakerArrangement**) ptr;
      if (ppInputArr)
      {
        *ppInputArr = &(_this->mInputSpkrArr);
      }
      if (ppOutputArr)
      {
        *ppOutputArr = &(_this->mOutputSpkrArr);
      }
      return 1;
    }
    case effGetEffectName:
    {
      if (ptr)
      {
        strcpy((char*) ptr, _this->GetPluginName());
        return 1;
      }
      return 0;
    }
    case effGetProductString:
    {
      if (ptr)
      {
        strcpy((char*) ptr, _this->GetProductName());
        return 1;
      }
      return 0;
    }
    case effGetVendorString:
    {
      if (ptr)
      {
        strcpy((char*) ptr, _this->GetMfrName());
        return 1;
      }
      return 0;
    }
    case effGetVendorVersion:
    {
      return _this->GetPluginVersion(true);
    }
    case effCanDo:
    {
      if (ptr)
      {
        Trace(TRACELOC, "VSTCanDo(%s)", (char*) ptr);
        if (!strcmp((char*) ptr, "receiveVstTimeInfo"))
        {
          return 1;
        }
        if (_this->DoesMIDI())
        {
          if (!strcmp((char*) ptr, "sendVstEvents") ||
              !strcmp((char*) ptr, "sendVstMidiEvent") ||
              !strcmp((char*) ptr, "receiveVstEvents") ||
              !strcmp((char*) ptr, "receiveVstMidiEvent"))   // ||
          {
            return 1;
          }
        }
        // Support Reaper VST extensions: http://www.reaper.fm/sdk/vst/
        if (!strcmp((char*) ptr, "hasCockosExtensions"))
        {
          _this->mHasVSTExtensions |= VSTEXT_COCKOS;
          return 0xbeef0000;
        }
        else if (!strcmp((char*) ptr, "hasCockosViewAsConfig"))
        {
          _this->mHasVSTExtensions |= VSTEXT_COCOA;
          return 0xbeef0000;
        }
      }
      return 0;
    }
    case effGetTailSize:
    {
      return _this->GetTailSize();
    }
    case effVendorSpecific:
    {
      switch (idx)
      {
          // Mouse wheel
//        case 0x73744341:
//        {
//          if (value == 0x57686565)
//          {
//            IGraphics* pGraphics = _this->GetUI();
//            if (pGraphics) {
//              return pGraphics->ProcessMouseWheel(opt);
//            }
//          }
//          break;
//        }
          // Support Reaper VST extensions: http://www.reaper.fm/sdk/vst/
        case effGetParamDisplay:
        {
          if (ptr)
          {
            if (value >= 0 && value < _this->NParams())
            {
              _this->GetParam((int) value)->GetDisplayForHost((double) opt, true, _this->mParamDisplayStr);
              strcpy((char*) ptr, _this->mParamDisplayStr.Get());
            }
            return 0xbeef;
          }
          break;
        }
        case effString2Parameter:
        {
          if (ptr && value >= 0 && value < _this->NParams())
          {
            if (*(char*) ptr != '\0')
            {
              IParam* pParam = _this->GetParam((int) value);
              sprintf((char*) ptr, "%.17f", pParam->GetNormalized(pParam->StringToValue((const char*) ptr)));
            }
            return 0xbeef;
          }
          break;
        }
        case kVstParameterUsesIntStep:
        {
          if (value >= 0 && value < _this->NParams())
          {
            IParam* pParam = _this->GetParam((int) value);
            switch (pParam->Type())
            {
              case IParam::kTypeBool:
                return 0xbeef;
              case IParam::kTypeInt:
              case IParam::kTypeEnum:
              {
                double min, max;
                pParam->GetBounds(min, max);
                if (std::fabs(max - min) < 1.5)
                  return 0xbeef;

                break;
              }
              default:
                break;
            }
          }
          break;
        }
      }
      return 0;
    }
    case effGetProgram:
    {
      return _this->GetCurrentPresetIdx();
    }
    case effSetProgram:
    {
      if (_this->DoesStateChunks() == false)
      {
        _this->ModifyCurrentPreset(); // TODO: test, something is funny about this http://forum.cockos.com/showpost.php?p=485113&postcount=22
      }
      _this->RestorePreset((int) value);
      return 0;
    }
    case effGetProgramNameIndexed:
    {
      strcpy((char*) ptr, _this->GetPresetName(idx));
      return (CSTR_NOT_EMPTY((char*) ptr) ? 1 : 0);
    }
    case effSetProgramName:
    {
      if (ptr)
      {
        _this->ModifyCurrentPreset((char*) ptr);
        _this->PresetsChangedByHost();
      }
      return 0;
    }
    case effGetProgramName:
    {
      if (ptr)
      {
        int idx = _this->GetCurrentPresetIdx();
        strcpy((char*) ptr, _this->GetPresetName(idx));
      }
      return 0;
    }
    case effGetMidiKeyName:
    {
      if (ptr)
      {
        MidiKeyName* pMKN = (MidiKeyName*) ptr;
        pMKN->keyName[0] = '\0';
        if (_this->GetMidiNoteText(pMKN->thisKeyNumber, pMKN->keyName))
        {
          return 1;
        }
      }
      return 0;
    }
    case effGetVstVersion:
    {
      return VST_VERSION;
    }
    case effEndSetProgram:
    case effBeginSetProgram:
    case effGetMidiProgramName:
    case effHasMidiProgramsChanged:
    case effGetMidiProgramCategory:
    case effGetCurrentMidiProgram:
    case effSetBypass:
    default:
    {
      return 0;
    }
  }
}

template <class SAMPLETYPE>
void IPlugVST::VSTPreProcess(SAMPLETYPE** inputs, SAMPLETYPE** outputs, VstInt32 nFrames)
{
  if (DoesMIDI())
    mHostCallback(&mAEffect, __audioMasterWantMidiDeprecated, 0, 0, 0, 0.0f);

  _AttachBuffers(ERoute::kInput, 0, MaxNChannels(ERoute::kInput), inputs, nFrames);
  _AttachBuffers(ERoute::kOutput, 0, MaxNChannels(ERoute::kOutput), outputs, nFrames);

  VstTimeInfo* pTI = (VstTimeInfo*) mHostCallback(&mAEffect, audioMasterGetTime, 0, kVstPpqPosValid | kVstTempoValid | kVstBarsValid | kVstCyclePosValid | kVstTimeSigValid, 0, 0);

  ITimeInfo timeInfo;

  if (pTI)
  {
    timeInfo.mSamplePos = pTI->samplePos;

    if ((pTI->flags & kVstPpqPosValid) && pTI->ppqPos >= 0.0) timeInfo.mPPQPos = pTI->ppqPos;
    if ((pTI->flags & kVstTempoValid) && pTI->tempo > 0.0) timeInfo.mTempo = pTI->tempo;
    if ((pTI->flags & kVstBarsValid) && pTI->barStartPos >= 0.0) timeInfo.mLastBar = pTI->barStartPos;
    if ((pTI->flags & kVstCyclePosValid) && pTI->cycleStartPos >= 0.0 && pTI->cycleEndPos >= 0.0)
    {
      timeInfo.mCycleStart = pTI->cycleStartPos;
      timeInfo.mCycleEnd = pTI->cycleEndPos;
    }
    if ((pTI->flags & kVstTimeSigValid) && pTI->timeSigNumerator > 0.0 && pTI->timeSigDenominator > 0.0)
    {
      timeInfo.mNumerator = pTI->timeSigNumerator;
      timeInfo.mDenominator = pTI->timeSigDenominator;
    }
    timeInfo.mTransportIsRunning = pTI->flags & kVstTransportPlaying;
    timeInfo.mTransportLoopEnabled = pTI->flags & kVstTransportCycleActive;
  }

  const bool renderingOffline = mHostCallback(&mAEffect, audioMasterGetCurrentProcessLevel, 0, 0, 0, 0.0f) == kVstProcessLevelOffline;

  _SetTimeInfo(timeInfo);
  _SetRenderingOffline(renderingOffline);
}

// Deprecated.
void VSTCALLBACK IPlugVST::VSTProcess(AEffect* pEffect, float** inputs, float** outputs, VstInt32 nFrames)
{
  TRACE;
  IPlugVST* _this = (IPlugVST*) pEffect->object;
  _this->VSTPreProcess(inputs, outputs, nFrames);
  _this->_ProcessBuffersAccumulating(nFrames);
}

void VSTCALLBACK IPlugVST::VSTProcessReplacing(AEffect* pEffect, float** inputs, float** outputs, VstInt32 nFrames)
{
  TRACE;
  IPlugVST* _this = (IPlugVST*) pEffect->object;
  _this->VSTPreProcess(inputs, outputs, nFrames);
  _this->_ProcessBuffers((float) 0.0f, nFrames);
}

void VSTCALLBACK IPlugVST::VSTProcessDoubleReplacing(AEffect* pEffect, double** inputs, double** outputs, VstInt32 nFrames)
{
  TRACE;
  IPlugVST* _this = (IPlugVST*) pEffect->object;
  _this->VSTPreProcess(inputs, outputs, nFrames);
  _this->_ProcessBuffers((double) 0.0, nFrames);
}

float VSTCALLBACK IPlugVST::VSTGetParameter(AEffect *pEffect, VstInt32 idx)
{
  Trace(TRACELOC, "%d", idx);
  IPlugVST* _this = (IPlugVST*) pEffect->object;
  if (idx >= 0 && idx < _this->NParams())
  {
    ENTER_PARAMS_MUTEX_STATIC;
    const float val = (float) _this->GetParam(idx)->GetNormalized();
    LEAVE_PARAMS_MUTEX_STATIC;

    return val;
  }
  return 0.0f;
}

void VSTCALLBACK IPlugVST::VSTSetParameter(AEffect *pEffect, VstInt32 idx, float value)
{
  Trace(TRACELOC, "%d:%f", idx, value);
  IPlugVST* _this = (IPlugVST*) pEffect->object;
  if (idx >= 0 && idx < _this->NParams())
  {
    ENTER_PARAMS_MUTEX_STATIC;
    _this->GetParam(idx)->SetNormalized(value);
    _this->SendParameterValueToUIFromAPI(idx, value, true);
    _this->OnParamChange(idx, kAutomation);
    LEAVE_PARAMS_MUTEX_STATIC;
  }
}
