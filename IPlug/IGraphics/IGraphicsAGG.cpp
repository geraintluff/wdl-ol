#include <cmath>

#include "IGraphicsAGG.h"

static StaticStorage<agg::font> s_fontCache;

// Utility

inline const agg::rgba8 AGGColor(const IColor& color, const IBlend* pBlend = nullptr)
{
  return agg::rgba8(color.R, color.G, color.B, (BlendWeight(pBlend) * color.A));
}

inline agg::comp_op_e AGGBlendMode(const IBlend* pBlend)
{
  if (!pBlend)
    return agg::comp_op_src;
  
  switch (pBlend->mMethod)
  {
    case kBlendClobber: return agg::comp_op_src_over;
    case kBlendAdd: return agg::comp_op_plus;
    case kBlendColorDodge: return agg::comp_op_color_dodge;
    case kBlendNone:
    default:
      return agg::comp_op_src_over;
  }
}

inline const agg::cover_type AGGCover(const IBlend* pBlend = nullptr)
{
  if (!pBlend)
    return 255;
  
  return std::max(agg::cover_type(0), std::min(agg::cover_type(roundf(pBlend->mWeight * 255.f)), agg::cover_type(255)));
}

// Rasterizing

template <typename FuncType, typename ColorArrayType>
void GradientRasterize(IGraphicsAGG::Rasterizer& rasterizer, const FuncType& gradientFunc, agg::trans_affine& xform, ColorArrayType& colorArray)
{
  IGraphicsAGG::SpanAllocatorType spanAllocator;
  IGraphicsAGG::InterpolatorType spanInterpolator(xform);
  
  // Gradient types
  
  typedef agg::span_gradient<agg::rgba8, IGraphicsAGG::InterpolatorType, FuncType, ColorArrayType> SpanGradientType;
  typedef agg::renderer_scanline_aa<IGraphicsAGG::RenbaseType, IGraphicsAGG::SpanAllocatorType, SpanGradientType> RendererGradientType;
  
  // Gradient objects
  
  SpanGradientType spanGradient(spanInterpolator, gradientFunc, colorArray, 0, 512);
  RendererGradientType renderer(rasterizer.GetBase(), spanAllocator, spanGradient);
  
  rasterizer.Rasterize(renderer);
}

template <typename FuncType, typename ColorArrayType>
void GradientRasterizeAdapt(IGraphicsAGG::Rasterizer& rasterizer, EPatternExtend extend, const FuncType& gradientFunc, agg::trans_affine& xform, ColorArrayType& colorArray)
{
  // FIX extend none
  
  switch (extend)
  {
    case kExtendNone:
    case kExtendPad:
      GradientRasterize(rasterizer, gradientFunc, xform, colorArray);
      break;
    case kExtendReflect:
      GradientRasterize(rasterizer, agg::gradient_reflect_adaptor<FuncType>(gradientFunc), xform, colorArray);
      break;
    case kExtendRepeat:
      GradientRasterize(rasterizer, agg::gradient_repeat_adaptor<FuncType>(gradientFunc), xform, colorArray);
      break;
  }
}

void IGraphicsAGG::Rasterizer::RasterizePattern(agg::trans_affine transform, const IPattern& pattern, const IBlend* pBlend, EFillRule rule)
{
  mRasterizer.filling_rule(rule == kFillWinding ? agg::fill_non_zero : agg::fill_even_odd );
  
  switch (pattern.mType)
  {
    case kSolidPattern:
    {
      RendererSolid renderer(mRenBase);
      
      const IColor &color = pattern.GetStop(0).mColor;
      renderer.color(AGGColor(color, pBlend));
      
      // Rasterize
      
      Rasterize(renderer);
    }
      break;
      
    case kLinearPattern:
    case kRadialPattern:
    {
      // Common gradient objects
      
      const float* xform = pattern.mTransform;
      
      agg::trans_affine gradientMTX(xform[0], xform[1] , xform[2], xform[3], xform[4], xform[5]);
      ColorArrayType colorArray;
      
      // Scaling
      
      gradientMTX = transform * gradientMTX * agg::trans_affine_scaling(512.0);
      
      // Make gradient lut
      
      colorArray.remove_all();
      
      for (int i = 0; i < pattern.NStops(); i++)
      {
        const IColorStop& stop = pattern.GetStop(i);
        float offset = stop.mOffset;
        colorArray.add_color(offset, AGGColor(stop.mColor, pBlend));
      }
      
      colorArray.build_lut();
      
      // Rasterize
      
      if (pattern.mType == kLinearPattern)
      {
        GradientRasterizeAdapt(*this, pattern.mExtend, agg::gradient_x(), gradientMTX, colorArray);
      }
      else
      {
        GradientRasterizeAdapt(*this, pattern.mExtend, agg::gradient_radial_d(), gradientMTX, colorArray);
      }
    }
    break;
  }
}

#pragma mark -

IGraphicsAGG::IGraphicsAGG(IDelegate& dlg, int w, int h, int fps)
: IGraphicsPathBase(dlg, w, h, fps)
, mFontEngine()
, mFontManager(mFontEngine)
, mFontCurves(mFontManager.path_adaptor())
, mFontContour(mFontCurves)
{
}

IGraphicsAGG::~IGraphicsAGG()
{
}

void IGraphicsAGG::SetDisplayScale(int scale)
{
  mPixelMap.create(Width() * scale, Height() * scale);
  mRenBuf.attach(mPixelMap.buf(), mPixelMap.width(), mPixelMap.height(), mPixelMap.row_bytes());
  mPixf = PixfmtType(mRenBuf);
  mRasterizer.SetOutput(mPixf);

  IGraphics::SetDisplayScale(scale);
}

//IFontData IGraphicsAGG::LoadFont(const char* name, const int size)
//{
//  WDL_String cacheName(name);
//  char buf [6] = {0};
//  sprintf(buf, "-%dpt", size);
//  cacheName.Append(buf);
//
//  agg::font* font_buf = s_fontCache.Find(cacheName.Get());
//  if (!font_buf)
//  {
//    font_buf = (agg::font*) OSLoadFont(name, size);
//#ifndef NDEBUG
//    bool fontResourceFound = font_buf;
//#endif
//    assert(fontResourceFound); // Protect against typos in resource.h and .rc files.
//
//    s_fontCache.Add(font_buf, cacheName.Get());
//  }
//  return IFontData(font_buf);
//}

void IGraphicsAGG::DrawBitmap(IBitmap& bitmap, const IRECT& dest, int srcX, int srcY, const IBlend* pBlend)
{
  agg::pixel_map* pSource = (agg::pixel_map*) bitmap.GetRawBitmap();
  agg::rendering_buffer src(pSource->buf(), pSource->width(), pSource->height(), pSource->row_bytes());;
  PixfmtType imgPixfSrc(src);
  
  // FIX - blending (and also elsewhere) and speed for no rotation/scaling and integer coords...
  
  agg::trans_affine dstMtx(mTransform);
  dstMtx *= agg::trans_affine_scaling(GetDisplayScale());
  
  agg::trans_affine srcMtx;
  srcMtx /= dstMtx;
  srcMtx *= agg::trans_affine_translation(-dest.L, -dest.T);
  srcMtx *= agg::trans_affine_translation(srcX, srcY);
  srcMtx *= agg::trans_affine_scaling(bitmap.GetScale());
  imgSourceType imgSrc(imgPixfSrc);
  InterpolatorType interpolator(srcMtx);
  SpanAllocatorType spanAllocator;
  SpanGeneratorType spanGenerator(imgSrc, interpolator);
  BitmapRenderType renderer(mRasterizer.GetBase(), spanAllocator, spanGenerator);
  agg::rounded_rect rect(dest.L, dest.T, dest.R, dest.B, 0);
  agg::conv_transform<agg::rounded_rect> tr(rect, dstMtx);
  
  mRasterizer.SetPath(tr);
  mRasterizer.Rasterize(renderer);
  /*
  IRECT rect = dest;
  rect.Scale(GetDisplayScale());
  
  srcX *= GetDisplayScale();
  srcY *= GetDisplayScale();

  agg::pixel_map* pPixelMap = (agg::pixel_map*) bitmap.GetRawBitmap();
  agg::rendering_buffer buf(pPixelMap->buf(), pPixelMap->width(), pPixelMap->height(), pPixelMap->row_bytes());
  
  mPixf.comp_op(AGGBlendMode(pBlend));
  
  agg::rect_i r(srcX, srcY, srcX + rect.W(), srcY + rect.H());
  mRasterizer.GetBase().blend_from(PixfmtType(buf), &r, -srcX + rect.L, -srcY + rect.T, AGGCover(pBlend));
   */
}

void IGraphicsAGG::DrawRotatedMask(IBitmap& base, IBitmap& mask, IBitmap& top, int x, int y, double angle, const IBlend* pBlend)
{
  x *= GetDisplayScale();
  y *= GetDisplayScale();

  agg::pixel_map* pm_base = (agg::pixel_map*) base.GetRawBitmap();
  agg::pixel_map* pm_mask = (agg::pixel_map*) mask.GetRawBitmap();
  agg::pixel_map* pm_top = (agg::pixel_map*) top.GetRawBitmap();
  
  agg::rendering_buffer rbuf_base(pm_base->buf(), pm_base->width(), pm_base->height(), pm_base->row_bytes());
  agg::rendering_buffer rbuf_mask(pm_mask->buf(), pm_mask->width(), pm_mask->height(), pm_mask->row_bytes());
  agg::rendering_buffer rbuf_top(pm_top->buf(), pm_top->width(), pm_top->height(), pm_top->row_bytes());

  PixfmtType img_base(rbuf_base);
  PixfmtType img_mask(rbuf_mask);
  PixfmtType img_top(rbuf_top);

  RenbaseType ren_base(img_base);
  
  ren_base.clear(agg::rgba8(255, 255, 255, 0));
  
  ren_base.blend_from(img_mask, 0, 0, agg::cover_mask);
  ren_base.copy_from(img_top);
  
  const double width = base.W() * GetDisplayScale();
  const double height = base.H() * GetDisplayScale();
  
  agg::trans_affine srcMatrix;
  srcMatrix *= agg::trans_affine_translation(-(width / 2), -(height / 2));
  srcMatrix *= agg::trans_affine_rotation(angle);
  srcMatrix *= agg::trans_affine_translation(x + (width / 2), y + (height / 2));
  
  agg::trans_affine imgMtx = srcMatrix;
  imgMtx.invert();
  
  InterpolatorType interpolator(imgMtx);
  
  imgSourceType imgSrc(img_base);
  
  SpanGeneratorType spanGenerator(imgSrc, interpolator);
  SpanAllocatorType spanAllocator;
  BitmapRenderType renderer(mRasterizer.GetBase(), spanAllocator, spanGenerator);
  
  agg::rounded_rect rect(0, 0, width, height, 0);
  agg::conv_transform<agg::rounded_rect> tr(rect, srcMatrix);
  
  mRasterizer.SetPath(tr);
  mRasterizer.Rasterize(renderer);
}

void IGraphicsAGG::PathArc(float cx, float cy, float r, float aMin, float aMax)
{
  agg::arc arc(cx, cy, r, r, DegToRad(aMin), DegToRad(aMax));
  mPath.join_path(arc);
}

template<typename StrokeType>
void StrokeOptions(StrokeType& strokes, double thickness, const IStrokeOptions& options)
{
  // Set stroke options
  
  strokes.width(thickness);
  
  switch (options.mCapOption)
  {
    case kCapButt:   strokes.line_cap(agg::butt_cap);     break;
    case kCapRound:  strokes.line_cap(agg::round_cap);    break;
    case kCapSquare: strokes.line_cap(agg::square_cap);   break;
  }
  
  switch (options.mJoinOption)
  {
    case kJoinMiter:   strokes.line_join(agg::miter_join);   break;
    case kJoinRound:   strokes.line_join(agg::round_join);   break;
    case kJoinBevel:   strokes.line_join(agg::bevel_join);   break;
  }
  
  strokes.miter_limit(options.mMiterLimit);
}

void IGraphicsAGG::PathStroke(const IPattern& pattern, float thickness, const IStrokeOptions& options, const IBlend* pBlend)
{
  agg::trans_affine xform = mTransform;
  xform *= agg::trans_affine_scaling(GetDisplayScale());
  
  if (options.mDash.GetCount())
  {
    CurvedPathType curvedPath(mPath);
    DashType dashed(curvedPath);
    DashStrokeType strokes(dashed);
    TransformedDashStrokePathType path(strokes, xform);

    // Set the dashes (N.B. - for odd counts the array is read twice)

    int dashCount = options.mDash.GetCount();
    int dashMax = dashCount & 1 ? dashCount *2 : dashCount;
    const float* dashArray = options.mDash.GetArray();
    
    dashed.remove_all_dashes();
    dashed.dash_start(options.mDash.GetOffset());
    
    for (int i = 0; i < dashMax; i += 2)
        dashed.add_dash(dashArray[i % dashCount], dashArray[(i + 1) % dashCount]);
    
    StrokeOptions(strokes, thickness, options);
    mRasterizer.Rasterize(path, GetRasterTransform(), pattern, pBlend);
  }
  else
  {
    CurvedPathType curvedPath(mPath);
    StrokeType strokes(curvedPath);
    TransformedStrokePathType path(strokes, xform);
    
    StrokeOptions(strokes, thickness, options);
    mRasterizer.Rasterize(path, GetRasterTransform(), pattern, pBlend);
  }
  
  if (!options.mPreserve)
    PathClear();
}

void IGraphicsAGG::PathFill(const IPattern& pattern, const IFillOptions& options, const IBlend* pBlend)
{
  agg::trans_affine xform = mTransform;
  xform *= agg::trans_affine_scaling(GetDisplayScale());

  TransformedPathType path(mPath, xform);
  CurvedTransformedPathType curvedPath(path);

  mRasterizer.Rasterize(curvedPath, GetRasterTransform(), pattern, pBlend, options.mFillRule);
  if (!options.mPreserve)
    PathClear();
}

IColor IGraphicsAGG::GetPoint(int x, int y)
{
  agg::rgba8 point = mRasterizer.GetPixel(x, y);
  IColor color(point.a, point.r, point.g, point.b);
  return color;
}

/*
IBitmap IGraphicsAGG::CropBitmap(const IBitmap& srcbitmap, const IRECT& rect, const char* cacheName, int scale)
{
  agg::pixel_map* pPixelMap = (agg::pixel_map*) srcbitmap.mAPIBitmap->GetBitmap();
  agg::pixel_map* pCopy = (agg::pixel_map*) CreateAPIBitmap(rect.W(), rect.H());
  
  agg::rendering_buffer src;
  agg::rendering_buffer dest;
  
  src.attach(pPixelMap->buf(), pPixelMap->width(), pPixelMap->height(), pPixelMap->row_bytes());
  dest.attach(pCopy->buf(), pCopy->width(), pCopy->height(), pCopy->row_bytes());
  
  PixfmtType imgPixfSrc(src);
  PixfmtType imgPixfDest(dest);
  
  RenbaseType renbase(imgPixfDest);

  renbase.clear(agg::rgba(0, 0, 0, 0));
  
  agg::rect_i src_r(rect.L, rect.T, rect.R, rect.B);
  
  renbase.copy_from(imgPixfSrc, &src_r, -rect.L, -rect.T);
  
  AGGBitmap *pAPIBitmap = new AGGBitmap(pCopy, scale);
  
  return IBitmap(new AGGBitmap(pCopy, scale));  //TODO: surface will not be destroyed, unless this is retained
}
*/
//IBitmap IGraphicsAGG::CreateIBitmap(const char* cacheName, int w, int h)
//{
//  agg::pixel_map* pPixelMap = (agg::pixel_map*) CreateAPIBitmap(w, h);
//
//  s_bitmapCache.Add(pPixelMap, cacheName, mScale);
//  
//  IBitmap bitmap(pPixelMap, pPixelMap->width(), pPixelMap->height());
//  
//  return bitmap;
//}

agg::pixel_map* IGraphicsAGG::CreateAPIBitmap(int w, int h)
{
#ifdef OS_MAC
  agg::pixel_map_mac* pPixelMap = new agg::pixel_map_mac();
#else
  //TODO: win
#endif
  
  pPixelMap->create(w, h, 0);

  return pPixelMap;
}

APIBitmap* IGraphicsAGG::LoadAPIBitmap(const WDL_String& resourcePath, int scale)
{
  const char *path = resourcePath.Get();
#ifdef OS_MAC
  if (CSTR_NOT_EMPTY(path))
  {
    const char* ext = path+strlen(path)-1;
    while (ext >= path && *ext != '.') --ext;
    ++ext;
    
    bool ispng = !stricmp(ext, "png");
#ifndef IPLUG_JPEG_SUPPORT
    if (!ispng) return 0;
#else
    bool isjpg = !stricmp(ext, "jpg");
    if (!isjpg && !ispng) return 0;
#endif
    
    agg::pixel_map_mac* pPixelMap = new agg::pixel_map_mac();
    if (pPixelMap->load_img(path, ispng ? agg::pixel_map::format_png : agg::pixel_map::format_jpg))
      return new AGGBitmap(pPixelMap, scale);
    else
      delete pPixelMap;
  }
  
#else // OS_WIN
  //TODO: win
#endif
  
  return new APIBitmap();
}

APIBitmap* IGraphicsAGG::ScaleAPIBitmap(const APIBitmap* pBitmap, int scale)
{
  int destW = (pBitmap->GetWidth() / pBitmap->GetScale()) * scale;
  int destH = (pBitmap->GetHeight() / pBitmap->GetScale()) * scale;
    
  agg::pixel_map* pSource = (agg::pixel_map*) pBitmap->GetBitmap();
  agg::pixel_map* pCopy = (agg::pixel_map*) CreateAPIBitmap(destW, destH);
  agg::rendering_buffer src(pSource->buf(), pSource->width(), pSource->height(), pSource->row_bytes());;
  agg::rendering_buffer dest(pCopy->buf(), pCopy->width(), pCopy->height(), pCopy->row_bytes());
  PixfmtType imgPixfSrc(src);
  PixfmtType imgPixfDest(dest);
  
  RenbaseType renBase(imgPixfDest);
  renBase.clear(agg::rgba(0, 0, 0, 0));
  
  agg::rasterizer_scanline_aa<> rasterizer;
  agg::trans_affine_scaling srcMtx(pBitmap->GetScale() / (double) scale);
  InterpolatorType interpolator(srcMtx);
  imgSourceType imgSrc(imgPixfSrc);
  SpanAllocatorType spanAllocator;
  agg::scanline_u8 scanline;
  SpanGeneratorType spanGenerator(imgSrc, interpolator);
  BitmapRenderType renderer(renBase, spanAllocator, spanGenerator);
  
  agg::rounded_rect rect(0, 0, destW, destH, 0);
  rasterizer.add_path(rect);
  agg::render_scanlines(rasterizer, scanline, renderer);
  
  return new AGGBitmap(pCopy, scale);
}

void IGraphicsAGG::RenderDrawBitmap()
{
#ifdef OS_MAC
  mPixelMap.draw((CGContext*) GetPlatformContext(), GetDisplayScale());
#else // OS_WIN
  //TODO: win
#endif
}

void IGraphicsAGG::CalculateTextLines(WDL_TypedBuf<LineInfo>* pLines, const IRECT& rect, const char* str, FontManagerType& manager)
{
  LineInfo info;
  info.mStartChar = 0;
  info.mEndChar = (int) strlen(str);
  pLines->Add(info);
  
  LineInfo* pLine = pLines->Get();
  
  size_t lineStart = 0;
  size_t lineWidth = 0;
  size_t linePos = 0;
  double xCount = 0.0;
  
  const char* cstr = str;
  
  while (*cstr)
  {
    const agg::glyph_cache* pGlyph = manager.glyph(*cstr);
    
    if (pGlyph)
    {
      xCount += pGlyph->advance_x;
    }

    cstr++;
    linePos++;
    
    if (*cstr == ' ' || *cstr == 0)
    {
      pLine->mStartChar = (int) lineStart;
      pLine->mEndChar = (int)  linePos;
      pLine->mWidth = xCount;
    }
    
    if (rect.W() > 0 && xCount >= rect.W())
    {
      assert(pLine);
      
      cstr = &str[pLine->mEndChar];
      lineStart = pLine->mEndChar + 1;
      linePos = pLine->mEndChar;
      
      LineInfo info;
      pLines->Add(info);
      pLine++;
      
      assert(pLines);
      
      xCount = 0;
      lineWidth = 0;
    }
    
  }
}

bool IGraphicsAGG::DrawText(const IText& text, const char* str, IRECT& destRect, bool measure)
{
//  if (!str || str[0] == '\0')
//  {
//    return true;
//  }
//
//  IRECT rect = destRect;
//  rect.Scale(GetDisplayScale());
//
//  RendererSolid renSolid(mRenBase);
//  RendererBin renBin(mRenBase);
//
//  agg::scanline_u8 sl;
//  agg::rasterizer_scanline_aa<> ras;
//
//  agg::glyph_rendering gren = agg::glyph_ren_agg_gray8;
//  //agg::glyph_rendering gren = agg::glyph_ren_outline;
//  //agg::glyph_rendering gren = agg::glyph_ren_agg_mono;
//  //agg::glyph_rendering gren = agg::glyph_ren_native_gray8;
//  //agg::glyph_rendering gren = agg::glyph_ren_native_mono;
//
//  float weight = 0.0;
//  bool kerning = false;
//  bool hinting = false;
//
//  if (gren == agg::glyph_ren_agg_mono)
//  {
//    mFontEngine.gamma(agg::gamma_threshold(0.5));
//  }
//  else
//  {
//    mFontEngine.gamma(agg::gamma_power(1.0));
//  }
//
//  if (gren == agg::glyph_ren_outline)
//  {
//    //for outline cache set gamma for the rasterizer
//    ras.gamma(agg::gamma_power(1.0));
//  }
//
//  mFontContour.width(-weight * (text.mSize * 0.05) * GetDisplayScale());
//
//  IFontData font = LoadFont(text.mFont, text.mSize);
//  agg::font* pFontData = (agg::font *)font.mData;
//
//  if (pFontData != 0 && mFontEngine.load_font("", 0, gren, pFontData->buf(), pFontData->size()))
//  {
//    mFontEngine.hinting(hinting);
//    mFontEngine.height(text.mSize * GetDisplayScale());
//    mFontEngine.width(text.mSize * GetDisplayScale());
//    mFontEngine.flip_y(true);
//
//    double x = rect.L;
//    double y = rect.T + (text.mSize * GetDisplayScale());
//
//    WDL_TypedBuf<LineInfo> lines;
//
//    CalculateTextLines(&lines, rect, str, mFontManager);
//
//    LineInfo * pLines = lines.Get();
//
//    for (int i=0; i<lines.GetSize(); ++i, ++pLines)
//    {
//      switch (text.mAlign)
//      {
//        case IText::kAlignNear:
//          x = rect.L;
//          break;
//        case IText::kAlignCenter:
//          x = rect.L + ((rect.W() - pLines->mWidth) / 2);
//          break;
//        case IText::kAlignFar:
//          x = rect.L + (rect.W() - pLines->mWidth);
//          break;
//      }
//
//      for (size_t c=pLines->mStartChar; c<pLines->mEndChar; c++)
//      {
//        const agg::glyph_cache* pGlyph = mFontManager.glyph(str[c]);
//
//        if (pGlyph)
//        {
//          if (kerning)
//          {
//            mFontManager.add_kerning(&x, &y);
//          }
//
//          mFontManager.init_embedded_adaptors(pGlyph, x, y);
//
//          switch (pGlyph->data_type)
//          {
//            case agg::glyph_data_mono:
//
//              renBin.color(IColorToAggColor(text.mColor));
//              agg::render_scanlines(mFontManager.mono_adaptor(),
//                                    mFontManager.mono_scanline(),
//                                    renBin);
//              break;
//
//            case agg::glyph_data_gray8:
//
//              renSolid.color(IColorToAggColor(text.mColor));
//              agg::render_scanlines(mFontManager.gray8_adaptor(),
//                                    mFontManager.gray8_scanline(),
//                                    renSolid);
//              break;
//
//            case agg::glyph_data_outline:
//
//              ras.reset();
//
//              if (fabs(weight) <= 0.01)
//              {
//                //for the sake of efficiency skip the
//                //contour converter if the weight is about zero.
//                ras.add_path(mFontCurves);
//              }
//              else
//              {
//                ras.add_path(mFontContour);
//              }
//
//              renSolid.color(IColorToAggColor(text.mColor));
//              agg::render_scanlines(ras, sl, renSolid);
//
//              break;
//
//            default: break;
//          }
//
//          //increment pen position
//          x += pGlyph->advance_x;
//          y += pGlyph->advance_y;
//        }
//      }
//      y += text.mSize * GetDisplayScale();
//    }
//  }
  return false;
}

bool IGraphicsAGG::MeasureText(const IText& text, const char* str, IRECT& destRect)
{
//  if (!str || str[0] == '\0')
//  {
//    destRect.Clear();
//    return true;
//  }

//  renderer_solid ren_solid(mRenBase);
//  renderer_bin ren_bin(mRenBase);
//
//  agg::scanline_u8 sl;
//  agg::rasterizer_scanline_aa<> ras;
//
//  agg::glyph_rendering gren = agg::glyph_ren_agg_gray8;
//  //agg::glyph_rendering gren = agg::glyph_ren_outline;
//  //agg::glyph_rendering gren = agg::glyph_ren_agg_mono;
//  //agg::glyph_rendering gren = agg::glyph_ren_native_gray8;
//  //agg::glyph_rendering gren = agg::glyph_ren_native_mono;
//
//  float weight = 0.0;
//  bool hinting = false;
//
//  if (gren == agg::glyph_ren_agg_mono)
//  {
//    mFontEngine.gamma(agg::gamma_threshold(0.5));
//  }
//  else
//  {
//    mFontEngine.gamma(agg::gamma_power(1.0));
//  }
//
//  if (gren == agg::glyph_ren_outline)
//  {
//    //for outline cache set gamma for the rasterizer
//    ras.gamma(agg::gamma_power(1.0));
//  }
//
//  mFontContour.width(-weight * (text.mSize * 0.05) * GetDisplayScale());
//
//  IFontData font = LoadFont(text.mFont, text.mSize);
//  agg::font * pFontData = (agg::font *)font.mData;
//
//  if (mFontEngine.load_font("", 0, gren, pFontData->buf(), pFontData->size()))
//  {
//    mFontEngine.hinting(hinting);
//    mFontEngine.height(text.mSize * GetDisplayScale());
//    mFontEngine.width(text.mSize * GetDisplayScale());
//    mFontEngine.flip_y(true);
//
//    WDL_TypedBuf<LineInfo> lines;
//
//    CalculateTextLines(&lines, destRect, str, mFontManager);
//
//    LineInfo * pLines = lines.Get();
//
//    int max_width = 0;
//    int height = 0;
//
//    for (int i=0; i<lines.GetSize(); ++i, ++pLines)
//    {
//      if (pLines->width > max_width)
//      {
//        max_width = pLines->width;
//      }
//      height += text.mSize * GetDisplayScale();
//    }
//
//    destRect.L = 0; destRect.T = 0;
//    destRect.R = max_width; destRect.B = height;
//
//    return true;
//  }
//
  return false;
}
/*
agg::pixel_map* IGraphicsAGG::load_image(const char* filename)
{
  IBitmap bitmap = LoadBitmap(filename, 1, 1.0);
  return (agg::pixel_map*) bitmap.mData;
}

*/

void IGraphicsAGG::Draw(const IRECT& rect)
{
  mRasterizer.ClearWhite();
  IGraphics::Draw(rect);
}

#include "IGraphicsAGG_src.cpp"
