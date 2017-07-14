/*
	Bubblehelp class Copyright  (c) 1998 Marco Nelissen <marcone@xs4all.nl>
	Freely usable in non-commercial applications, as long as proper credit
	is given.
	
	FilWip (c) 2002 Ramshankar
	Many thanks to Marco Nelissen for providing the Be community with this
	easy to use tooltip class. Thanks Marco!
	
	* NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE *
	
		This file was generously distributed free of cost by a third party developer
		and does not fall under this program's license. The author of this program
		wishes to thank the thirdy party developer (Marco Nelissen) for his efforts

	* NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE * NOTICE *
*/

#ifndef _BUBBLEHELPER_H
	#define _BUBBLEHELPER_H

#include <OS.h>

class BubbleHelper
{
	public:
		BubbleHelper ();
		virtual ~BubbleHelper ();
		
		/* Additional hooks */
		void			SetHelp (BView *view, char *text);
		void			EnableHelp (bool enable = true);

		void			SetDelayTime (bigtime_t useconds);
		bigtime_t		DelayTime ();
		
	private:
		/* Private hooks */
		void			HideBubble ();
		void			ShowBubble (BPoint dest);
		void			DisplayHelp (char *text,BPoint where);
		void			Helper();
		char			*GetHelp (BView *view);
		BView			*FindView (BPoint where);
		
		/* Thread function */
		static int32		_helper (void *arg);

		/* Misc. private variables */
		bigtime_t		delayTime;
		thread_id		helperthread;
		BList			*helplist;
		BWindow			*textwin;
		BTextView		*textview;
		bool			enabled;
		
		static int32		runcount;
};

#endif /* _BUBBLEHELPER_H */
