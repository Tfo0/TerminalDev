// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

namespace winrt::TerminalApp::implementation
{
    class EdgeTitlebarContent final
    {
    public:
        using MenuRequested = std::function<void()>;

        explicit EdgeTitlebarContent(MenuRequested menuRequested);
        ~EdgeTitlebarContent();

        winrt::Windows::UI::Xaml::UIElement Root() const noexcept;
        winrt::Windows::UI::Xaml::Controls::Button MenuButton() const noexcept;
        void ShowElevationShield(bool visible);
        void Title(const winrt::hstring& title);
        void Background(const winrt::Windows::UI::Xaml::Media::Brush& brush);

    private:
        winrt::Windows::UI::Xaml::Controls::Grid _root{ nullptr };
        winrt::Windows::UI::Xaml::Controls::Button _menuButton{ nullptr };
        winrt::Windows::UI::Xaml::Controls::FontIcon _elevationShield{ nullptr };
        winrt::Windows::UI::Xaml::Controls::TextBlock _title{ nullptr };
        winrt::event_token _menuClickToken{};
    };
}
