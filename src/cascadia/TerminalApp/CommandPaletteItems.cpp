// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "Tab.h"

#include "CommandPaletteItems.h"
#include "WindowPaletteItem.g.cpp"

using namespace winrt;
using namespace winrt::TerminalApp;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::System;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Microsoft::Terminal::Settings::Model;

namespace winrt::TerminalApp::implementation
{
    TabPaletteItem::TabPaletteItem(const winrt::TerminalApp::Tab& tab) :
        _tab{ tab }
    {
        _tabChangedRevoker = tab.PropertyChanged(winrt::auto_revoke, [=](auto& sender, auto& e) {
            if (auto senderTab{ sender.try_as<winrt::TerminalApp::Tab>() })
            {
                auto changedProperty = e.PropertyName();
                if (changedProperty == L"Title")
                {
                    BaseRaisePropertyChanged(L"Name");
                }
                else if (changedProperty == L"Icon")
                {
                    BaseRaisePropertyChanged(L"Icon");
                    InvalidateResolvedIcon();
                }
            }
        });

        if (const auto status = tab.TabStatus())
        {
            _tabStatusChangedRevoker = status.PropertyChanged(winrt::auto_revoke, [=](auto& /*sender*/, auto& /*e*/) {
                // Sometimes nested bindings do not get updated,
                // thus let's notify property changed on TabStatus when one of its properties changes
                BaseRaisePropertyChanged(L"TabStatus");
            });
        }
    }

    WindowPaletteItem::WindowPaletteItem(uint64_t windowId, const winrt::hstring& title, const winrt::hstring& name) :
        _windowId{ windowId },
        _title{ title },
        _icon{ L"" } // TaskView glyph
    {
        // Display the window as "#2" or "#2 [quake]", matching the tray icon menu.
        std::wstring subtitle{ fmt::format(L"#{}", windowId) };
        if (!name.empty())
        {
            fmt::format_to(std::back_inserter(subtitle), L" [{}]", name);
        }
        _subtitle = winrt::hstring{ subtitle };
    }
}
