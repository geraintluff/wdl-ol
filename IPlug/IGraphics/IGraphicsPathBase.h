#pragma once
#include <algorithm>

#include "IGraphics.h"
#include "nanosvg.h"

class IGraphicsPathBase : public IGraphics
{
public:
  IGraphicsPathBase(IDelegate& dlg, int w, int h, int fps) 
  : IGraphics(dlg, w, h, fps) 
  {}

  void DrawRotatedBitmap(IBitmap& bitmap, int destCtrX, int destCtrY, double angle, int yOffsetZeroDeg, const IBlend* pBlend) override
  {
    //TODO: offset support
    
    float width = bitmap.W();
    float height = bitmap.H();
    
    PathStateSave();
    PathTransformTranslate(destCtrX, destCtrY);
    PathTransformRotate(angle);
    DrawBitmap(bitmap, IRECT(-width * 0.5, - height * 0.5, width * 0.5, height * 0.5), 0, 0, pBlend);
    PathStateRestore();
  }
  
  void DrawRotatedMask(IBitmap& base, IBitmap& mask, IBitmap& top, int x, int y, double angle, const IBlend* pBlend) override
  {
    float width = base.W();
    float height = base.H();
    
    IBlend addBlend(kBlendAdd);
    PathStateSave();
    DrawBitmap(base, IRECT(x, y, x + width, y + height), 0, 0, pBlend);
    PathTransformTranslate(x + 0.5 * width, y + 0.5 * height);
    PathTransformRotate(angle);
    DrawBitmap(mask, IRECT(-width * 0.5, - height * 0.5, width * 0.5, height * 0.5), 0, 0, &addBlend);
    DrawBitmap(top, IRECT(-width * 0.5, - height * 0.5, width * 0.5, height * 0.5), 0, 0, pBlend);
    PathStateRestore();
  }
  
  void DrawLine(const IColor& color, float x1, float y1, float x2, float y2, const IBlend* pBlend) override
  {
    PathMoveTo(x1, y1);
    PathLineTo(x2, y2);
    PathStroke(color, 1.0, IStrokeOptions(), pBlend);
  }
  
  void DrawTriangle(const IColor& color, float x1, float y1, float x2, float y2, float x3, float y3, const IBlend* pBlend) override
  {
    PathTriangle(x1, y1, x2, y2, x3, y3);
    PathStroke(color, 1.0, IStrokeOptions(), pBlend);
  }
  
  void DrawRect(const IColor& color, const IRECT& bounds, const IBlend* pBlend) override
  {
    PathRect(bounds);
    PathStroke(color, 1.0, IStrokeOptions(), pBlend);
  }
  
  void DrawRoundRect(const IColor& color, const IRECT& bounds, float corner, const IBlend* pBlend) override
  {
    PathRoundRect(bounds, corner);
    PathStroke(color, 1.0, IStrokeOptions(), pBlend);
  }
  
  void DrawConvexPolygon(const IColor& color, float* x, float* y, int nPoints, const IBlend* pBlend) override
  {
    PathConvexPolygon(x, y, nPoints);
    PathStroke(color, 1.0, IStrokeOptions(), pBlend);
  }
  
  void DrawArc(const IColor& color, float cx, float cy, float r, float aMin, float aMax, const IBlend* pBlend) override
  {
    PathArc(cx, cy, r, aMin -90.f, aMax -90.f);
    PathStroke(color, 1.0, IStrokeOptions(), pBlend);
  }
  
  void DrawCircle(const IColor& color, float cx, float cy, float r, const IBlend* pBlend) override
  {
    PathCircle(cx, cy, r);
    PathStroke(color, 1.0, IStrokeOptions(), pBlend);
  }
  
  void DrawDottedRect(const IColor& color, const IRECT& bounds, const IBlend* pBlend) override
  {
    float dashLength = 2;
    IStrokeOptions options;
    options.mDash.SetDash(&dashLength, 0.0, 1);
    PathRect(bounds);
    PathStroke(color, 1.0, options, pBlend);
  }

  void FillTriangle(const IColor& color, float x1, float y1, float x2, float y2, float x3, float y3, const IBlend* pBlend) override
  {
    PathTriangle(x1, y1, x2, y2, x3, y3);
    PathFill(color, IFillOptions(), pBlend);
  }
  
  void FillRect(const IColor& color, const IRECT& bounds, const IBlend* pBlend) override
  {
    PathRect(bounds);
    PathFill(color, IFillOptions(), pBlend);
  }
  
  void FillRoundRect(const IColor& color, const IRECT& bounds, float corner, const IBlend* pBlend) override
  {
    PathRoundRect(bounds, corner);
    PathFill(color, IFillOptions(), pBlend);
  }
  
  void FillConvexPolygon(const IColor& color, float* x, float* y, int nPoints, const IBlend* pBlend) override
  {
    PathConvexPolygon(x, y, nPoints);
    PathFill(color, IFillOptions(), pBlend);
  }
  
  void FillArc(const IColor& color, float cx, float cy, float r, float aMin, float aMax, const IBlend* pBlend) override
  {
    PathMoveTo(cx, cy);
    PathArc(cx, cy, r, aMin - 90.f, aMax - 90.f);
    PathClose();
    PathFill(color, IFillOptions(), pBlend);
  }
  
  void FillCircle(const IColor& color, float cx, float cy, float r, const IBlend* pBlend) override
  {
    PathCircle(cx, cy, r);
    PathFill(color, IFillOptions(), pBlend);
  }
  
  void DrawPoint(const IColor& color, float x, float y, const IBlend* pBlend) override
  {
    FillRect(color, IRECT(x, y, 1, 1), pBlend);
  }
  
  bool HasPathSupport() const override { return true; }
  
  void PathTriangle(float x1, float y1, float x2, float y2, float x3, float y3) override
  {
    PathMoveTo(x1, y1);
    PathLineTo(x2, y2);
    PathLineTo(x3, y3);
    PathClose();
  }
  
  void PathRect(const IRECT& bounds) override
  {
    PathMoveTo(bounds.L, bounds.T);
    PathLineTo(bounds.R, bounds.T);
    PathLineTo(bounds.R, bounds.B);
    PathLineTo(bounds.L, bounds.B);
    PathClose();
  }
  
  void PathRoundRect(const IRECT& bounds, float cr) override
  {
    const double y = bounds.B - bounds.H();
    PathMoveTo(bounds.L, y + cr);
    PathArc(bounds.L + cr, y + cr, cr, 180.0, 270.0);
    PathArc(bounds.L + bounds.W() - cr, y + cr, cr, 270.0, 360.0);
    PathArc(bounds.L + bounds.W() - cr, y + bounds.H() - cr, cr, 0.0, 90.0);
    PathArc(bounds.L + cr, y + bounds.H() - cr, cr, 90.0, 180.0);
    PathClose();
  }
  
  void PathCircle(float cx, float cy, float r) override
  {
    PathMoveTo(cx + r, cy);
    PathArc(cx, cy, r, 0.f, 360.f);
    PathClose();
  }
  
  void PathConvexPolygon(float* x, float* y, int nPoints) override
  {
    PathMoveTo(x[0], y[0]);
    for(int i = 1; i < nPoints; i++)
      PathLineTo(x[i], y[i]);
    PathClose();
  }
  
  virtual void PathStateSave() = 0;
  virtual void PathStateRestore() = 0;
  
  virtual void PathTransformTranslate(float x, float y) = 0;
  virtual void PathTransformScale(float scale) = 0;
  virtual void PathTransformRotate(float angle) = 0;
  
  void DrawSVG(ISVG& svg, const IRECT& dest, const IBlend* pBlend) override
  {
    double xScale = dest.W() / svg.W();
    double yScale = dest.H() / svg.H();
    double scale = xScale < yScale ? xScale : yScale;
    
    PathStateSave();
    PathTransformTranslate(dest.L, dest.T);
    ClipRegion(IRECT(0, 0, dest.W(), dest.H()));
    PathTransformScale(scale);
    RenderNanoSVG(svg.mImage);
    PathStateRestore();
  }
  
  void DrawRotatedSVG(ISVG& svg, float destCtrX, float destCtrY, float width, float height, double angle, const IBlend* pBlend) override
  {
    PathStateSave();
    PathTransformTranslate(destCtrX, destCtrY);
    PathTransformRotate(angle);
    DrawSVG(svg, IRECT(-width * 0.5, - height * 0.5, width * 0.5, height * 0.5), pBlend);
    PathStateRestore();
  }

private:
  IPattern GetSVGPattern(const NSVGpaint& paint, float opacity)
  {
    int alpha = std::min(255, std::max(int(roundf(opacity * 255.f)), 0));
    
    switch (paint.type)
    {
      case NSVG_PAINT_COLOR:
        return IColor(alpha, (paint.color >> 0) & 0xFF, (paint.color >> 8) & 0xFF, (paint.color >> 16) & 0xFF);
        
      case NSVG_PAINT_LINEAR_GRADIENT:
      case NSVG_PAINT_RADIAL_GRADIENT:
      {
        NSVGgradient* pGrad = paint.gradient;
        
        IPattern pattern(paint.type == NSVG_PAINT_LINEAR_GRADIENT ? kLinearPattern : kRadialPattern);
        
        // Set Extend Rule
        switch (pGrad->spread)
        {
          case NSVG_SPREAD_PAD:       pattern.mExtend = kExtendPad;       break;
          case NSVG_SPREAD_REFLECT:   pattern.mExtend = kExtendReflect;   break;
          case NSVG_SPREAD_REPEAT:    pattern.mExtend = kExtendRepeat;    break;
        }
        
        // Copy Stops        
        for (int i = 0; i < pGrad->nstops; i++)
        {
          int color = pGrad->stops[i].color;
          pattern.AddStop(IColor(255, (color >> 0) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF), pGrad->stops[i].offset);
        }
        
        // Copy transform        
        pattern.SetTransform(pGrad->xform[0], pGrad->xform[1], pGrad->xform[2], pGrad->xform[3], pGrad->xform[4], pGrad->xform[5]);
        
        return pattern;
      }
      default:
        return IColor(alpha, 0, 0, 0);
    }
  }
  
  void RenderNanoSVG(NSVGimage* pImage)
  {
    for (NSVGshape* pShape = pImage->shapes; pShape; pShape = pShape->next)
    {
      if (!(pShape->flags & NSVG_FLAGS_VISIBLE))
        continue;
      
      for (NSVGpath* pPath = pShape->paths; pPath; pPath = pPath->next)
      {
        PathMoveTo(pPath->pts[0], pPath->pts[1]);
        
        for (int i = 0; i < pPath->npts - 1; i += 3)
        {
          float *p = pPath->pts + i * 2 + 2;
          PathCurveTo(p[0], p[1], p[2], p[3], p[4], p[5]);
        }
        
        if (pPath->closed)
          PathClose();
      }
      
      // Fill
      if (pShape->fill.type != NSVG_PAINT_NONE)
      {
        IFillOptions options;
        
        if (pShape->fillRule == NSVG_FILLRULE_EVENODD)
          options.mFillRule = kFillEvenOdd;
        else
          options.mFillRule = kFillWinding;
        
        options.mPreserve = pShape->stroke.type != NSVG_PAINT_NONE;
        PathFill(GetSVGPattern(pShape->fill, pShape->opacity), options, nullptr);
      }
      
      // Stroke
      if (pShape->stroke.type != NSVG_PAINT_NONE)
      {
        IStrokeOptions options;
        
        options.mMiterLimit = pShape->miterLimit;
        
        switch (pShape->strokeLineCap)
        {
          case NSVG_CAP_BUTT:   options.mCapOption = kCapButt;    break;
          case NSVG_CAP_ROUND:  options.mCapOption = kCapRound;   break;
          case NSVG_CAP_SQUARE: options.mCapOption = kCapSquare;  break;
        }
        
        switch (pShape->strokeLineJoin)
        {
          case NSVG_JOIN_MITER:   options.mJoinOption = kJoinMiter;   break;
          case NSVG_JOIN_ROUND:   options.mJoinOption = kJoinRound;   break;
          case NSVG_JOIN_BEVEL:   options.mJoinOption = kJoinBevel;   break;
        }
        
        options.mDash.SetDash(pShape->strokeDashArray, pShape->strokeDashOffset, pShape->strokeDashCount);
        
        PathStroke(GetSVGPattern(pShape->stroke, pShape->opacity), pShape->strokeWidth, options, nullptr);
      }
    }
  }
};
