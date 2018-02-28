#ifndef _IPLUGAPI_
#define _IPLUGAPI_

/**
 * @file
 * @copydoc IPlugAAX
 */


#include "IPlugPlatform.h"
#include "IPlugBase_select.h"
#include "IPlugAAX_Parameters.h"
#include "AAX_CEffectGUI.h"

#include "AAX_Push8ByteStructAlignment.h"

const int kAAXParamIdxOffset = 1;

/** Used to pass various instance info to the API class */
struct IPlugInstanceInfo {};

class IPlugAAX;

/**  AAX_CEffectGUI base class for an IPlug AAX view */
class AAX_CEffectGUI_IPLUG : public AAX_CEffectGUI
{
public:
  AAX_CEffectGUI_IPLUG() {}
  ~AAX_CEffectGUI_IPLUG() {}
  static AAX_IEffectGUI* AAX_CALLBACK Create();
  AAX_Result SetControlHighlightInfo(AAX_CParamID iParameterID, AAX_CBoolean iIsHighlighted, AAX_EHighlightColor iColor);
  
private:
  void CreateViewContents();
  void CreateViewContainer();
  void DeleteViewContainer();
  AAX_Result GetViewSize ( AAX_Point *oEffectViewSize ) const;
  AAX_Result ParameterUpdated (const char* iParameterID);
private:
  IPlugAAX* mPlug = nullptr;
};

/**  AAX API base class for an IPlug plug-in, inherits from IPlugBase or IPlugBaseGraphics 
*   @ingroup APIClasses
*/
class IPlugAAX : public IPLUG_BASE_CLASS
               , public IPlugProcessor<PLUG_SAMPLE_DST>
               , public IPlugPresetHandler
               , public AAX_CIPlugParameters
{
public:
  IPlugAAX(IPlugInstanceInfo instanceInfo, IPlugConfig config);
  ~IPlugAAX();
  
  //IPlugBase Overrides
  void BeginInformHostOfParamChange(int idx) override;
  void InformHostOfParamChange(int idx, double normalizedValue) override;
  void EndInformHostOfParamChange(int idx) override;
  
  void InformHostOfProgramChange() override { }; //NA
  
  void ResizeGraphics() override;
  
  //IPlug Processor Overrides
  void SetLatency(int samples) override;
  bool SendMidiMsg(const IMidiMsg& msg) override;
  
  AAX_Result UpdateParameterNormalizedValue(AAX_CParamID iParameterID, double iValue, AAX_EUpdateSource iSource ) override;
  
  //AAX_CIPlugParameters Overrides
  static AAX_CEffectParameters *AAX_CALLBACK Create();
  AAX_Result EffectInit() override;
  void RenderAudio(AAX_SIPlugRenderInfo* ioRenderInfo) override;
  
  //AAX_CEffectParameters Overrides
  AAX_Result GetChunkIDFromIndex(int32_t index, AAX_CTypeID* pChunkID) const override;
  AAX_Result GetChunkSize(AAX_CTypeID chunkID, uint32_t* pChunkSize) const override;
  AAX_Result GetChunk(AAX_CTypeID chunkID, AAX_SPlugInChunk* pChunk) const override;
  AAX_Result SetChunk(AAX_CTypeID chunkID, const AAX_SPlugInChunk* pChunk) override;
  AAX_Result CompareActiveChunk(const AAX_SPlugInChunk* pChunk, AAX_CBoolean* pIsEqual) const override;

  //IPlugAAX
  /** This is needed in chunks based plugins to tell PT a non-indexed param changed and to turn on the compare light. You can call this method from your plug-in implementation by doing a dynamic_cast in order to convert an "IPlug" into a "IPlugAAX"
   */
  void DirtyPTCompareState() { mNumPlugInChanges++; }

private:
  AAX_CParameter<bool>* mBypassParameter = nullptr;
  AAX_ITransport* mTransport = nullptr;
  WDL_PtrList<WDL_String> mParamIDs;
};

IPlugAAX* MakePlug();

#include "AAX_PopStructAlignment.h"

#endif
