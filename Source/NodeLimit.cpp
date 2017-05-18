/*
 * FilWip
 * Copyright (c) 2001 Ramshankar
 *
 * This license does not apply to source code written by third parties included with
 * and/or used by this software. This includes third party modules or any other form
 * of source code provided by third parties or by an unknown source.
 *
 * Original code by:
 * Unknown source
 *
 * Modifications/Additions by:
 * Ramshankar
 *
 */

#include "NodeLimit.h"

#include <errno.h>
#include <sys/resource.h>
#include <sys/stat.h>

/* Set nodeCount to the current limit and bumpValue additional monitors for each 4096 limit */
int nodeCount = 4096;
const int32 bumpValue = 512;

/* Define a code locking object for making these function thread safe */
BLocker codeLocker;

/*============================================================================================================*/

status_t NeedMoreNodeMonitors ()
{
	/* Bump node monitor count up BY "bumpValue" */
	codeLocker.Lock();
	nodeCount += bumpValue;
	codeLocker.Unlock();

	struct rlimit rl;
	if (nodeCount < 1)
		return EINVAL;
	rl.rlim_cur = nodeCount;
	rl.rlim_max = RLIM_SAVED_MAX;
	if (setrlimit(RLIMIT_NOVMON, &rl) < 0)
		return errno;
	return B_OK;
}

/*============================================================================================================*/

status_t WatchNode (BEntry *entry, uint32 flags, const BHandler *handler, const BLooper *looper)
{
	codeLocker.Lock();

	/* Derive a node_ref from the entry received */
	node_ref node;
	entry->GetNodeRef (&node);


	/* Allocate a node monitor normally */	
	status_t result = watch_node (&node, flags, handler, looper);
	if (result == B_OK || result != B_NO_MEMORY)
	{
		codeLocker.Unlock();
		return result;
	}
	
	
	/* Failed to start monitor, try to allocate more monitors */
	result = NeedMoreNodeMonitors();


	/* Failed to allocate more monitors */
	if (result != B_OK)
	{
		codeLocker.Unlock();
		return result;
	}
	
	
	codeLocker.Unlock();
	
	/* Try again, this time with more node monitors */
	return watch_node (&node, flags, handler, looper);
}

/*============================================================================================================*/

status_t WatchNode (node_ref node, uint32 flags, const BHandler *handler, const BLooper *looper)
{
	/* Allocate a node monitor normally */	
	codeLocker.Lock();
	status_t result = watch_node (&node, flags, handler, looper);
	if (result == B_OK || result != B_NO_MEMORY)
	{
		codeLocker.Unlock();
		return result;
	}
	
	/* Failed to start monitor, try to allocate more monitors */
	result = NeedMoreNodeMonitors();

	/* Failed to allocate more monitors */
	if (result != B_OK)
	{
		codeLocker.Unlock();
		return result;
	}
	
	/* Try again, this time with more node monitors */
	codeLocker.Unlock();
	return watch_node (&node, flags, handler, looper);
}

/*============================================================================================================*/
