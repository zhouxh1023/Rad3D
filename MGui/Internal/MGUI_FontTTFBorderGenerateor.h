/*
*	FontTTF Border Generator
*
*	Author: SiZhong.Wang, M-001
*
*	CopyRight: SilverEyes Information CO. LTD.
*/
#pragma once

#include "MGUI_Font.h"

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftbitmap.h>
#include <freetype/ftstroke.h>

#pragma warning (push)
#pragma warning (disable : 4244)

namespace Rad { namespace MGUI {

	class TTFBorderGenerator
	{
		struct RECT
		{
			RECT() { }
			RECT(int left, int top, int right, int bottom)
				: xmin(left), xmax(right), ymin(top), ymax(bottom) { }

			void Include(int x, int y)
			{
				xmin = Min(xmin, x);
				ymin = Min(ymin, y);
				xmax = Max(xmax, x);
				ymax = Max(ymax, y);
			}

			int Width() const { return xmax - xmin + 1; }
			int Height() const { return ymax - ymin + 1; }

			int xmin, xmax, ymin, ymax;
		};


		// A horizontal pixel span generated by the FreeType renderer.

		struct Span
		{
			Span() { }
			Span(int _x, int _y, int _width, int _coverage)
				: x(_x), y(_y), width(_width), coverage(_coverage) { }

			int x, y, width, coverage;
		};

		typedef Array<Span> Spans;


		// Each time the renderer calls us back we just push another span entry on
		// our list.

		static void
			RasterCallback(const int y,
			const int count,
			const FT_Span * const spans,
			void * const user) 
		{
			Spans *sptr = (Spans *)user;
			for (int i = 0; i < count; ++i) 
				sptr->PushBack(Span(spans[i].x, y, spans[i].len, spans[i].coverage));
		}


		// Set up the raster parameters and render the outline.

		static void
			RenderSpans(FT_Library &library,
			FT_Outline * const outline,
			Spans *spans) 
		{
			FT_Raster_Params params;
			memset(&params, 0, sizeof(params));
			params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
			params.gray_spans = RasterCallback;
			params.user = spans;

			FT_Outline_Render(library, outline, &params);
		}


	public:
		int _width, _height;
		Array<PixelLA> _bitmap;
		int _advance;
		int _bearingX;
		int _bearingY;

		Spans spans, outlineSpans;

		bool LoadGlyph(uchar_t ch, int border, FT_Library library, FT_Face face)
		{
			if (FT_Load_Char(face, ch, FT_LOAD_NO_BITMAP) == 0)
			{
				if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
				{
					spans.Clear(), outlineSpans.Clear();

					RenderSpans(library, &face->glyph->outline, &spans);

					FT_Glyph glyph;
					if (FT_Get_Glyph(face->glyph, &glyph) == 0)
					{
						FT_Stroker stroker;
						FT_Stroker_New(library, &stroker);
						FT_Stroker_Set(stroker, border << 6, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

						FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);

						if (glyph->format == FT_GLYPH_FORMAT_OUTLINE)
						{
							// Render the outline spans to the span list
							FT_Outline *o = &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline;
							RenderSpans(library, o, &outlineSpans);
						}

						// Clean up afterwards.
						FT_Stroker_Done(stroker);
						FT_Done_Glyph(glyph);

						// Now we need to put it all together.
						if (!spans.Empty())
						{
							// Figure out what the bounding rect is for both the span lists.
							RECT rect(spans[0].x, spans[0].y, spans[0].x, spans[0].y);
							for (int i = 0; i < spans.Size(); ++i)
							{
								Span * s = &spans[i];
								rect.Include(s->x, s->y);
								rect.Include(s->x + s->width - 1, s->y);
							}

							for (int i = 0; i < outlineSpans.Size(); ++i)
							{
								Span * s = &outlineSpans[i];
								rect.Include(s->x, s->y);
								rect.Include(s->x + s->width - 1, s->y);
							}

							_bearingX = face->glyph->metrics.horiBearingX >> 6;
							_bearingY = face->glyph->metrics.horiBearingY >> 6;
							_advance = face->glyph->advance.x >> 6;

							int imgWidth = rect.Width(),
								imgHeight = rect.Height(),
								imgSize = imgWidth * imgHeight;

							// Allocate data for our image and clear it out to transparent.
							_bitmap.Resize(imgSize);
							_width = imgWidth;
							_height = imgHeight;

							PixelLA *pxl = &_bitmap[0];
							memset(pxl, 0, sizeof(PixelLA) * imgSize);

							// Loop over the outline spans and just draw them into the
							// image.
							for (int i = 0; i < outlineSpans.Size(); ++i)
							{
								Span * s = &outlineSpans[i];

								for (int w = 0; w < s->width; ++w)
								{
									pxl[(int)((imgHeight - 1 - (s->y - rect.ymin)) * imgWidth + s->x - rect.xmin + w)] = PixelLA(0, s->coverage);
								}
							}

							// Then loop over the regular glyph spans and blend them into
							// the image.
							int nRow = 0, nLine = 0;
							for (int i = 0; i < spans.Size(); ++i)
							{
								Span * s = &spans[i];

								for (int w = 0; w < s->width; ++w)
								{
									nRow = imgHeight - 1 - (s->y - rect.ymin);
									nLine = s->x - rect.xmin + w;

									PixelLA &dst = pxl[nRow * imgWidth + nLine];
									PixelLA src = PixelLA(255, s->coverage);
									dst.l = (int)(dst.l + ((src.l - dst.l) * src.a) / 255.0f);
									dst.a = Min(255, (int)(dst.a + src.a));
								}
							}

							return true;
						}
					}
				}
			}

			return false;
		}
	};

}}

#pragma warning (pop)