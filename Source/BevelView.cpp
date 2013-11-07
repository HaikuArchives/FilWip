/*
 * FilWip
 * Copyright (c) 2001 Ramshankar
 *
 * This license does not apply to source code written by third parties included with
 * and/or used by this software. This includes third party modules or any other form
 * of source code provided by third parties or by an unknown source.
 *
 * You are allowed to  use parts of  this code in  your own program(s) provided  the
 * program  is  freeware  and  you  give  credit to  the  author in  your  program's
 * interactive mode (ie viewable by the end-user). You are not allowed to distribute
 * the code in any other language other than the language it was  originally written
 * in. Any alterations made  to the original  code must be  mentioned explicitly and
 * the author must be explicitly disclaimed of any responsibility.
 *
 * If you wish to  use the source code  or parts of  it thereof  in a  commercial or
 * shareware  application, the author  receives a free  version of  your program and
 * contact the author to negotiate  a fee for your  use of the author's source code.
 * Distributing  modified  versions  of  the source  code  is  allowed provided  you
 * explicitly disclaim the author and claim that the distribution is not an original
 * distribution from the author.
 * 
 * Original code by:
 * Ramshankar
 *
 */

#include "BevelView.h"
#include "Constants.h"

/*============================================================================================================*/

BevelView::BevelView (BRect frame, const char *name, BevelType bevelMode, uint32 resizeMask, uint32 flags)
	: BView (frame, name, resizeMask, flags | B_FRAME_EVENTS)
{
	/* Set up colors, edges and cache the Bounds() rectangle */
	bevelType = bevelMode;
	rgb_color backColor;
	
	if (Parent())
		backColor = ViewColor();
	else
		backColor = BeViewColor;
	
	switch (bevelType)
	{
		case btDeep: case btInset:
			darkEdge1 = tint_color (backColor, B_DARKEN_2_TINT);
			darkEdge2 = tint_color (backColor, B_DARKEN_3_TINT);
			lightEdge = BePureWhite;
			edgeThickness = bevelType == btInset ? btInsetThickness : btDeepThickness;
			break;

		case btOutset:
			darkEdge1 = BePureWhite;
			darkEdge2 = tint_color (backColor, B_DARKEN_3_TINT);
			lightEdge = tint_color (backColor, B_DARKEN_2_TINT);
			edgeThickness = btOutsetThickness;
			break;
		
		case btBulge:
			lightEdge = tint_color (backColor, B_DARKEN_3_TINT);
			darkEdge2 = tint_color (backColor, B_DARKEN_2_TINT);
			darkEdge1 = tint_color (backColor, B_LIGHTEN_1_TINT);
			edgeThickness = btBulgeThickness;
			break;
		
		case btNoBevel:
			break;
	}
	
	cachedRect = Bounds();
}

/*============================================================================================================*/

void BevelView::Draw (BRect updateRect)
{
	/* Draw the edges based on the type of edge specified */
	switch (bevelType)
	{
		case btNoBevel:
			break;
		
		case btDeep: case btBulge:
		{
			SetHighColor (darkEdge2);
			StrokeRect (BRect (cachedRect.left + 1, cachedRect.top + 1, cachedRect.right - 1,
							cachedRect.bottom - 1));
			
			BeginLineArray (4L);
			AddLine (cachedRect.LeftTop(), cachedRect.RightTop(), darkEdge1);
			AddLine (cachedRect.LeftTop(), cachedRect.LeftBottom(), darkEdge1);
			
			AddLine (cachedRect.RightTop(), cachedRect.RightBottom(), lightEdge);
			AddLine (cachedRect.RightBottom(), cachedRect.LeftBottom(), lightEdge);
			EndLineArray ();
			break;
		}
		
		case btInset: case btOutset:
		{
			rgb_color c = lightEdge;
			c.red += 30; c.green += 30; c.blue += 30;
			SetHighColor (bevelType == btInset ? lightEdge : c);
			StrokeRect (Bounds());
			
			SetHighColor (darkEdge1);
			StrokeLine (cachedRect.LeftTop(), cachedRect.RightTop());
			StrokeLine (cachedRect.LeftTop(), cachedRect.LeftBottom());
			break;
		}		
	}
	
	BView::Draw (updateRect);
}

/*============================================================================================================*/

void BevelView::FrameResized (float newWidth, float newHeight)
{
	/* Cached drawing. Draw only when the "extra" area */
	BRect newRect (Bounds());
	float minX, maxX, minY, maxY;
	
	
	/* Capture the new co-ords of the "extra" rect */
	minX = newRect.right > cachedRect.right ? cachedRect.right : newRect.right;
	maxX = newRect.right < cachedRect.right ? cachedRect.right : newRect.right;
	minY = newRect.bottom > cachedRect.bottom ? cachedRect.bottom : newRect.bottom;
	maxY = newRect.bottom < cachedRect.bottom ? cachedRect.bottom : newRect.bottom;

	
	/* Draw if the rectangle is really valid */
	cachedRect = newRect;
	if (minX != maxX)
		Invalidate (BRect (minX - 1, newRect.top, maxX, maxY));

	if (minY != maxY)
		Invalidate (BRect (newRect.left, minY - 1, maxX, maxY));
	
	BView::FrameResized (newWidth, newHeight);
}

/*============================================================================================================*/

float BevelView::EdgeThickness() const
{
	return edgeThickness;
}

/*============================================================================================================*/

float BevelView::EdgeThickness (BevelType type)
{
	switch (type)
	{
		case btInset: return btInsetThickness;
		case btOutset: return btOutsetThickness;
		case btDeep: return btDeepThickness;
		case btBulge: return btBulgeThickness;
		default: return 0;
	}
}

/*============================================================================================================*/
