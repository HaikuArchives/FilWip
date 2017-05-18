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

#include <Debug.h>
#include <Message.h>
#include <Path.h>
#include <String.h>
#include <Directory.h>
#include <Entry.h>
#include <Alert.h>
#include <Application.h>
#include <Messenger.h>
#include <NodeInfo.h>

#include <iostream>

#include "FileLooper.h"
#include "FilWip.h"
#include "EraserLooper.h"
#include "Constants.h"
#include "MainWindow.h"
#include "NodeLimit.h"

/*============================================================================================================*/

NodeEntry::NodeEntry ()
{
	/* Initialize the next element in the list to NULL by default */
	next = NULL;
}

/*============================================================================================================*/
/*============================================================================================================*/
/*============================================================================================================*/

HashTable::HashTable (int64 sizeOfTable)
{
	/* Create an empty hash table for the specified size */
	tableSize = sizeOfTable;
	table = new NodeEntry*[tableSize];
	nElements = 0L;

	InitializeTable();
}

/*============================================================================================================*/

HashTable::~HashTable ()
{
	/* Free the hash table elements and finally the table itself */
	MakeEmpty();
	delete[] table;
}

/*============================================================================================================*/

void HashTable::InitializeTable ()
{
	/* Very important we initialize NULL (zero) pointers so our "for" loops works */
	memset ((void*)table, 0, tableSize * sizeof (NodeEntry*));
}

/*============================================================================================================*/

void HashTable::MakeEmpty ()
{
	/* Free the hash table elements if any */
	for (int64 bucket = 0; bucket < tableSize; bucket++)
		for (NodeEntry *element = table[bucket]; element != NULL; element = element->next)
			delete element;

	nElements = 0;
}

/*============================================================================================================*/

int64 HashTable::CountElements() const
{
	return nElements;
}

/*============================================================================================================*/

int64 HashTable::Hash (node_ref *nref) const
{
	/* Simple modulo hash function, the reason why we use this is that node's are incremental in
		a directory, hence will be neatly arranged in consecutive buckets from 0 to tableSize */
	return ((nref->node) % tableSize);
}

/*============================================================================================================*/

NodeEntry* HashTable::LookUp (node_ref *nref, entry_ref *eref, bool insert, bool *wasFound)
{
	int64 hashValue = Hash (nref);
	NodeEntry *bucket = NULL;
	*wasFound = false;

	/* Lookup node in table */
	for (bucket = table[hashValue]; bucket != NULL; bucket = bucket->next)
	{
		if (bucket->nref == *nref)
		{
			*wasFound = true;
			return bucket;
		}
	}


	/* nref is not found, if insert is needed then add it to the table
		NOTE: Addition is done at the TOP of the bucket */
	if (insert == false)
		return bucket;
	
	bucket = new NodeEntry;
	bucket->nref = *nref;
	bucket->eref = *eref;
	BEntry entry (eref);
	entry.GetSize (&(bucket->size));	/* Bug-fix: Don't remove the extra braces */

	bucket->next = table[hashValue];
	table[hashValue] = bucket;	
	nElements ++;
	
	return bucket;
}

/*============================================================================================================*/

bool HashTable::IsFound (node_ref *nref, entry_ref *eref)
{
	/* Check if node_ref is found */
	bool temp;
	LookUp (nref, eref, false, &temp);
	return temp;
}

/*============================================================================================================*/

NodeEntry* HashTable::Find (node_ref *nref)
{
	/* Return pointer to the found item if any */
	bool temp;
	return LookUp (nref, NULL, false, &temp);
}

/*============================================================================================================*/

NodeEntry* HashTable::Insert (node_ref *nref, entry_ref *eref, bool *wasFound)
{
	/* Insert the element in the hash table if unique */
	return LookUp (nref, eref, true, wasFound);
}

/*============================================================================================================*/

bool HashTable::Delete (node_ref *nref, off_t *size)
{
	/* Find the element with the given nref & carefully remove not destroying the  links between the element,
		it is very important we don't break links. Write "size" with the size of the deleted element */
	NodeEntry *targetElement = NULL, *prevElement = NULL;
	int64 hashValue = Hash (nref);

	for (targetElement = table[hashValue]; targetElement != NULL; targetElement = targetElement->next)
	{
		if (targetElement->nref == *nref)
		{
			/* Tricky order, don't alter this segment */
			*size = targetElement->size;
			if (prevElement != NULL)
				prevElement->next = targetElement->next;
			else
				table[hashValue] = targetElement->next;
			
			delete targetElement;
			nElements--;
			return true;
		}
		prevElement = table[hashValue];
	}
	
	size = NULL;
	return false;
}

/*============================================================================================================*/
/*============================================================================================================*/
/*============================================================================================================*/

FileLooper::FileLooper (EraserLooper *eraser, const char *processPath, const char *name, int32 priority,
				bool performSafeCheck, bool isDir, bool recurse, bool debugMode, char *fileName, int32 uniqueID,
				int32 portCapacity)
	: BLooper (name, priority, portCapacity),
	eraserLooper (eraser),
	includedMimeType (NULL),
	excludedFileName (NULL)
{
	/* Setup our internal hash-table representation, make sure its allocated. Assume on an average
		a folder will contain 197 entries :) nice prime number */
	hashTable = new HashTable (197L);
	ASSERT_WITH_MESSAGE (hashTable, "could not create hash table");

	/* Initialize some critical variables, pointers */
	ignoreChanges = false;
	firstScan = true;
	BEntry processEntry (processPath, true);
	isFolder = isDir;
	recurseDir = recurse;
	isDebugMode = debugMode;
	pluginFileName = fileName;
	looperID = uniqueID;
	ResetStatVariables ();
	nSizeLive = nEntriesLive = 0;

	BPath path (processPath);
	if (isFolder == true)
	{
		folderEntry = new BEntry (processPath, false);
		folderEntry->GetNodeRef (&folderNodeRef);
		fileEntry = NULL;

		processDir.SetTo (processPath);
		processDirName = path.Leaf();
		fileEntryName = "";
	}
	else
	{
		fileEntry = new BEntry (processPath, false);
		fileEntry->GetNodeRef (&fileNodeRef);

		BEntry tempEntry;
		fileEntry->GetParent (&tempEntry);
		tempEntry.GetNodeRef (&fileParentNodeRef);
		
		folderEntry = NULL;
		
		processDirName = "";
		fileEntryName = path.Leaf();
	}


	/* Set up ourselves */
	SetUpSelf (performSafeCheck, processPath, priority, fileName);


	/* If our user is working in debug mode, pop him the paths from the plugin */
	if (isDebugMode == true)
	{
		BString debugStr = processPath;
		debugStr.Prepend ("Parsed:\n");
		debugStr.Append ("\n\n" B_UTF8_ELLIPSIS "seems valid, acceptible" B_UTF8_ELLIPSIS);
		BAlert *debugAlert = new BAlert ("OK", debugStr.String(), "OK");
		debugAlert->Go();
		
		std::cout << processPath << std::endl; std::cout.flush();
	}
}

/*============================================================================================================*/

FileLooper::~FileLooper ()
{
	/* Check and delete pointers to free up the memory used */
	stop_watching (NULL, this);
	if (fileEntry) delete fileEntry;
	if (folderEntry) delete folderEntry;
	if (includedMimeType) delete[] includedMimeType;
	if (excludedFileName) delete[] excludedFileName;

	DeleteNodeEntryList();
}

/*============================================================================================================*/

void FileLooper::SetUpSelf (bool performSafeCheck, BPath processLocation, int32 priority, char *fileName)
{
	/* Common function to all versions of the constructor */
	if (performSafeCheck == true)
	{
		/* Fill out the paths that FileLooper will never process (these add a measure of safety
			for misspelt paths in plugins) */
		BString corePaths[] =							// The following dirs can't be deleted
		{
			"/",
			"/boot",
			"/boot/home",
			"/boot/home/config",
			"/boot/home/config/settings",
			"/boot/home/Desktop"
		};	/* 6 paths (0 to 5) */

		/* SafePath check begins!
		/* Next make sure it isn't any of the corePaths (but it CAN be some file/folder
			inside the corePath entry) the below check makes sure any trailing '/'s are removed */
		BString passedPath = processLocation.Path();
		passedPath.Append ('/', 1);
		if (passedPath.ByteAt (passedPath.CountChars() - 1) == '/');
			passedPath.RemoveLast ("/");
		
		for (int32 i = 0; i < 6; i++)
		{
			if (corePaths[i] == passedPath)
			{
				ShowErrorAndQuit (passedPath.String(), fileName);
				return;
			}
		}
	}		
	
	
	/* OK, now we got the path sorted out, meaning its probably a genuine one (which means it is hopefully
		harmless to the system) begin the message loop */
	set_thread_priority (find_thread (NULL), priority);
	Run();
}

/*============================================================================================================*/

void FileLooper::MessageReceived (BMessage *message)
{
	switch (message->what)
	{		
		/* Just do a quick overview to calc file sizes, setup monitors initially */
		case M_BEGIN_OVERVIEW:
		{
			ignoreChanges = false;
			BeginOverviewOperation();
			break;
		}
		
		/* A message from one of our node monitors, handle it */
		case B_NODE_MONITOR:
		{
			if (ignoreChanges == true)
				break;

			int32 opcode;
			if (message->FindInt32 ("opcode", &opcode) != B_OK)
				return;

			dev_t device;
			ino_t node, directory, toDir, fromDir;
			const char *name;
			node_ref nref;
			entry_ref eref;
	
			if (message->FindInt64 ("node", &node) != B_OK) return;
			if (message->FindInt32 ("device", &device) != B_OK) return; 
			
			nref.device = device;
			nref.node = node;
			
			switch (opcode)
			{
				case B_ENTRY_CREATED:
				{
					PRINT (("FileLooper::MessageReceived()\n >> %s->%s Action=B_ENTRY_CREATED\n", pluginFileName.String(),
								isFolder ? processDirName.String() : fileEntryName.String()));

					message->FindInt64 ("directory", &directory); 
					message->FindString ("name", &name);

					eref.device = device;
					eref.directory = directory;
					eref.set_name (name);
					
					/* If recursion is OFF, don't bother about directory creations */
					if (recurseDir == false)
					{
						BEntry entry (&eref);
						if (entry.IsDirectory())
							break;
					}
					
					AddUniqueNodeToList (&nref, &eref);
					SendNodeChangedMessage ();
					break;
				}
				
				case B_ENTRY_REMOVED:
				{
					PRINT (("FileLooper::MessageReceived()\n >> %s->%s Action=B_ENTRY_REMOVED\n",
						pluginFileName.String(), isFolder ? processDirName.String() : fileEntryName.String()));
					
					/* We have a problem here! Because node is removed we cannot construct a BEntry to
						the node and check if it is a dir or not. So how do we find if it is a sub-dir
						or file that has been deleted? What we do is we count both files+folders and size of
						both files+folders - so don't bother if it is a file/folder */
					RemoveNodeFromList (&nref);
					SendNodeChangedMessage ();
					break;
				}
				
				case B_STAT_CHANGED:
				{
					PRINT (("FileLooper::MessageReceived()\n >> %s->%s Action=B_STAT_CHANGED\n", pluginFileName.String(),
								isFolder ? processDirName.String() : fileEntryName.String()));

					UpdateNodeSize (&nref);
					SendNodeChangedMessage ();
					break;
				}
				
				/* Renamed and moved entries both show up here */
				case B_ENTRY_MOVED:
				{
					PRINT (("FileLooper::MessageReceived()\n >> %s->%s Action=B_ENTRY_MOVED\n", pluginFileName.String(),
								isFolder ? processDirName.String() : fileEntryName.String()));
					
					message->FindInt64 ("from directory", &fromDir);
					message->FindInt64 ("to directory", &toDir);
					message->FindString ("name", &name);
					
					node_ref folderRef;

					/* Modify eref here to suit the moved (change directory to destination directory ino_t) */
					eref.device = device;
					eref.directory = toDir;
					eref.set_name (name);
					
					node_ref toDirNode;
					toDirNode.node = toDir;
					toDirNode.device = device;

					node_ref fromDirNode;
					fromDirNode.node = fromDir;
					fromDirNode.device = device;
					
					/* Check if there has been a renaming of file or folder */
					if (fileEntry != NULL)
					{
						/* Check if our file has been renamed */
						if (strcmp (eref.name, fileEntryName.String()) != 0)
						{
							DeleteNodeEntryList();
							SendNodeChangedMessage ();
							
							BEntry parentDir;
							BEntry entry (&eref);
							entry.GetParent (&parentDir);
							WatchNode (&parentDir, B_WATCH_DIRECTORY, NULL, this);
							break;
						}
						else
							AddUniqueNodeToList (&nref, &eref);
					}
					else
					{
						/* Check if our folder has been renamed */
						if (folderNodeRef == nref)
						{
							DeleteNodeEntryList();
							if (strcmp (eref.name, processDirName.String()) != 0)
								SendNodeChangedMessage();
							else
								BeginOverviewOperation();

							break;
						}
					}

					/* Check if a renamed item matches/unmatches the criteria */
					if (excludedFileName != NULL)
					{
						if (strcmp (eref.name, excludedFileName) == 0)
						{
							RemoveNodeFromList (&nref);
							SendNodeChangedMessage();
							return;
						}	
					}

					/* If we are processing a folder, get its node_ref, if a file
						get its parent directory's node_ref */
					if (isFolder == true && processDir.IsDirectory() == true)
						folderRef = folderNodeRef;
					else
						folderRef = fileParentNodeRef;
					
					/* Check if mimetype is allowed (to be included) */
					if (includedMimeType != NULL)
					{
						BNode node (&eref);
						BNodeInfo nodeInfo (&node);
						
						char buf[B_MIME_TYPE_LENGTH];
						nodeInfo.GetType (buf);
						
						/* Mime-type doesn't match - forget it*/
						if (strcmp (buf, includedMimeType) != 0)
							break;
					}
					
					if (recurseDir == false)
					{
						BEntry entry (&eref);
						if (entry.IsDirectory())
							break;
					}
					else
					{
						BEntry entry (&eref);
						if (entry.IsDirectory())
						{
							/* Thus if the moving from or to the dir we are monitoring has folders the entire
								overview operation restarts */
							DeleteNodeEntryList();
							BeginOverviewOperation();
							break;
						}
					}
					
					/* If the file was moved To our dir, add it */
					if (folderRef.node == toDir)
					{
						AddUniqueNodeToList (&nref, &eref);
					}
					else if (folderRef.node == fromDir)
					{
						/* Check if the entry was NOT moved to a sub-dir */
						if (hashTable->Find (&toDirNode) == NULL)
							RemoveNodeFromList (&nref);
					}
					else
					{
						/* An entry could have been moved TO our sub-dir from somewhere check that here */
						if (hashTable->Find (&fromDirNode) != NULL)
						{
							/* Make sure it isn't renamed meaning FROM and TO will be sub-dirs */
							if (hashTable->Find (&toDirNode) == NULL)
								RemoveNodeFromList (&nref);
						}
						else if (hashTable->Find (&toDirNode) != NULL)
						{
							/* An entry could have been moved TO our sub-dir from somewhere */
							AddUniqueNodeToList (&nref, &eref);
						}
					}

					SendNodeChangedMessage();
					break;
				}
				
				/* Update our list for attribute changes if need be */
				case B_ATTR_CHANGED:
				{
					if (includedMimeType == NULL)
						break;
					
					PRINT (("FileLooper::MessageReceived()\n >> %s->%s Action=B_ATTR_CHANGED\n", pluginFileName.String(),
								isFolder ? processDirName.String() : fileEntryName.String()));
					
					UpdateNodeAttribute (&nref);
					SendNodeChangedMessage ();
					break;
				}
			}
			break;
		}
		
		default:
			BLooper::MessageReceived (message);
	}
}

/*============================================================================================================*/

void FileLooper::ShowErrorAndQuit (const char *pathNotProcessable, char *fileName)
{
	/* Declare all variables here to avoid jumps to labels */
	BString errorString, infoStr;
	BAlert *alert, *info;
	int32 index;

	
	/* Time to show the error and quit ourself, the cause being a messed-up/unsafe path */
	errorString = "Parsed:\n";
	errorString.Append (pathNotProcessable);
	errorString.Append ("\n\n" B_UTF8_ELLIPSIS "not acceptible!" B_UTF8_ELLIPSIS "\n");
	errorString.Append ("\nPlugin: ");
	errorString.Append (fileName);

	/* Construct and show the about box */
	alert = new BAlert ("Error", errorString.String(), "More Info" B_UTF8_ELLIPSIS, "OK", NULL,
					B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_STOP_ALERT);
	index = alert->Go();
	
	if (index == 1)
		goto total_melt_down;
	
	infoStr << "This error was encountered because the plugin tries to process a file/folder critical ";
	infoStr << "to BeOS. Because plugins are modifiyable, a special check has been added to make ";
	infoStr << "sure such system entries cannot be removed.\n\n";
	infoStr << "To rectify the problem, correct the 'illegal' path in the mentioned plugin file.";
	infoStr << "\n\n\t\t\t\t\tHappy debugging :)";

	info = new BAlert ("Error", infoStr.String(), "Close", NULL, NULL, B_WIDTH_AS_USUAL,
						B_EVEN_SPACING, B_STOP_ALERT);
	info->Go();


	/* This sub hides all the windows and sends a quit requested message to our master */
	total_melt_down:
		isDebugMode = false;
		filWip->mainWnd->Lock();
		filWip->mainWnd->Hide();
		filWip->mainWnd->debugMode = false;
		filWip->mainWnd->Unlock();
		
		/* Snooze for sometime till other FileLoopers have settled down */
		snooze (100000);
		be_app->PostMessage (B_QUIT_REQUESTED);
		Quit();
}

/*============================================================================================================*/

void FileLooper::IncludeMimeType (char *mimeType)
{
	if (includedMimeType != NULL)
		delete[] includedMimeType;

	if (mimeType == NULL)
		return;

	includedMimeType = new char[strlen (mimeType)];
	strcpy (includedMimeType, mimeType);
}

/*============================================================================================================*/

void FileLooper::ExcludeFileName (char *fileName)
{
	if (excludedFileName != NULL)
		delete[] excludedFileName;
	
	if (fileName == NULL)
		return;
	
	excludedFileName = new char[strlen (fileName)];
	strcpy (excludedFileName, fileName);
}

/*============================================================================================================*/

void FileLooper::ResetStatVariables ()
{
	/* Just reset stat variables */
	entriesWiped = 0;
	sizeWiped = 0;
}

/*============================================================================================================*/

void FileLooper::BeginOverviewOperation()
{
	/* Control the overview operation just report the size, no of entries without doing anything */
	BMessage reportMsg (M_OVERVIEW_STATS);
	

	/* Free our internal representation so we don't monitor nodes twice */
	DeleteNodeEntryList ();
	
	if (fileEntry != NULL)
		OverviewFile();
	else if (excludedFileName != NULL)
		OverviewFolderExcludeFileName (processDir);
	else if (includedMimeType != NULL)
		OverviewFolderIncludeMimeType (processDir);
	else
		OverviewFolder (processDir);
	

	/* Post message about details of the wipeout */
	reportMsg.AddInt32 ("looper_id", looperID);
	reportMsg.AddInt64 ("entries_counted", entriesWiped);
	reportMsg.AddInt64 ("bytes_counted", sizeWiped);
	reportMsg.AddBool ("first_scan", firstScan);
	firstScan = false;
	filWip->mainWnd->PostMessage (&reportMsg);
	
	nSizeLive = sizeWiped;
	nEntriesLive = entriesWiped;
	ResetStatVariables();
}

/*============================================================================================================*/

void FileLooper::OverviewFile ()
{
	if (fileEntry->Exists() == true)
	{
		fileEntry->GetSize (&sizeWiped);
		entriesWiped ++;
	}
	
		
	/* Setup a monitor for the file & its parent folder so we notifications if the file is created newly */
	AddNodeMonitor (*fileEntry);
	BEntry dirEntry;
	fileEntry->GetParent (&dirEntry);
	WatchNode (&dirEntry, B_WATCH_DIRECTORY | B_WATCH_NAME | B_WATCH_STAT, NULL, this);
}

/*============================================================================================================*/

void FileLooper::OverviewFolderExcludeFileName (BDirectory folder)
{
	/* Overview all the entries in a folder excepting a filenamed 'excludedFileName' */
	BEntry entry;
	off_t size;
	char buffer[B_FILE_NAME_LENGTH];

	if (folder.InitCheck() != B_OK)
		return;
	
	
	/* Monitor the directory as well as all its files */
	BEntry dirEntry;	
	folder.GetEntry (&dirEntry);
	WatchNode (&dirEntry, B_WATCH_DIRECTORY | B_WATCH_NAME | B_WATCH_STAT, NULL, this);
	
	while (folder.GetNextEntry (&entry, false) != B_ENTRY_NOT_FOUND)
	{
		entry.GetName (buffer);
		if (strcmp (buffer, excludedFileName) == 0)
			continue;
		
		if (entry.IsDirectory() == true)
		{
			if (recurseDir == false)
				continue;

			BDirectory subDir (&entry);
			OverviewFolderExcludeFileName (subDir);
		}
	
		entriesWiped++;
		entry.GetSize (&size);
		sizeWiped += size;
		AddNodeMonitor (entry);
	}
}

/*============================================================================================================*/

void FileLooper::OverviewFolderIncludeMimeType (BDirectory folder)
{
	/* Overview all entries in a folder with the mimetype in 'includedMimeType' */
	BEntry entry;
	off_t size;
	char buffer[B_MIME_TYPE_LENGTH];

	if (folder.InitCheck() != B_OK)
		return;

	BEntry dirEntry;	
	folder.GetEntry (&dirEntry);
	WatchNode (&dirEntry, B_WATCH_DIRECTORY, NULL, this);
	
	while (folder.GetNextEntry (&entry, false) != B_ENTRY_NOT_FOUND)
	{
		BNode node (&entry);
		BNodeInfo nodeInfo (&node);
		nodeInfo.GetType (buffer);

		if (strcmp (buffer, includedMimeType) != 0)
			continue;
		
		if (entry.IsDirectory() == true)
		{
			if (recurseDir == false)
				continue;

			BDirectory subDir (&entry);
			OverviewFolderIncludeMimeType (subDir);
		}

		entriesWiped++;
		entry.GetSize (&size);
		sizeWiped += size;
		AddNodeMonitor (entry);
	}
}

/*============================================================================================================*/

void FileLooper::OverviewFolder (BDirectory folder)
{
	/* Overview full folder and subdirs */
	BEntry entry;
	off_t size;
	if (folder.InitCheck() != B_OK)
		return;

	BEntry dirEntry;
	folder.GetEntry (&dirEntry);
	WatchNode (&dirEntry, B_WATCH_DIRECTORY, NULL, this);

	while (folder.GetNextEntry (&entry, false) != B_ENTRY_NOT_FOUND)
	{
		entry.GetSize (&size);
		if (entry.IsDirectory() == true)
		{
			if (recurseDir == false)
				continue;

			BDirectory subDir (&entry);
			OverviewFolder (subDir);
		}

		entriesWiped++;
		entry.GetSize (&size);
		sizeWiped += size;
		AddNodeMonitor (entry);
	}
}

/*============================================================================================================*/

void FileLooper::AddNodeMonitor (BEntry entry)
{
	/* Setup the node monitor */
	if (entry.IsDirectory())
		WatchNode (&entry, B_WATCH_DIRECTORY, NULL, this);
	else
		WatchNode (&entry, B_WATCH_NAME | B_WATCH_STAT | B_WATCH_ATTR, NULL, this);
	
	/* Add the corresponding node_ref and entry_ref into our internal representation */
	node_ref nref;
	entry_ref eref;
	
	if (entry.Exists() == false)
		return;

	entry.GetNodeRef (&nref);
	entry.GetRef (&eref);
	bool temp;
	hashTable->Insert (&nref, &eref, &temp);
}

/*============================================================================================================*/

void FileLooper::DeleteNodeEntryList ()
{
	/* Free our internal node monitor cache & reset live-state variables */
	stop_watching (NULL, this);
	hashTable->MakeEmpty();
	hashTable->InitializeTable ();
	nSizeLive = 0;
	nEntriesLive = 0;
}

/*============================================================================================================*/

void FileLooper::SendNodeChangedMessage ()
{
	/* Post message about details of the node monitor changes */
	BMessage reportMsg (M_OVERVIEW_STATS);
	reportMsg.AddInt32 ("looper_id", looperID);
	reportMsg.AddInt64 ("entries_counted", nEntriesLive);
	reportMsg.AddInt64 ("bytes_counted", nSizeLive);
	filWip->mainWnd->PostMessage (&reportMsg);
}

/*============================================================================================================*/

void FileLooper::AddUniqueNodeToList (node_ref *nref, entry_ref *eref)
{
	/* Make sure we add things to our list adhering to the criteria if any */
	if (excludedFileName != NULL)
		if (strcmp (eref->name, excludedFileName) == 0)
			return;

	/* Can't remember why i put this check ... */
	if (fileEntry != NULL)
	{
		entry_ref ref;
		node_ref noder;

		fileEntry->GetRef (&ref);
		fileEntry->GetNodeRef (&noder);
		if (ref != *eref || noder != *nref)
			return;
	}
	
	/* Begin our hash table operations */
	bool wasFound;
	NodeEntry *newElement = hashTable->Insert (nref, eref, &wasFound);
	if (wasFound == false)
	{
		nSizeLive += (newElement->size);
		nEntriesLive++;

		/* Setup the node monitor */
		BEntry entry (eref);
		if (entry.IsDirectory())
			WatchNode (newElement->nref, B_WATCH_DIRECTORY, NULL, this);
		else
			WatchNode (newElement->nref, B_WATCH_NAME | B_WATCH_STAT | B_WATCH_ATTR, NULL, this);
	}
}

/*============================================================================================================*/

void FileLooper::RemoveNodeFromList (node_ref *nref)
{
	/* Delete an existing item, adjust our size and entriesLive statistic variables */
	off_t size;
	if (hashTable->Delete (nref, &size) == true)
	{
		nSizeLive -= size;
		nEntriesLive--;
	}
}

/*============================================================================================================*/

void FileLooper::UpdateNodeSize (node_ref *nref)
{
	/* Search and update size of node_ref from our internal representation */
	entry_ref eref;
	off_t newSize, oldSize;
	BEntry entry;
	
	NodeEntry *existingElement = hashTable->Find (nref);
	if (existingElement == NULL)
		return;
	
	oldSize = existingElement->size;
	entry.SetTo (&(existingElement->eref));
	entry.GetSize (&newSize);
	
	if (oldSize > newSize)
		nSizeLive -= (oldSize - newSize);
	else
		nSizeLive += (newSize - oldSize);

	existingElement->size = newSize;
}

/*============================================================================================================*/

void FileLooper::UpdateNodeAttribute (node_ref *nref)
{
	/* Check to add a new item based on its attribute change, NOTE: Don't alter this function freely,
		it has been regorously tested like: changing mime of several files at once, delete, create, move etc. */
	entry_ref eref;
	node_ref newNodeRef;
	char buf[B_MIME_TYPE_LENGTH];

	BDirectory parentDir (processDir);
	parentDir.Rewind();

	/* Locate the newly made entry through its node -no other way- scan the parent
		folder, get entry, compare if their nodes are equal, if so check the attrib, if equal, add */
	while (parentDir.GetNextRef (&eref) != B_ENTRY_NOT_FOUND)
	{
		BNode tmp (&eref);
		tmp.GetNodeRef (&newNodeRef);
		
		if (newNodeRef == *nref)		/* Found the entry_ref, now check its attrib */
		{
			BNodeInfo nodeInfo (&tmp);
			nodeInfo.GetType (buf);
			if (strcmp (buf, includedMimeType) == 0)
			{
				AddUniqueNodeToList (nref, &eref);
				SendNodeChangedMessage();
				return;
			}
		}
	}


	/* Search and update size of node_ref from our internal representation we come here only when
		have something in the list to be removed if it mime-type is NOT includedMimeType */
	NodeEntry *ne = hashTable->Find (nref);
	if (ne == NULL)
		return;

	eref = ne->eref;
	BEntry entry (&eref);
	BNode node (&eref);
	BNodeInfo nodeInfo (&node);

	nodeInfo.GetType (buf);	
	if (!(strcmp (buf, includedMimeType) == 0))
	{
		RemoveNodeFromList (nref);
		return;
	}
}

/*============================================================================================================*/

void FileLooper::AddFile ()
{
	/* Add the file entry to the global eraser looper */
	if (fileEntry->Exists() == false)
		return;

	BPath path;
	fileEntry->GetPath (&path);

	off_t size;
	fileEntry->GetSize (&size);

	char *pathStr = new char[B_PATH_NAME_LENGTH];
	strcpy (pathStr, path.Path());

	eraserLooper->entryList.AddItem (pathStr);	
	eraserLooper->totalFiles++;
	eraserLooper->totalBytes += size;
}

/*============================================================================================================*/

void FileLooper::AddFolder (BDirectory folder)
{
	/* No tension, just add all entries no matter what name/mime type to our eraser */
	BEntry entry;

	if (folder.InitCheck() != B_OK)
		return;
	
	int32 fileCount = 0L, folderCount = 0L;
	int64 byteCount = 0LL;
	off_t size;
	while (folder.GetNextEntry (&entry, false) != B_ENTRY_NOT_FOUND)
	{
		if (entry.IsDirectory() == true)
		{
			if (recurseDir == false)
				continue;

			BDirectory subDir (&entry);
			AddFolder (subDir);
			folderCount++;
		}
		else
		{
			BPath path;
			entry.GetPath (&path);

			char *pathStr = new char[B_PATH_NAME_LENGTH];
			strcpy (pathStr, path.Path());

			eraserLooper->entryList.AddItem (pathStr);
			fileCount++;
		}

		entry.GetSize (&size);
		byteCount += size;
	}
	
	/* Add sub-directories to delete list (if recursive option is ON) */
	if (folder != processDir && recurseDir == true)
	{
		folder.GetEntry (&entry);
		BPath path;
		entry.GetPath (&path);

		char *pathStr = new char[B_PATH_NAME_LENGTH];
		strcpy (pathStr, path.Path());

		eraserLooper->entryList.AddItem (pathStr);
	}
	
	eraserLooper->totalFiles += fileCount;
	eraserLooper->totalFolders += folderCount;
	eraserLooper->totalBytes += byteCount;
}

/*============================================================================================================*/

void FileLooper::AddFolderExcludeFileName (BDirectory folder)
{
	/* Remove all the entries in a folder excepting a filenamed 'excludedFileName' */
	BEntry entry;
	char buffer[B_FILE_NAME_LENGTH];

	if (folder.InitCheck() != B_OK)
		return;

	int32 fileCount = 0L, folderCount = 0L;
	int64 byteCount = 0LL;
	off_t size;
	while (folder.GetNextEntry (&entry, false) == B_OK)
	{
		entry.GetName (buffer);
		if (strcmp (buffer, excludedFileName) == 0)
			continue;

		if (entry.IsDirectory() == true)
		{
			if (recurseDir == false)
				continue;

			BDirectory subDir (&entry);
			AddFolderExcludeFileName (subDir);
			folderCount++;
		}
		else
		{
			BPath path;
			entry.GetPath (&path);
		
			char *pathStr = new char[B_PATH_NAME_LENGTH];
			strcpy (pathStr, path.Path());

			eraserLooper->entryList.AddItem (pathStr);
			fileCount++;
		}

		entry.GetSize (&size);
		byteCount += size;
	}

	/* Add sub-directories to delete list */
	if (folder != processDir && recurseDir == true)
	{
		folder.GetEntry (&entry);
		BPath path;
		entry.GetPath (&path);

		char *pathStr = new char[B_PATH_NAME_LENGTH];
		strcpy (pathStr, path.Path());

		eraserLooper->entryList.AddItem (pathStr);
	}

	eraserLooper->totalFiles += fileCount;
	eraserLooper->totalFolders += folderCount;
	eraserLooper->totalBytes += byteCount;
}

/*============================================================================================================*/

void FileLooper::AddFolderIncludeMimeType (BDirectory folder)
{
	/* Add all entries in a folder with the mimetype in 'includedMimeType' to a BList, then add that
		BList to our eraser looper */
	BEntry entry;
	char buffer[B_MIME_TYPE_LENGTH];

	if (folder.InitCheck() != B_OK)
		return;
	
	int32 fileCount = 0L, folderCount = 0L;
	int64 byteCount = 0LL;
	off_t size;
	while (folder.GetNextEntry (&entry, false) == B_OK)
	{
		BNode node (&entry);
		BNodeInfo nodeInfo (&node);
		nodeInfo.GetType (buffer);

		if (strcmp (buffer, includedMimeType) != 0)
			continue;
		
		if (entry.IsDirectory() == true)
		{
			if (recurseDir == false)
				continue;

			BDirectory subDir (&entry);
			AddFolderIncludeMimeType (subDir);
			folderCount++;
		}
		else
		{
			BPath path;
			entry.GetPath (&path);

			char *pathStr = new char[B_PATH_NAME_LENGTH];
			strcpy (pathStr, path.Path());

			eraserLooper->entryList.AddItem (pathStr);
			fileCount++;
		}

		entry.GetSize (&size);
		byteCount += size;
	}

	/* Add sub-directories to delete list */
	if (folder != processDir && recurseDir == true)
	{
		folder.GetEntry (&entry);
		BPath path;
		entry.GetPath (&path);

		char *pathStr = new char[B_PATH_NAME_LENGTH];
		strcpy (pathStr, path.Path());

		eraserLooper->entryList.AddItem (pathStr);
	}
	
	/* Don't put this inside loop, maybe it improves speed this way */
	eraserLooper->totalFiles += fileCount;
	eraserLooper->totalFolders += folderCount;
	eraserLooper->totalBytes += byteCount;
}

/*============================================================================================================*/

void FileLooper::BeginAddOperation ()
{
	/* Use the appropriate hook to add the required entries to the eraser looper */
	if (eraserLooper->Lock() == true)
	{
		if (fileEntry != NULL)
			AddFile();
		else if (excludedFileName != NULL)
			AddFolderExcludeFileName (processDir);
		else if (includedMimeType != NULL)
			AddFolderIncludeMimeType (processDir);
		else
			AddFolder (processDir);

		DeleteNodeEntryList();
		SendNodeChangedMessage();

		eraserLooper->Unlock();
	}
}

/*============================================================================================================*/
