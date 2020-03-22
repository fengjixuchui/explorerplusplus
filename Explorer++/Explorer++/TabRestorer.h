// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "PreservedTab.h"
#include "../Helper/Macros.h"
#include <boost/signals2.hpp>

class TabContainer;

class TabRestorer
{
public:
	TabRestorer(TabContainer *tabContainer);

	const std::vector<std::unique_ptr<PreservedTab>> &GetClosedTabs() const;
	void RestoreLastTab();
	void RestoreTabById(int id);

private:
	DISALLOW_COPY_AND_ASSIGN(TabRestorer);

	void OnTabPreRemoval(const Tab &tab);

	TabContainer *m_tabContainer;
	std::vector<boost::signals2::scoped_connection> m_connections;

	std::vector<std::unique_ptr<PreservedTab>> m_closedTabs;
};