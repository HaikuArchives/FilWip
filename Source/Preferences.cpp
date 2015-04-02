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

#include "Preferences.h"
#include "FilWip.h"
#include "Constants.h"
#include "Directory.h"

Preferences prefs;

/*============================================================================================================*/

Preferences::Preferences ()
{
	/* Initialize "prefsPath" */
	if (find_directory (B_USER_SETTINGS_DIRECTORY, &prefsPath) != B_OK)
		return;

	prefsPath.Append("FilWip");
	BDirectory prefsDir(prefsPath.Path());

	if (!prefsDir.Contains(prefsPath.Path()))
		prefsDir.CreateDirectory(prefsPath.Path(), NULL);

	prefsPath.SetTo (prefsPath.Path(), PREFS_FILENAME);

	ReadSettings ();
}

/*============================================================================================================*/

Preferences::~Preferences ()
{
	/* Write settings to the file before dying */
	WriteSettings();
}

/*============================================================================================================*/

void Preferences::WriteSettings ()
{
	/* Serialize contents of "prefs" to a file on disk */
	RemoveName ("presetVerifyCode");
	AddFloat ("presetVerifyCode", PresetVerifyCode);

	BFile file (prefsPath.Path(), B_CREATE_FILE | B_WRITE_ONLY);
	Flatten (&file);
}

/*============================================================================================================*/

void Preferences::ReadSettings ()
{
	/* Load "prefs" from the preferences file */
	BFile file (prefsPath.Path(), B_READ_ONLY);
	Unflatten (&file);
}

/*============================================================================================================*/

const char* Preferences::PrefFilePath ()
{
	return prefsPath.Path();
}

/*============================================================================================================*/

status_t Preferences::SetBool (const char *name, bool b)
{
	if (HasBool (name) == true)
		return ReplaceBool (name, 0, b);

	return AddBool(name, b);
}

/*============================================================================================================*/

status_t Preferences::SetInt8 (const char *name, int8 i)
{
	if (HasInt8 (name) == true)
		return ReplaceInt8 (name, 0, i);

	return AddInt8 (name, i);
}

/*============================================================================================================*/

status_t Preferences::SetInt16 (const char *name, int16 i)
{
	if (HasInt16 (name) == true)
		return ReplaceInt16 (name, 0, i);

	return AddInt16 (name, i);
}

/*============================================================================================================*/

status_t Preferences::SetInt32 (const char *name, int32 i)
{
	if (HasInt32 (name) == true)
		return ReplaceInt32 (name, 0, i);

	return AddInt32 (name, i);
}

/*============================================================================================================*/

status_t Preferences::SetInt64 (const char *name, int64 i)
{
	if (HasInt64 (name) == true)
		return ReplaceInt64 (name, 0, i);

	return AddInt64 (name, i);
}

/*============================================================================================================*/

status_t Preferences::SetFloat (const char *name, float f)
{
	if (HasFloat (name) == true)
		return ReplaceFloat (name, 0, f);

	return AddFloat(name, f);
}

/*============================================================================================================*/

status_t Preferences::SetDouble (const char *name, double f)
{
	if (HasDouble (name) == true)
		return ReplaceDouble (name, 0, f);

	return AddDouble (name, f);
}

/*============================================================================================================*/

status_t Preferences::SetString (const char *name, const char *s)
{
	if (HasString (name) == true)
		return ReplaceString (name, 0, s);
	
	return AddString (name, s);
}

/*============================================================================================================*/

status_t Preferences::SetPoint (const char *name, BPoint p)
{
	if (HasPoint (name) == true)
		return ReplacePoint (name, 0, p);

	return AddPoint (name, p);
}

/*============================================================================================================*/

status_t Preferences::SetRect (const char *name, BRect r)
{
	if (HasRect (name) == true)
		return ReplaceRect (name, 0, r);

	return AddRect (name, r);
}

/*============================================================================================================*/

status_t Preferences::SetMessage (const char *name, const BMessage *message)
{
	if (HasMessage (name) == true)
		return ReplaceMessage (name, 0, message);

	return AddMessage (name, message);
}

/*============================================================================================================*/

status_t Preferences::SetFlat (const char *name, const BFlattenable *obj)
{
	if (HasFlat (name, obj) == true)
		return ReplaceFlat (name, 0, (BFlattenable*)obj);

	return AddFlat (name, (BFlattenable*) obj);
}

/*============================================================================================================*/

bool Preferences::FindBoolDef (const char *name, bool def)
{
	bool v;
	status_t result = FindBool (name, &v);
	if (result != B_OK)
		return def;
	else
		return v;
}

/*============================================================================================================*/
