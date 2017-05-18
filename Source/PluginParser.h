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
 * Modified by:
 * :Puck Meerburg
 */

#ifndef _PLUGIN_PARSER_H
#define _PLUGIN_PARSER_H

#include <Entry.h>
#include <String.h>
#include <List.h>

#include <fstream>
#include <string.h>

#define PLUGIN_NAME_DEF					"name"
#define PLUGIN_TYPE_DEF					"type"
#define PLUGIN_TYPE_DEF_LEN				4
#define PLUGIN_TYPE_LINEAR				"linear"
#define PLUGIN_TYPE_HIERARCHIAL			"hierarchical"
#define PLUGIN_CHECKINSTALL_DEF			"check install of"
#define PLUGIN_CHECKINSTALL_DEF_LEN		16
#define PLUGIN_ITEM_FOLDER_DEF			"folder"
#define PLUGIN_ITEM_FOLDER_DEF_LEN		6
#define PLUGIN_ITEM_FILE_DEF			"file"
#define PLUGIN_ITEM_FILE_DEF_LEN		4
#define PLUGIN_ITEM_EXCLUDE_FILE		"exclude file"
#define PLUGIN_ITEM_EXCLUDE_FILE_LEN	12
#define PLUGIN_ITEM_INCLUDE_MIME		"include mime"
#define PLUGIN_ITEM_INCLUDE_MIME_LEN	12
#define PLUGIN_ITEM_RECURSE				"recurse"
#define PLUGIN_ITEM_RECURSE_LEN			7
#define PLUGIN_ITEM_RECURSE_YES			"yes"

/*============================================================================================================*/

class PluginContainerItem
{
	public:
		PluginContainerItem ();
		~PluginContainerItem ();
		
		bool		isLinear;
		BList		subItems;
		BString		checkPath,
					name;
};

/*============================================================================================================*/

class PluginItem
{
	public:
		PluginItem ();
		~PluginItem ();
		
		/* Data members */
		BString	itemName;
		bool	isFolder,
				recurse;
		BString	itemPath,
				excludeFilePath,
				includeMimeType;
};

/*============================================================================================================*/

class PluginParser
{
	public:
		PluginParser (const char *pluginFilePath);
		~PluginParser ();

		/* Hook functions */		
		void			FillContainerItem (PluginContainerItem *item);
		BList			Items ();
		bool			DoesFileExist () const;
		
	private:
		/* Private variables */
		BEntry			pluginEntry;
		bool			fileExists;
		std::fstream	pluginStream;
		BList			itemList;
};

/*============================================================================================================*/

#endif /* _PLUGIN_PARSER_H */
