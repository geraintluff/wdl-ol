#include "IPlugAAX.h"

#include "AAX_CBinaryTaperDelegate.h"
#include "AAX_CBinaryDisplayDelegate.h"
#include "AAX_CStringDisplayDelegate.h"
#include "AAX_CLinearTaperDelegate.h"

// custom taper for IParam::kTypeDouble
#include "AAX/AAX_CIPlugTaperDelegate.h"
#include "AAX_CNumberDisplayDelegate.h"

AAX_CEffectParameters *AAX_CALLBACK IPlugAAX::Create()
{
  return MakePlug();
}

void AAX_CEffectGUI_IPLUG::CreateViewContents() 
{
  IGraphics* gui = ((IPlugAAX*) GetEffectParameters())->GetGUI();
  
  if (gui) 
  {
    mGraphics = gui;
  }
  else
  {
    mGraphics = 0;
  }
}

void AAX_CEffectGUI_IPLUG::CreateViewContainer()
{
  void* winPtr = GetViewContainerPtr();
  
  if (winPtr && mGraphics)
  {
    mGraphics->SetViewContainer(GetViewContainer());
    mGraphics->OpenWindow(winPtr);
  }
}

void AAX_CEffectGUI_IPLUG::DeleteViewContainer() 
{
  if(mGraphics)
  {
    mGraphics->CloseWindow();
  }
}

AAX_Result AAX_CEffectGUI_IPLUG::GetViewSize( AAX_Point *oEffectViewSize ) const
{
  if (mGraphics)
  {
    oEffectViewSize->horz = (float) mGraphics->Width();
    oEffectViewSize->vert = (float) mGraphics->Height();
  }
  
  return AAX_SUCCESS; 
}

AAX_Result AAX_CEffectGUI_IPLUG::ParameterUpdated (const char* iParameterID)
{
  AAX_Result err = AAX_ERROR_INVALID_PARAMETER_ID;
  
  int paramIdx = atoi(iParameterID) - kPTParamIdxOffset;
  
  if ((mGraphics) && (paramIdx >= 0)) 
  {
    double  normalizedValue = 0;
    err = GetEffectParameters()->GetParameterNormalizedValue( iParameterID, &normalizedValue );
    
    if (err == AAX_SUCCESS)
    {
      //TODO: does this happen twice?
      mGraphics->SetParameterFromPlug(paramIdx, normalizedValue, true);
    }
  }
  
  return err;
} 

AAX_IEffectGUI* AAX_CALLBACK AAX_CEffectGUI_IPLUG::Create()
{
  return new AAX_CEffectGUI_IPLUG;
}

IPlugAAX::IPlugAAX(IPlugInstanceInfo instanceInfo, 
                                 int nParams, 
                                 const char* channelIOStr, 
                                 int nPresets,
                                 const char* effectName, 
                                 const char* productName, 
                                 const char* mfrName,
                                 int vendorVersion, 
                                 int uniqueID, 
                                 int mfrID, 
                                 int latency,
                                 bool plugDoesMidi, 
                                 bool plugDoesChunks, 
                                 bool plugIsInst,
                                 int plugScChans)
: IPlugBase(nParams,
            channelIOStr,
            nPresets,
            effectName,
            productName,
            mfrName,
            vendorVersion,
            uniqueID,
            mfrID,
            latency,
            plugDoesMidi,
            plugDoesChunks,
            plugIsInst,
            kAPIAAX)

, AAX_CIPlugParameters()
, mTransport(0)
//, mDoesMidi(plugDoesMidi)
{
  Trace(TRACELOC, "%s%s", effectName, channelIOStr);

  SetInputChannelConnections(0, NInChannels(), true);
  SetOutputChannelConnections(0, NOutChannels(), true);  
  
  SetBlockSize(DEFAULT_BLOCK_SIZE);
  SetHost("ProTools", vendorVersion); // TODO:vendor version correct?  
}

AAX_Result IPlugAAX::EffectInit()
{ 
  TRACE;

  AAX_CString bypassID = NULL;
  this->GetMasterBypassParameter( &bypassID );
  mBypassParameter = new AAX_CParameter<bool>(bypassID.CString(), 
                                              AAX_CString("Master Bypass"), 
                                              false, 
                                              AAX_CBinaryTaperDelegate<bool>(),
                                              AAX_CBinaryDisplayDelegate<bool>("bypass", "on"), 
                                              true);
  mBypassParameter->SetNumberOfSteps( 2 );
  mBypassParameter->SetType( AAX_eParameterType_Discrete );
  mParameterManager.AddParameter(mBypassParameter);
    
  for (int i=0;i<NParams();i++)
  {
    IParam *p = GetParam(i);
    AAX_IParameter* param = 0;
    WDL_String paramID;
    paramID.SetFormatted(32, "%i", i+1);
    
    switch (p->Type()) 
    {
      case IParam::kTypeDouble:
      {
        param = new AAX_CParameter<double>(paramID.Get(), 
                                          AAX_CString(p->GetNameForHost()), 
                                          p->GetDefault(), 
                                          AAX_CIPlugTaperDelegate<double>(p->GetMin(), p->GetMax(), p->GetShape()), 
                                          AAX_CNumberDisplayDelegate<double>(), 
                                          p->GetCanAutomate());
        
        param->SetNumberOfSteps(128); // TODO: check this https://developer.digidesign.com/index.php?L1=5&L2=13&L3=56
        param->SetType(AAX_eParameterType_Continuous);

        break;
      }
      case IParam::kTypeInt:
      {
        param = new AAX_CParameter<int>(paramID.Get(), 
                                        AAX_CString(p->GetNameForHost()), 
                                        (int)p->GetDefault(), 
                                        AAX_CLinearTaperDelegate<int>((int)p->GetMin(), (int)p->GetMax()), 
                                        AAX_CNumberDisplayDelegate<int>(), 
                                        p->GetCanAutomate());
        
        param->SetNumberOfSteps(128);
        param->SetType(AAX_eParameterType_Continuous);

        break;
      }
      case IParam::kTypeEnum:
      case IParam::kTypeBool: 
      {
        int nTexts = p->GetNDisplayTexts();
        
        std::map<int, AAX_CString> displayTexts;
        
        for (int j=0; j<p->GetNDisplayTexts(); j++) 
        {
          int value;
          const char* text = p->GetDisplayTextAtIdx(j, &value);
          
          displayTexts.insert(std::pair<int, AAX_CString>(value, AAX_CString(text)) );
        }
        
        param = new AAX_CParameter<int>(paramID.Get(), 
                                        AAX_CString(p->GetNameForHost()), 
                                        (int)p->GetDefault(), 
                                        AAX_CLinearTaperDelegate<int>((int)p->GetMin(), (int)p->GetMax()), 
                                        AAX_CStringDisplayDelegate<int>(displayTexts),
                                        p->GetCanAutomate());
        
        param->SetNumberOfSteps(nTexts);
        param->SetType(AAX_eParameterType_Discrete);
                
        break; 
      }
      default:
        break;
    }
    
    mParameterManager.AddParameter(param);    
  }
  
  AAX_CSampleRate sr;
  Controller()->GetSampleRate(&sr);
  SetSampleRate(sr);
  Reset();
  
  return AAX_SUCCESS;
}

AAX_Result IPlugAAX::UpdateParameterNormalizedValue (AAX_CParamID iParameterID, double iValue, AAX_EUpdateSource iSource )
{
  TRACE;
  
  AAX_Result  result = AAX_SUCCESS;
  
  AAX_IParameter* parameter = mParameterManager.GetParameterByID( iParameterID );
  if (parameter == 0)
    return AAX_ERROR_INVALID_PARAMETER_ID;
  
  // Store the value into the parameter.
  parameter->UpdateNormalizedValue(iValue);
  
  int paramIdx = atoi(iParameterID) - kPTParamIdxOffset;
  
  if ((paramIdx >= 0) && (paramIdx < NParams())) 
  {
    GetParam(paramIdx)->SetNormalized(iValue);
    OnParamChange(paramIdx);      
  }
  
  // Now the control has changed
  result = mPacketDispatcher.SetDirty(iParameterID);
  
  ++mNumPlugInChanges;
  return result;
}

void IPlugAAX::RenderAudio(AAX_SIPlugRenderInfo* ioRenderInfo)
{
  TRACE_PROCESS;

  IMutexLock lock(this);

  // Get bypass parameter value
  bool bypass;
  mBypassParameter->GetValueAsBool(&bypass);
  
  AAX_EStemFormat inFormat, outFormat;
  Controller()->GetInputStemFormat(&inFormat);
  Controller()->GetOutputStemFormat(&outFormat);
  
  // Setup MIDI node pointers
//  AAX_IMIDINode* midiIn = ioRenderInfo->mInputNode;
//  AAX_CMidiStream* midiBuffer = midiIn->GetNodeBuffer();
//  AAX_CMidiPacket* midiBufferPtr = midiBuffer->mBuffer;
//  uint32_t packets_count = midiBuffer->mBufferSize;
//  
//  AAX_IMIDINode* globalNode = ioRenderInfo->mGlobalNode;
//  AAX_CMidiStream* globalBuffer = globalNode->GetNodeBuffer();
//  AAX_CMidiPacket* globalBufferPtr = globalBuffer->mBuffer;
//  uint32_t global_packets = globalBuffer->mBufferSize;
  
  AAX_IMIDINode* transportNode = ioRenderInfo->mTransportNode;
  mTransport = transportNode->GetTransport();

  int32_t numSamples = *(ioRenderInfo->mNumSamples);
  int32_t numInChannels = AAX_STEM_FORMAT_CHANNEL_COUNT(inFormat);
  int32_t numOutChannels = AAX_STEM_FORMAT_CHANNEL_COUNT(outFormat);

  SetInputChannelConnections(0, numInChannels, true);
  SetInputChannelConnections(numInChannels, NInChannels() - numInChannels, false);
  AttachInputBuffers(0, NInChannels(), ioRenderInfo->mAudioInputs, numSamples);
  
  SetOutputChannelConnections(0, numOutChannels, true);
  SetOutputChannelConnections(numOutChannels, NOutChannels() - numOutChannels, false);
  
  AttachOutputBuffers(0, NOutChannels(), ioRenderInfo->mAudioOutputs);
  
  if (bypass) 
  {
    PassThroughBuffers(0.0f, numSamples);
  }
  else 
  {
    ProcessBuffers(0.0f, numSamples);
  }
}

AAX_Result IPlugAAX::GetChunkIDFromIndex( int32_t index, AAX_CTypeID * chunkID )  const 
{
  IPlugAAX* _this = const_cast<IPlugAAX*>(this);

	if (index != 0)
	{
		*chunkID = AAX_CTypeID(0);
		return AAX_ERROR_INVALID_CHUNK_INDEX;
	}
	
	*chunkID = _this->GetUniqueID();
  
	return AAX_SUCCESS;	
}

AAX_Result IPlugAAX::GetChunkSize(AAX_CTypeID chunkID, uint32_t * oSize ) const
{
  TRACE;
  
  IPlugAAX* _this = const_cast<IPlugAAX*>(this);
  
  if (chunkID == _this->GetUniqueID()) 
  {
    ByteChunk IPlugChunk;
    
    _this->InitChunkWithIPlugVer(&IPlugChunk);
    
    if (_this->SerializeState(&IPlugChunk))
    {
      *oSize = IPlugChunk.Size();
    }
    
    return AAX_SUCCESS;
  }
  else 
  {
    *oSize = 0;
    return AAX_ERROR_INVALID_CHUNK_ID;
  }
}

AAX_Result IPlugAAX::GetChunk(AAX_CTypeID chunkID, AAX_SPlugInChunk * oChunk ) const
{
  TRACE;
  IPlugAAX* _this = const_cast<IPlugAAX*>(this);

  //called when project is saved
  if (chunkID == _this->GetUniqueID()) 
  {
    ByteChunk IPlugChunk;
    
    _this->InitChunkWithIPlugVer(&IPlugChunk);
    
    if (_this->SerializeState(&IPlugChunk)) 
    {
      oChunk->fSize = IPlugChunk.Size();
      memcpy(oChunk->fData, IPlugChunk.GetBytes(), IPlugChunk.Size());
      return AAX_SUCCESS;
    }
  }
  
  return AAX_ERROR_INVALID_CHUNK_ID;
}

AAX_Result IPlugAAX::SetChunk(AAX_CTypeID chunkID, const AAX_SPlugInChunk * iChunk )
{
  TRACE;
  
  if (chunkID == GetUniqueID())
  {    
    ByteChunk IPlugChunk;
    IPlugChunk.PutBytes(iChunk->fData, iChunk->fSize);
    int pos = 0;
    GetIPlugVerFromChunk(&IPlugChunk, &pos);
    pos = UnserializeState(&IPlugChunk, pos);

    RedrawParamControls(); //TODO: what about icontrols not linked to params how do they get redrawn - setdirty via UnserializeState()?
    mNumPlugInChanges++; // necessary in order to cause CompareActiveChunk() to get called again and turn off the compare light 
    
    return AAX_SUCCESS;
  }
  
  return AAX_ERROR_INVALID_CHUNK_ID;
}

AAX_Result IPlugAAX::CompareActiveChunk(const AAX_SPlugInChunk * aChunkP, AAX_CBoolean * aIsEqualP ) const 
{
  TRACE;

  IPlugAAX* _this = const_cast<IPlugAAX*>(this);

	if (aChunkP->fChunkID != _this->GetUniqueID()) 
	{
		*aIsEqualP = true;
		return AAX_SUCCESS; 
	}
  
	*aIsEqualP = true;
  
  ByteChunk IPlugChunk;
  _this->InitChunkWithIPlugVer(&IPlugChunk);
  _this->SerializeState(&IPlugChunk);

  if(memcmp(IPlugChunk.GetBytes(), aChunkP->fData, IPlugChunk.Size()) != 0)
  {
    *aIsEqualP = false;
  }
  
  return AAX_SUCCESS;
}  

void IPlugAAX::BeginInformHostOfParamChange(int idx)
{
  TRACE;

  WDL_String paramID;
  paramID.SetFormatted(32, "%i", idx+1);
  TouchParameter(paramID.Get());
}

void IPlugAAX::InformHostOfParamChange(int idx, double normalizedValue)
{
  TRACE;

  WDL_String paramID;
  paramID.SetFormatted(32, "%i", idx+1);
  SetParameterNormalizedValue(paramID.Get(), normalizedValue );
}

void IPlugAAX::EndInformHostOfParamChange(int idx)
{
  TRACE;

  WDL_String paramID;
  paramID.SetFormatted(32, "%i", idx+1);
  ReleaseParameter(paramID.Get());
}

void IPlugAAX::InformHostOfProgramChange()
{
}

int IPlugAAX::GetSamplePos()
{ 
  int64_t samplePos;
  mTransport->GetCurrentNativeSampleLocation(&samplePos);
  return (int) samplePos;
}

double IPlugAAX::GetTempo()
{
  double tempo;
  mTransport->GetCurrentTempo(&tempo);
  return tempo;
}

void IPlugAAX::GetTime(ITimeInfo* pTimeInfo)
{
  int32_t num, denom;
  int64_t ppqPos, samplePos, cStart, cEnd;

  mTransport->GetCurrentTempo(&pTimeInfo->mTempo);
  mTransport->IsTransportPlaying(&pTimeInfo->mTransportIsRunning);
  
  mTransport->GetCurrentMeter(&num, &denom);
  pTimeInfo->mNumerator = (int) num;
  pTimeInfo->mDenominator = (int) denom;
  
  mTransport->GetCurrentTickPosition(&ppqPos);
  pTimeInfo->mPPQPos = (double) ppqPos;
  
  mTransport->GetCurrentNativeSampleLocation(&samplePos);
  pTimeInfo->mSamplePos = (double) samplePos;
  
  mTransport->GetCurrentLoopPosition(&pTimeInfo->mTransportLoopEnabled, &cStart, &cEnd);
  pTimeInfo->mCycleStart = (double) cStart;
  pTimeInfo->mCycleEnd = (double) cEnd;
  
  //pTimeInfo->mLastBar ??
}

void IPlugAAX::GetTimeSig(int* pNum, int* pDenom)
{
  int32_t num, denom;
  mTransport->GetCurrentMeter(&num, &denom);
  *pNum = (int) num;
  *pDenom = (int) denom;
}

void IPlugAAX::ResizeGraphics(int w, int h)
{
  if (GetGUI()) 
  {
    AAX_Point oEffectViewSize;
    oEffectViewSize.horz = (float) w;
    oEffectViewSize.vert = (float) h;
    GetGUI()->GetViewContainer()->SetViewSize(oEffectViewSize);
  }
}

void IPlugAAX::SetLatency(int latency)
{
  Controller()->SetSignalLatency(latency);
}

// TODO: SendMidiMsg()
bool IPlugAAX::SendMidiMsg(IMidiMsg* pMsg)
{
  return false;
}

// TODO: SendMidiMsgs()
bool IPlugAAX::SendMidiMsgs(WDL_TypedBuf<IMidiMsg>* pMsgs)
{
  return false;
}