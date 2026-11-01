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
        // Show what this tab is actually doing as the subtitle: the active
        // pane's CWD and last executed command (when shell integration is
        // available), so tabs are recognizable without relying on the title.
        if (const auto control{ winrt::get_self<winrt::TerminalApp::implementation::Tab>(tab)->GetActiveTerminalControl() })
        {
            const auto cwd{ control.CurrentWorkingDirectory() };
            winrt::hstring lastCommand;
            if (const auto context{ control.CommandHistory() })
            {
                if (const auto history{ context.History() })
                {
                    if (const auto size = history.Size(); size > 0)
                    {
                        lastCommand = history.GetAt(size - 1);
                    }
                }
            }

            std::wstring subtitle;
            if (!cwd.empty() && !lastCommand.empty())
            {
                subtitle = fmt::format(L"{} \x276F {}", cwd, lastCommand);
            }
            else if (!lastCommand.empty())
            {
                subtitle = fmt::format(L"\x276F {}", lastCommand);
            }
            else if (!cwd.empty())
            {
                subtitle = fmt::format(L"{}", cwd);
            }
            _subtitle = winrt::hstring{ subtitle };
        }
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

    WindowPaletteItem::WindowPaletteItem(uint64_t windowId, const winrt::hstring& title, const winrt::hstring& name, const winrt::hstring& contextInfo, const winrt::hstring& icon) :
        _windowId{ windowId },
        _title{ title },
        _icon{ icon }
    {
        // Display the window as "#2 [quake] · C:\path > last-command".
        std::wstring subtitle{ fmt::format(L"#{}", windowId) };
        if (!name.empty())
        {
            fmt::format_to(std::back_inserter(subtitle), L" [{}]", name);
        }
        if (!contextInfo.empty())
        {
            fmt::format_to(std::back_inserter(subtitle), L" \x00B7 {}", contextInfo);
        }
        _subtitle = winrt::hstring{ subtitle };
    }
}
