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

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#define AppTitle					"FilWip"
#define AppSignature				"application/x-vnd.Ram-FilWip"
#define AppVersion					"0.1.0 (x86)"
#define PresetSignature				"application/x-vnd.FilWip-Preset"

const uint32
	M_PARSE_AND_SETUP_UI =	'psui',
	 M_CHECKBOX_CHANGED = 	'chgd',
	 M_CLEANUP =			'clup',
	 M_PREVIEW =			'prvw',
	 M_CLOSE_PREVIEW =		'clpw',
	 M_HELP =				'chlp',
	 M_PREFS =				'opts',
	 M_CLOSE_PREFS =		'clst',
	 M_SELECT_ALL =			'slal',
	 M_DESELECT_ALL =		'dsal',
	 M_SMART_SELECT =		'smsl',
	 M_REPORT =				'rprt',
	 M_CLOSE_REPORT =		'clrp',
	 M_ABOUT =				'abtc',
	 M_CLOSE_ABOUT =		'clab',
	 M_SAVE_PRESET =		'svps',
	 M_OPEN_PRESET =		'rdps',
	 M_JUST_GOT_FOCUS =		'jgfc',
	 M_THREAD_EXITED = 		'tskd',
	 M_SUPERITEM_EXP =		'spxp',
	 M_SUPERITEM_COL =		'spcl',
	 M_SUPERITEM_MID =		'spmw',
	 M_TREE_STATES =		'tres',
	 M_WIPE_DONE =			'wpdn',
	 M_BEGIN_OVERVIEW =		'bgov',
	 M_OVERVIEW_STATS =		'ovws',
	 M_RESET_LOOPERS =		'rslp';

const float
	DialogMargin =			12,
	SmallMargin =			6,
	ButtonHeight =			25,
	ButtonWidth =			73,
	ButtonSpacing =			12,
	PresetVerifyCode =		-184597316.598712;

const rgb_color
	TreeLabelColor =		{152,   0,   0, 255},
	TreeLabelDimmed =		{68 ,   0,   0, 255},
	ItemsViewColor =		{236, 241, 241, 255},
	ItemsSelectColor =		{  0,   0, 172, 255},
	ItemsUnselectColor =	{  0,   0,   0, 255},
	StatusBarColor =		{245, 187,  46, 255},
	PrefsItemSelBackColor =	{202, 212, 240, 255},
	PrefsItemSelForeColor = {  0,   0,   0, 255},

	BeViewColor =			{219, 219, 219, 255},
	BeLightenedShadow =		{162, 162, 162, 255},
	BeDarkenedShadow =		{152, 152, 152, 255},
	BeDarkestShadow =		{108, 108, 108, 255},
	BeLightestShadow =		{194, 194, 194, 255},
	BeLightedEdge =			{255, 255, 255, 255},
	BePureWhite =			{255, 255, 255, 255},
	BeJetBlack =			{  0,   0,   0, 255},
	GoldenColor =			{247, 211, 105, 255};

#endif /* _CONSTANTS_H */
