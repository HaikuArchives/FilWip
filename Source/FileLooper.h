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

#ifndef _FILE_LOOPER_H
#define _FILE_LOOPER_H

#include <Looper.h>
#include <ColumnListView.h>

class EraserLooper;

class NodeEntry
{
	public:
		NodeEntry();
		
		node_ref			nref;
		entry_ref			eref;
		off_t				size;
		
		NodeEntry			*next;
};

class HashTable
{
	public:
		HashTable (int64 sizeOfTable);
		~HashTable ();
		
		/* Hooks */
		void				InitializeTable ();
		void				MakeEmpty ();
		bool				IsFound (node_ref *nref, entry_ref *eref);
		bool				Delete (node_ref *nref, off_t *size);
		int64				Hash (node_ref *nref) const;
		int64				CountElements () const;
		NodeEntry			*LookUp (node_ref *nref, entry_ref *eref, bool insert, bool *wasFound);
		NodeEntry			*Insert (node_ref *nref, entry_ref *eref, bool *wasFound);
		NodeEntry			*Find (node_ref *nref);
		
	private:
		/* Private variables */
		NodeEntry			**table;
		int64				tableSize,
							nElements;
};

class FileLooper : public BLooper
{
	public:
		FileLooper (EraserLooper *eraser, const char *path, const char *name, int32 priority,
				bool performSafeCheck, bool isDir, bool recurse, bool debugMode, char *fileName,
				int32 uniqueID, BRow *row, int32 portCapacity = 1000);
		virtual ~FileLooper();
		
		/* Extra hooks */
		void				IncludeMimeType (char *mimeType);
		void				ExcludeFileName (char *fileName);
		void				BeginAddOperation ();
		
		volatile bool		stopProcessing,
							ignoreChanges;
		
	protected:
		/* Protected inherited hooks */
		virtual void		MessageReceived (BMessage *message);

		/* Protected extra hooks */
		void				WipeFile ();
		void				AddFile ();
		void				AddFolderExcludeFileName (BDirectory folder);
		void				AddFolderIncludeMimeType (BDirectory folder);
		void				AddFolder (BDirectory folder);
		void				ResetStatVariables ();
		
		void				BeginOverviewOperation ();
		void				OverviewFile ();
		void				OverviewFolderExcludeFileName (BDirectory folder);
		void				OverviewFolderIncludeMimeType (BDirectory folder);
		void				OverviewFolder (BDirectory folder);
		
		void				AddNodeMonitor (BEntry entry);
		void				DeleteNodeEntryList ();
		void				SendNodeChangedMessage ();
		void				AddUniqueNodeToList (node_ref *nref, entry_ref *eref);
		void				RemoveNodeFromList (node_ref *nref);
		void				UpdateNodeSize (node_ref *nref);
		void				UpdateNodeAttribute (node_ref *nref);
				
		void				ShowErrorAndQuit (const char *pathNotProcessable, char *fileName);
		void				SetUpSelf (bool performSafeCheck, BPath processLocation, int32 priority,
								char *fileName);

		/* Private variables (better kept private) */
		EraserLooper		*eraserLooper;
		HashTable			*hashTable;
		BEntry				*fileEntry,
							*folderEntry;
		BDirectory			processDir;
		BPath				processPath;
		bool				isFolder,
							isDebugMode,
							isFirstTime,
							recurseDir,
							firstScan;
		char				*includedMimeType,
							*excludedFileName;
		BString				pluginFileName,
							processDirName,
							fileEntryName;
		node_ref			fileNodeRef,
							folderNodeRef,
							fileParentNodeRef;
		BRow				*fRow;
		/* Statistics :) */
		off_t				sizeWiped,
							nSizeLive;
		bigtime_t			timeTaken;
		int64				entriesWiped,
							nEntriesLive,
							nFoldersLive,
							nFilesLive,
							foldersWiped,
							filesWiped;
		int32				looperID;
};

#endif /* _FILE_LOOPER_H */
