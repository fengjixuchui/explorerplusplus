// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

/*
 * Remembers path history, and
 * includes the ability to browse
 * back/forward through a set
 * of paths.
 */

#include "stdafx.h"
#include "PathManager.h"
#include "../Helper/Helper.h"
#include "../Helper/Macros.h"
#include "../Helper/ShellHelper.h"

PathManager::PathManager()
{
	m_iCurrent = 0;

	m_nAllocated = DEFAULT_ALLOCATION;
	ppidlList = (PIDLIST_ABSOLUTE *)malloc(m_nAllocated * sizeof(PIDLIST_ABSOLUTE));

	m_nTotal = 0;
}

void PathManager::AddEntry(PCIDLIST_ABSOLUTE pidl)
{
	/* Check if the number of idl's stored has reached the number of
	spaces allocated. If so, allocate a new block. */
	if(m_iCurrent >= (m_nAllocated - 1))
	{
		m_nAllocated += DEFAULT_ALLOCATION;

		ppidlList = (PIDLIST_ABSOLUTE *)realloc(ppidlList,
		m_nAllocated * sizeof(PIDLIST_ABSOLUTE));
	}

	ppidlList[m_iCurrent++] = ILCloneFull(pidl);

	/* "Erases" idl's forward of the current one. */
	m_nTotal = m_iCurrent;
}

PIDLIST_ABSOLUTE PathManager::GetEntry(int iIndex)
{
	if((m_iCurrent + iIndex) < 0 ||
	(m_iCurrent + iIndex) > m_nTotal)
	{
		return NULL;
	}

	/* Update the current folder pointer to point to the
	folder that was selected. */
	m_iCurrent += iIndex;

	return ILCloneFull(ppidlList[m_iCurrent - 1]);
}

PIDLIST_ABSOLUTE PathManager::GetEntryWithoutUpdate(int iIndex)
{
	if((m_iCurrent + iIndex) < 0 ||
	(m_iCurrent + iIndex) > m_nTotal)
	{
		return NULL;
	}

	return ILCloneFull(ppidlList[m_iCurrent + iIndex - 1]);
}

int PathManager::GetNumBackEntriesStored(void) const
{
	/* CurrentPath pointer points to the current path in the array.
	All items before this one are 'back' paths, all items after
	this one are 'forward' paths. */
	return m_iCurrent - 1;
}

int PathManager::GetNumForwardEntriesStored(void) const
{
	/* iNumStoredPaths indexes the end of the array.
	Difference between it and the iCurrentPath index
	gives the number of 'forward' paths stored. */
	return m_nTotal - m_iCurrent;
}

std::list<PIDLIST_ABSOLUTE> PathManager::GetBackHistory() const
{
	std::list<PIDLIST_ABSOLUTE> history;

	int iStartIndex = m_iCurrent > 10 ? m_iCurrent - 10 : 0;
	int iEndIndex = m_iCurrent - 1;

	for(int i = iEndIndex - 1;i >= iStartIndex;i--)
	{
		history.push_back(ILCloneFull(ppidlList[i]));
	}

	return history;
}

std::list<PIDLIST_ABSOLUTE> PathManager::GetForwardHistory() const
{
	std::list<PIDLIST_ABSOLUTE> history;

	int iStartIndex = m_iCurrent;
	int iEndIndex = (m_nTotal - m_iCurrent) > 10 ? m_iCurrent + 10 : m_nTotal;

	for(int i = iStartIndex;i < iEndIndex;i++)
	{
		history.push_back(ILCloneFull(ppidlList[i]));
	}

	return history;
}

UINT PathManager::CreateHistoryPopupMenu(HWND Parent,POINT *Origin,BOOL bBack)
{
	HMENU	hMenu;
	TCHAR	szMenuText[MAX_PATH];
	UINT	ItemReturned;
	int		NumMenuPaths;
	int		iStartIndex;
	int		iEndIndex;
	int		i = 0;

	hMenu = CreatePopupMenu();

	if(bBack)
	{
		NumMenuPaths = GetNumBackEntriesStored();

		iEndIndex = m_iCurrent - 1;

		iStartIndex = m_iCurrent > 10 ? m_iCurrent - 10 : 0;

		for(i = iEndIndex - 1;i >= iStartIndex;i--)
		{
			GetDisplayName(ppidlList[i],szMenuText,SIZEOF_ARRAY(szMenuText),SHGDN_INFOLDER);
			AppendMenu(hMenu,MF_STRING,iEndIndex - i,szMenuText);
		}
	}
	else
	{
		NumMenuPaths = GetNumForwardEntriesStored();

		iStartIndex = m_iCurrent;

		iEndIndex = (m_nTotal - m_iCurrent) > 10 ? m_iCurrent + 10 : m_nTotal;

		for(i = iStartIndex;i < iEndIndex;i++)
		{
			GetDisplayName(ppidlList[i],szMenuText,SIZEOF_ARRAY(szMenuText),SHGDN_INFOLDER);
			AppendMenu(hMenu,MF_STRING,i - iStartIndex + 1,szMenuText);
		}
	}

	/* Show the popup menu. */
	ItemReturned = TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_RETURNCMD,Origin->x,Origin->y,0,Parent,0);

	return ItemReturned;
}