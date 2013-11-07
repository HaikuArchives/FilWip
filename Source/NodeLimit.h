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
 * Minor extensions by:
 * Ramshankar
 *
 */

#ifndef _NODE_LIMIT_H
#define _NODE_LIMIT_H

#include <SupportDefs.h>
#include <StorageDefs.h>
#include <Messenger.h>
#include <NodeMonitor.h>
#include <Entry.h>
#include <Locker.h>

/* We need this internal call in order to break the node limit */
#ifdef __cplusplus
	extern "C" _IMPEXP_ROOT int _kset_mon_limit_ (int num);
#else
	_IMPEXP_ROOT int _kset_mon_limit_ (int num);
#endif

/* Prototypes */
status_t NeedMoreNodeMonitors ();
status_t WatchNode (BEntry *entry, uint32 flags, const BHandler *handler, const BLooper *looper);
status_t WatchNode (node_ref node, uint32 flags, const BHandler *handler, const BLooper *looper);

#endif /* _NODE_LIMIT_H */
