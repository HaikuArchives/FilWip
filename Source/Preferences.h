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

#ifndef _PREFERENCES_H
#define _PREFERENCES_H

#include <Path.h>
#include <FindDirectory.h>
#include <File.h>

#define PREFS_FILENAME "FilWip_settings"

class Preferences : public BMessage
{
	public:
		Preferences ();
		~Preferences ();
	
		/* Additional Hooks */
		void				WriteSettings ();
		void				ReadSettings ();
		void				InitSettings ();
		const char			*PrefFilePath();

		status_t			SetBool (const char *name, bool b);
		status_t			SetInt8 (const char *name, int8 i);
		status_t			SetInt16 (const char *name, int16 i);
		status_t			SetInt32 (const char *name, int32 i);
		status_t			SetInt64 (const char *name, int64 i);
		status_t			SetFloat (const char *name, float f);
		status_t			SetDouble (const char *name, double d);
		status_t			SetString (const char *name, const char *string);
		status_t			SetPoint (const char *name, BPoint p);
		status_t			SetRect (const char *name, BRect r);
		status_t			SetMessage (const char *name, const BMessage *message);
		status_t			SetFlat (const char *name, const BFlattenable *obj);
		
		bool				FindBoolDef (const char *name, bool defaultValue);
		
	private:
		BPath				prefsPath;
};

extern Preferences prefs;

#endif /* _PREFERENCES_H */
