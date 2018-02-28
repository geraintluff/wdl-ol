#pragma once

/**
 * @file
 * @brief A collection of IControls for common UI widgets, such as knobs, sliders, switches
 */

#include "IControl.h"

/**
 * \defgroup Controls IGraphics::IControls
 * @{
 */

#pragma mark - Vector Controls

/** A vector switch control. Click to cycle through states. */
class IVSwitchControl : public ISwitchControlBase
                      , public IVectorBase
{
public:
  IVSwitchControl(IDelegate& dlg, IRECT rect, int paramIdx = kNoParameter, IActionFunction actionFunc = nullptr,
                  const IVColorSpec& colorSpec = DEFAULT_SPEC,
                  uint32_t numStates = 2, EDirection dir = kVertical);

  void Draw(IGraphics& graphics)  override;
  
private:
  float mStep;
  EDirection mDirection;
};

/** A vector knob control drawn using graphics primitves */
class IVKnobControl : public IKnobControlBase
                    , public IVectorBase
{
public:
  IVKnobControl(IDelegate& dlg, IRECT rect, int paramIdx,
                const IVColorSpec& colorSpec = DEFAULT_SPEC,
                float rMin = 0.f, float rMax = 1.f, float aMin = -135.f, float aMax = 135.f,
                EDirection direction = kVertical, double gearing = DEFAULT_GEARING);
  ~IVKnobControl() {}

  void Draw(IGraphics& graphics) override;
  
protected:
  float mAngleMin, mAngleMax, mInnerRadius, mOuterRadius;
};

/** A vector knob control which rotates an SVG image */
class IVSVGKnob : public IKnobControlBase
{
public:
  IVSVGKnob(IDelegate& dlg, IRECT rect, ISVG& svg, int paramIdx = kNoParameter)
    : IKnobControlBase(dlg, rect, paramIdx)
    , mSVG(svg)
  {
  }

  void Draw(IGraphics& g) override
  {
#ifdef IGRAPHICS_LICE
    g.DrawText(mText, "NO LICE SVG", mRECT);
#else
    g.DrawRotatedSVG(mSVG, mRECT.MW(), mRECT.MH(), mRECT.W(), mRECT.H(), mStartAngle + mValue * (mEndAngle - mStartAngle));
#endif
  }

  void SetSVG(ISVG& svg)
  {
    mSVG = svg;
    SetDirty(false);
  }

private:
  ISVG mSVG;
  float mStartAngle = -135.f;
  float mEndAngle = 135.f;
};

class IVSliderControl : public IControl
                      , public IVectorBase
{
public:
  IVSliderControl(IDelegate& dlg, IRECT rect, int paramIdx, const IVColorSpec& colorSpec = DEFAULT_SPEC, EDirection dir = kVertical)
  : IControl(dlg, rect, paramIdx)
  , IVectorBase(colorSpec)
  , mDirection(dir)
  {
  }
  
  void Draw(IGraphics& graphics) override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override { SnapToMouse(x, y, mDirection, mTrack); }
  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override { SnapToMouse(x, y, mDirection, mTrack); }
  void OnResize() override;
private:
  EDirection mDirection;
  IRECT mTrack;
};

/*
 
 IVKeyboardControl by Eugene Yakshin, 2018
 
 based on
 
 IKeyboardControl
 (c) Theo Niessink 2009, 2010
 <http://www.taletn.com/>
 
 This software is provided 'as-is', without any express or implied
 warranty. In no event will the authors be held liable for any damages
 arising from the use of this software.
 
 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:
 
 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software in a
 product, an acknowledgment in the product documentation would be
 appreciated but is not required.
 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.
 3. This notice may not be removed or altered from any source distribution.
 
 
 This keyboard is runtime customizable. Any key range is supported.
 Key proportions, colors and some other design elements can be changed at any time too.
 See the interface for details.
 */

class IVKeyboardControl : public IControl
                        , public IVectorBase
{
public:
  static const IColor DEFAULT_BK_COLOR;
  static const IColor DEFAULT_WK_COLOR;
  static const IColor DEFAULT_PK_COLOR;
  static const IColor DEFAULT_FR_COLOR;
  
  // map to IVectorBase colors
  enum EVKColor
  {
    kBK = kFG,
    kWK = kBG,
    kPK = kHL,
    //kFR = kFR
  };
  
  IVKeyboardControl(IDelegate& dlg, IRECT rect,
                    int minNote = 36, int maxNote = 60);
  
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  void OnMouseUp(float x, float y, const IMouseMod& mod) override;
  void OnMouseOut() override;
  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override;
  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override;
  void OnMouseOver(float x, float y, const IMouseMod& mod) override;
  void OnResize() override;
  
  void Draw(IGraphics& graphics) override;
  
  void SetMinMaxNote(int min, int max, bool keepWidth = true);
  void SetNoteIsPlayed(int noteNum, bool played);
  void SetBlackToWhiteWidthAndHeightRatios(float widthR, float heightR = 0.6);
  void SetHeight(float h, bool keepProportions = false);
  void SetWidth(float w, bool keepProportions = false);
  void SetShowNotesAndVelocity(bool show);
  void SetColors(const IColor bkColor, const IColor& wkColor, const IColor& pkColor = DEFAULT_PK_COLOR, const IColor& frColor = DEFAULT_FR_COLOR);
  
  void SetDrawShadows(bool draw)
  {
    mDrawShadows = draw;
    SetDirty();
  }
  
  void SetDrawBorders(bool draw)
  {
    mDrawBorders = draw;
    SetDirty();
  }
  
  // returns pressed key number inside the keyboard
  int GetKey() const
  {
    return mKey;
  }
  // returns pressed MIDI note number
  int GetNote() const
  {
    if (mKey > -1) return mMinNote + mKey;
    else return -1;
  }
  
  double GetVelocity() const { return mVelocity * 127.f; }
  double GetVelocityNormalized() const { return mVelocity; }
  int GetVelocityInt() const { return (int)(mVelocity * 127. + 0.5); }
  
private:
  void RecreateKeyBounds(bool keepWidth);
  int GetKeyUnderMouse(float x, float y);
  void UpdateVelocity(float y);
  void GetNoteNameStr(int midiNoteNum, bool addOctave, WDL_String& str);
  bool IsBlackKey(int i) const { return *(mIsBlackKeyList.Get() + i); }
  float KeyLCoord(int i) { return *(mKeyLCoords.Get() + i); }
  float* KeyLCoordPtr(int i) { return mKeyLCoords.Get() + i; }
  bool NoteIsPlayed(int i) const { return *(mNoteIsPlayed.Get() + i); }
  int NumKeys() const { return mMaxNote - mMinNote + 1; }
  
  float CalcBKWidth() const
  {
    auto w = mWKWidth;
    if (NumKeys() > 1)
      w *= mBKWidthR;
    return w;
  }
  
protected:
  bool mShowNoteAndVel = false;
  bool mDrawShadows = true;
  bool mDrawBorders = true;

  float mWKWidth = 0.f;
  float mBKWidthR = 0.6f;
  float mBKHeightRatio = 0.6f;
  float mBKAlpha = 100.f;
  int mKey = -1;
  int mMouseOverKey = -1;
  float mVelocity = 0.f;
  bool mVelByWheel = false;
  int mMinNote, mMaxNote;
  WDL_TypedBuf<bool> mIsBlackKeyList;
  WDL_TypedBuf<bool> mNoteIsPlayed;
  WDL_TypedBuf<float> mKeyLCoords;
};

#pragma mark - Bitmap Controls

/** A vector switch control. Click to cycle through states. */
class IBSwitchControl : public IBitmapControl
{
public:
  IBSwitchControl(IDelegate& dlg, float x, float y, int paramIdx, IBitmap& bitmap)
  : IBitmapControl(dlg, x, y, paramIdx, bitmap) {}
  ~IBSwitchControl() {}

  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {  OnMouseDown(x, y, mod); }
};

/** A slider with a bitmap for the handle. The bitmap snaps to a mouse click or drag. */
class IBSliderControl : public IControl
{
public:
  IBSliderControl(IDelegate& dlg, float x, float y, int len, int paramIdx,
                  IBitmap& bitmap, EDirection direction = kVertical, bool onlyHandle = false);
  ~IBSliderControl() {}

  virtual void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  virtual void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override { return SnapToMouse(x, y); }
  virtual void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override;
  virtual void Draw(IGraphics& graphics) override;
  virtual bool IsHit(float x, float y) const override;
  virtual void OnRescale() override;
  virtual void OnResize() override;

  int GetLength() const { return mLen; }
  int GetHandleHeadroom() const { return mHandleHeadroom; }
  double GetHandleValueHeadroom() const { return (double) mHandleHeadroom / (double) mLen; }
  IRECT GetHandleRECT(double value = -1.0) const;
protected:
  virtual void SnapToMouse(float x, float y);
  int mLen, mHandleHeadroom;
  IBitmap mHandleBitmap;
  EDirection mDirection;
  bool mOnlyHandle;
};

/** Display monospace bitmap font text */
// TODO: fix Centre/Right aligned behaviour when string exceeds bounds or should wrap onto new line
class IBTextControl : public ITextControl
{
public:
  IBTextControl(IDelegate& dlg, IRECT rect, IBitmap& bitmap, const IText& text = DEFAULT_TEXT, const char* str = "", int charWidth = 6, int charHeight = 12, int charOffset = 0, bool multiLine = false, bool vCenter = true, EBlendType bl = kBlendNone)
  : ITextControl(dlg, rect, text, str)
  , mCharWidth(charWidth)
  , mCharHeight(charHeight)
  , mCharOffset(charOffset)
  , mMultiLine(multiLine)
  , mVCentre(vCenter)
  , mTextBitmap(bitmap)
  {
    mStr.Set(str);
  }

  void Draw(IGraphics& graphics) override
  {
    graphics.DrawBitmapedText(mTextBitmap, mRECT, mText, &mBlend, mStr.Get(), mVCentre, mMultiLine, mCharWidth, mCharHeight, mCharOffset);
  }

protected:
  WDL_String mStr;
  int mCharWidth, mCharHeight, mCharOffset;
  bool mMultiLine;
  bool mVCentre;
  IBitmap mTextBitmap;
};

/**@}*/

