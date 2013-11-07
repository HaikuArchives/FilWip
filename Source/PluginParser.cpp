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

/*
 *	TODO:
 *	1. Improve the tolerance of the parser (be more flexible in the format)
 *	2. Implement better error checking and handling
 *
 */

#include <Debug.h> //temp

#include "PluginParser.h"

/*============================================================================================================*/

PluginContainerItem::PluginContainerItem ()
{
	/* This class is basically used as a struct with an added advantage of freeing memory while it gets
		destroyed. This is a container class for holding all the sub items for a plugin.
		Eg: NetPositive is the container, cookies, cache, history are PluginItems stored in "subItems" */
	isLinear = false;
}

/*============================================================================================================*/

PluginContainerItem::~PluginContainerItem ()
{
	/* Free all our PluginItems in the subItems BList */
	PluginItem *sItem (NULL);
	for (;;)
	{
		sItem = (PluginItem*)subItems.RemoveItem (0L);
		if (sItem == NULL)
			break;
		
		delete sItem;
	}
}

/*============================================================================================================*/
/*============================================================================================================*/
/*============================================================================================================*/

PluginItem::PluginItem ()
{
	/* This class is also used as a struct and can be made a struct too, at first char* was used so
		it was made into a class to auto-free the char*, now BString is used which takes care of freeing...
		This class stores the locations, name etc for each item: Eg- cookies, <path>, <check> etc. */
	isFolder = false;
	recurse = false;
}

/*============================================================================================================*/

PluginItem::~PluginItem()
{
}

/*============================================================================================================*/
/*============================================================================================================*/
/*============================================================================================================*/

PluginParser::PluginParser (const char *pluginFilePath)
{
	fileExists = true;
	
	pluginEntry.SetTo (pluginFilePath, true);
	fileExists = pluginEntry.Exists();
	pluginStream.open (pluginFilePath, ios::in);
}

/*============================================================================================================*/

PluginParser::~PluginParser ()
{
}

/*============================================================================================================*/

bool PluginParser::DoesFileExist () const
{
	/* Return whether the file exists to be parsed or not */
	return fileExists;
}

/*============================================================================================================*/

BList PluginParser::Items ()
{
	PRINT (("PluginParser::Items ()\n"));

	/* Iterative parse routine for parsing items with their paths and exclusions/inclusions definitions */
	itemList.MakeEmpty();
	char temp[300];
	char temp2[300];
	char temp3[300];
	char temp4[300];

	while (!pluginStream.eof())
	{
		pluginStream.getline (temp, 299, '\n');
		
		/* Check empty line, if so skip it */
		if (temp[0] == '\0')
			continue;
		
		pluginStream.getline (temp2, 299, '\n');
		pluginStream.getline (temp3, 299, '\n');
		pluginStream.getline (temp4, 299, '\n');

		BString firstLine = temp;
		BString propertyName = temp2;
		BString lastLine = temp3;
		BString recurseLine = temp4;
		PluginItem *pItem = new PluginItem();
		
		/* Check for the name of the item [Cache] eg */
		if (firstLine.ByteAt (0) == '[')
		{
			firstLine.Remove (0, 1);
			firstLine.RemoveLast ("]");
			
			pItem->itemName = firstLine;
		}
	
		/* Parse fo the property=value line, FOLDER=/boot/config... eg */
		if (propertyName.ICompare (PLUGIN_ITEM_FOLDER_DEF, PLUGIN_ITEM_FOLDER_DEF_LEN) == 0)
		{
			pItem->isFolder = true;
			propertyName.Remove (0, PLUGIN_ITEM_FOLDER_DEF_LEN + 1);
			pItem->itemPath = propertyName;
		}
		else if (propertyName.ICompare (PLUGIN_ITEM_FILE_DEF, PLUGIN_ITEM_FILE_DEF_LEN) == 0)
		{
			pItem->isFolder = false;
			propertyName.Remove (0, PLUGIN_ITEM_FILE_DEF_LEN + 1);
			pItem->itemPath = propertyName;
		}

		PRINT ((" >> %s\n    %s\n", propertyName.String(), lastLine.String()));

		/* Parse the final line to see any inclusions/exclusions, EXCLUDE FILE=cookies eg*/
		if (lastLine.ICompare (PLUGIN_ITEM_EXCLUDE_FILE, PLUGIN_ITEM_EXCLUDE_FILE_LEN) == 0)
		{
			lastLine.Remove (0, PLUGIN_ITEM_EXCLUDE_FILE_LEN + 1);
			pItem->excludeFilePath = lastLine;
		}
		else if (lastLine.ICompare (PLUGIN_ITEM_INCLUDE_MIME, PLUGIN_ITEM_INCLUDE_MIME_LEN) == 0)
		{
			lastLine.Remove (0, PLUGIN_ITEM_INCLUDE_MIME_LEN + 1);
			pItem->includeMimeType = lastLine;
		}
		
		if (recurseLine.ICompare (PLUGIN_ITEM_RECURSE, PLUGIN_ITEM_RECURSE_LEN) == 0)
		{
			recurseLine.Remove (0, PLUGIN_ITEM_RECURSE_LEN + 1);
			if (recurseLine.ICompare (PLUGIN_ITEM_RECURSE_YES) == 0)
				pItem->recurse = true;
			else
				pItem->recurse = false;
		}
		
		/* Add the item (3 lines have been parsed per iteration of this loop) */
		itemList.AddItem ((void*)pItem);
	}
	
	return itemList;
}

/*============================================================================================================*/

void PluginParser::FillContainerItem (PluginContainerItem *item)
{
	PRINT (("PluginParser::FillContainerItem (PluginContainerItem*)\n"));

	/* Declare some variable we are going to need */
	char temp[500];
	BString lineText;

	/* Set default values for the container item */
	item->name = "";
	item->checkPath = "";
	item->isLinear = true;
	
	/* PARSE PLUGIN NAME: Skip all lines not beginning with "[" */
	pluginStream.getline (temp, 300, '\n');
	while (temp[0] != '[' && !pluginStream.eof())
		pluginStream.getline (temp, 300, '\n');

	/* Got to a section start character, now set the name in Container */
	if (temp[0] == '[')
	{
		lineText = temp;
		lineText.Remove (0, 1);
		lineText.RemoveLast ("]");
		item->name = lineText;
	}
	
	/* PARSE PLUGIN TYPE: */
	pluginStream.getline (temp, 500, '\n');
	lineText = temp;

	if (lineText.FindFirst ('=') != B_ERROR)
	{
		BString propertyName (lineText);

		/* Check the first sequence of "propertyName" to see if it is the TYPE= definition */
		if (propertyName.ICompare (PLUGIN_TYPE_DEF, PLUGIN_TYPE_DEF_LEN) == 0)
		{
			/* Now remove "TYPE" and the following equal to "=" */
			lineText.Remove (0, strlen (PLUGIN_TYPE_DEF) + 1);
			
			if (lineText.ICompare (PLUGIN_TYPE_HIERARCHIAL) == 0)
				item->isLinear = false;
			else
				item->isLinear = true;
		}
	}
	
	/* Parse CHECK INSTALL parameter */
	pluginStream.getline (temp, 500, '\n');
	lineText = temp;

	if (lineText.ICompare (PLUGIN_CHECKINSTALL_DEF, PLUGIN_CHECKINSTALL_DEF_LEN) == 0)
	{
		lineText.Remove (0, PLUGIN_CHECKINSTALL_DEF_LEN + 1);
		item->checkPath = lineText;
	}

	/* PARSE SUB-ITEMS: Call a seperate function for this to avoid further confusions */
	item->subItems = Items();
		
	pluginStream.close();
}

/*============================================================================================================*/
