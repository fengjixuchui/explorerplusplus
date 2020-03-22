// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "TabRestorerUI.h"
#include "CoreInterface.h"
#include "MainResource.h"
#include "ResourceHelper.h"
#include "ShellBrowser/PreservedHistoryEntry.h"
#include "../Helper/ImageHelper.h"
#include "../Helper/ShellHelper.h"
#include <boost/range/adaptor/sliced.hpp>

TabRestorerUI::TabRestorerUI(HINSTANCE instance, IExplorerplusplus *expp, TabRestorer *tabRestorer,
	int menuStartId, int menuEndId) :
	m_instance(instance),
	m_expp(expp),
	m_tabRestorer(tabRestorer),
	m_menuStartId(menuStartId),
	m_menuEndId(menuEndId)
{
	SHGetImageList(SHIL_SYSSMALL, IID_PPV_ARGS(&m_systemImageList));
	m_defaultFolderIconBitmap =
		ImageHelper::ImageListIconToBitmap(m_systemImageList.get(), GetDefaultFolderIconIndex());

	m_connections.push_back(m_expp->AddMainMenuPreShowObserver(
		boost::bind(&TabRestorerUI::OnMainMenuPreShow, this, _1)));
}

TabRestorerUI::~TabRestorerUI()
{
	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU;
	mii.hSubMenu = nullptr;
	SetMenuItemInfo(GetMenu(m_expp->GetMainWindow()), IDM_FILE_REOPEN_RECENT_TAB, FALSE, &mii);
}

void TabRestorerUI::OnMainMenuPreShow(HMENU mainMenu)
{
	std::vector<wil::unique_hbitmap> menuImages;
	std::unordered_map<int, int> menuItemMappings;
	auto recentTabsMenu = BuildRecentlyClosedTabsMenu(menuImages, menuItemMappings);

	MENUITEMINFO mii;
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_SUBMENU;
	mii.hSubMenu = recentTabsMenu.get();
	SetMenuItemInfo(mainMenu, IDM_FILE_REOPEN_RECENT_TAB, FALSE, &mii);

	m_recentTabsMenu = std::move(recentTabsMenu);
	m_menuImages = std::move(menuImages);
	m_menuItemMappings = menuItemMappings;
}

wil::unique_hmenu TabRestorerUI::BuildRecentlyClosedTabsMenu(
	std::vector<wil::unique_hbitmap> &menuImages, std::unordered_map<int, int> &menuItemMappings)
{
	wil::unique_hmenu menu(CreatePopupMenu());

	if (m_tabRestorer->GetClosedTabs().empty())
	{
		auto noRecentTabsText = ResourceHelper::LoadString(m_instance, IDS_NO_RECENT_TABS);

		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_STATE | MIIM_STRING;
		mii.fState = MFS_DISABLED;
		mii.dwTypeData = noRecentTabsText.data();
		InsertMenuItem(menu.get(), 0, TRUE, &mii);
		return menu;
	}

	int numInserted = 0;

	for (auto &closedTab : m_tabRestorer->GetClosedTabs()
			| boost::adaptors::sliced::sliced(
				0, min(MAX_MENU_ITEMS, m_tabRestorer->GetClosedTabs().size())))
	{
		auto currentEntry = closedTab->history.at(closedTab->currentEntry).get();

		std::wstring menuText = currentEntry->displayName;

		if (numInserted == 0)
		{
			// TODO: As accelerator key bindings can be customized, this should
			// be dynamically looked up.
			menuText += L"\tCtrl+Shift+T";
		}

		int id = m_menuStartId + numInserted;

		if (id >= m_menuEndId)
		{
			break;
		}

		MENUITEMINFO mii;
		mii.cbSize = sizeof(mii);
		mii.fMask = MIIM_ID | MIIM_STRING;
		mii.wID = id;
		mii.dwTypeData = menuText.data();

		HBITMAP bitmap = nullptr;
		auto iconIndex = currentEntry->systemIconIndex;

		if (iconIndex)
		{
			wil::unique_hbitmap iconBitmap =
				ImageHelper::ImageListIconToBitmap(m_systemImageList.get(), *iconIndex);

			if (iconBitmap)
			{
				bitmap = iconBitmap.get();
				menuImages.push_back(std::move(iconBitmap));
			}
		}
		else
		{
			bitmap = m_defaultFolderIconBitmap.get();
		}

		if (bitmap)
		{
			mii.fMask |= MIIM_BITMAP;
			mii.hbmpItem = bitmap;
		}

		InsertMenuItem(menu.get(), numInserted, TRUE, &mii);

		menuItemMappings.insert({ id, closedTab->id });

		numInserted++;
	}

	return menu;
}

void TabRestorerUI::OnMenuItemClicked(int menuItemId)
{
	auto itr = m_menuItemMappings.find(menuItemId);

	if (itr == m_menuItemMappings.end())
	{
		return;
	}

	m_tabRestorer->RestoreTabById(itr->second);
}