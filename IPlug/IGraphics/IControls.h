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

class IVButtonControl : public IControl,
                        public IVectorBase
{
public:

  static const IColor DEFAULT_BG_COLOR;
  static const IColor DEFAULT_PR_COLOR;
  static const IColor DEFAULT_TXT_COLOR;
  static const IColor DEFAULT_FR_COLOR;

  // map to IVectorBase colors
  enum EVBColor
  {
    bTXT = kFG,
    bBG = kBG,
    bPR = kHL,
    bFR = kFR
  };

  IVButtonControl(IDelegate& dlg, IRECT rect, int param,
                  const char *txtOff = "off", const char *txtOn = "on");
  ~IVButtonControl() {};

  void Draw(IGraphics& graphics) override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;

  void SetTexts(const char *txtOff, const char *txtOn, bool fitToText = false, float pad = 10.0);

  void SetDrawBorders(bool draw)
  {
    mDrawBorders = draw;
    SetDirty(false);
  }
  void SetDrawShadows(bool draw, bool keepButtonRect = true);
  void SetEmboss(bool emboss, bool keepButtonRect = true);
  void SetShadowOffset(float offset, bool keepButtonRect = true);
  void SetRect(IRECT r)
  {
    mRECT = mTargetRECT = r;
    SetDirty(false);
  }

protected:
  WDL_String mTxtOff, mTxtOn;
  float mTxtH[2]; // [off, on], needed for nice multiline text drawing
  float mTxtW[2];

  bool mDrawBorders = true;
  bool mDrawShadows = true;
  bool mEmboss = false;
  float mShadowOffset = 3.0;

  void DrawInnerShadowForRect(IRECT r, IColor shadowColor, IGraphics& graphics);
  void DrawOuterShadowForRect(IRECT r, IColor shadowColor, IGraphics& graphics)
  {
    auto sr = ShiftRectBy(r, mShadowOffset, mShadowOffset);
    graphics.FillRect(shadowColor, sr);
  }
  IRECT GetRectToAlignTextIn(IRECT r, int state);
  IRECT GetRectToFitTextIn(IRECT r, float fontSize, float widthInSymbols, float numLines, float padding = 0.0);
  IRECT GetButtonRect();
  IRECT ShiftRectBy(IRECT r, float x, float y = 0.0)
  {
    return IRECT(r.L + x, r.T + y, r.R + x, r.B + y);
  }
};

class IVContactControl : public IVButtonControl
{
public:
  IVContactControl(IDelegate& dlg, IRECT rect, int param,
                   const char *txtOff = "off", const char *txtOn = "on") :
    IVButtonControl(dlg, rect, param, txtOff, txtOn) {};

  ~IVContactControl() {};

  void OnMouseUp(float x, float y, const IMouseMod& mod) override
  {
    mValue = 0.0;
    SetDirty();
  }
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
//class IBSliderControl : public IControl
//{
//public:
//  IBSliderControl(IDelegate& dlg, float x, float y, int len, int paramIdx,
//                  IBitmap& bitmap, EDirection direction = kVertical, bool onlyHandle = false);
//  ~IBSliderControl() {}
//
//  virtual void OnMouseDown(float x, float y, const IMouseMod& mod) override;
//  virtual void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override { return SnapToMouse(x, y); }
//  virtual void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override;
//  virtual void Draw(IGraphics& graphics) override;
//  virtual bool IsHit(float x, float y) const override;
//  virtual void OnRescale() override;
//  virtual void OnResize() override;
//
//  int GetLength() const { return mLen; }
//  int GetHandleHeadroom() const { return mHandleHeadroom; }
//  double GetHandleValueHeadroom() const { return (double) mHandleHeadroom / (double) mLen; }
//  IRECT GetHandleRECT(double value = -1.0) const;
//protected:
//  virtual void SnapToMouse(float x, float y);
//  int mLen, mHandleHeadroom;
//  IBitmap mHandleBitmap;
//  EDirection mDirection;
//  bool mOnlyHandle;
//};

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

