/*
 *	Beezer
 *	Copyright (c) 2002 Ramshankar (aka Somebody)
 *
 *	See "License.txt" for licensing info.
*/

#ifndef _BEVEL_VIEW_H
#define _BEVEL_VIEW_H

#include <View.h>

enum BevelThickness
{
	btInsetThickness = 1,
	btOutsetThickness = 1,
	btBulgeThickness = 2,
	btDeepThickness = 2,
	btNoBevelThickness = 0
};

enum BevelType
{
	btInset,
	btDeep,
	btOutset,
	btBulge,
	btNoBevel
};

class BevelView : public BView
{
	public:
		BevelView (BRect frame, const char *name, BevelType bevelMode, uint32 resizeMask, uint32 flags);
		
		/* Inherited Hooks */
		virtual void		Draw (BRect updateRect);
		virtual void		FrameResized (float newWidth, float newHeight);
	
		/* Additional Hooks */
		float				EdgeThickness () const;
		static float		EdgeThickness (BevelType type);

	protected:
		/* Protected Members */
		BRect				cachedRect;
		BevelType			bevelType;
		rgb_color			darkEdge1,
							darkEdge2,
							lightEdge;
		float				edgeThickness;
};

#endif /* _BEVEL_VIEW_H */
