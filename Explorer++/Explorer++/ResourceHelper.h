// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "Icon.h"
#include <wil/resource.h>
#include <tuple>
#include <unordered_map>

static const int DIALOG_ICON_SIZE_96DPI = 16;

using IconImageListMapping = std::unordered_map<Icon, int>;

void SetMenuItemImage(HMENU menu, UINT menuItemId, Icon icon, int dpi, std::vector<wil::unique_hbitmap> &menuImages);
std::tuple<wil::unique_himagelist, IconImageListMapping> CreateIconImageList(int iconSize, int dpi, const std::initializer_list<Icon> &icons);
void AddIconToImageList(HIMAGELIST imageList, Icon icon, int iconSize, int dpi, IconImageListMapping &imageListMappings);