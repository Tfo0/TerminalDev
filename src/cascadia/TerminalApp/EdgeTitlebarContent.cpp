// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "EdgeTabLayoutConstants.h"
#include "EdgeTitlebarContent.h"

using namespace winrt;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Controls;

namespace winrt::TerminalApp::implementation
{
    EdgeTitlebarContent::EdgeTitlebarContent(MenuRequested menuRequested)
    {
        _root = Grid{};
        _root.Height(EdgeTabLayout::TitlebarHeight);
        _root.MinWidth(EdgeTabLayout::CollapsedWidth);

        if (const auto resources = Application::Current().Resources())
        {
            if (const auto value = resources.Lookup(box_value(L"TabViewBackground")))
            {
                if (const auto brush = value.try_as<winrt::Windows::UI::Xaml::Media::Brush>())
                {
                    _root.Background(brush);
                }
            }
        }

        ColumnDefinition menuColumn{};
        menuColumn.Width(GridLengthHelper::FromPixels(EdgeTabLayout::CollapsedWidth));
        ColumnDefinition shieldColumn{};
        shieldColumn.Width(GridLengthHelper::Auto());
        ColumnDefinition titleColumn{};
        titleColumn.Width(GridLengthHelper::Auto());
        _root.ColumnDefinitions().Append(menuColumn);
        _root.ColumnDefinitions().Append(shieldColumn);
        _root.ColumnDefinitions().Append(titleColumn);

        _menuButton = Button{};
        _menuButton.Width(EdgeTabLayout::CollapsedWidth);
        _menuButton.MinWidth(EdgeTabLayout::CollapsedWidth);
        _menuButton.Height(EdgeTabLayout::TitlebarHeight);
        _menuButton.Padding({ 0, 0, 0, 0 });
        _menuButton.BorderThickness({ 0, 0, 0, 0 });
        _menuButton.Background(Media::SolidColorBrush{ Windows::UI::Colors::Transparent() });
        _menuButton.HorizontalContentAlignment(HorizontalAlignment::Center);
        _menuButton.VerticalContentAlignment(VerticalAlignment::Center);

        FontIcon menuIcon{};
        menuIcon.FontFamily(Media::FontFamily{ L"Segoe Fluent Icons, Segoe MDL2 Assets" });
        menuIcon.FontSize(10.0);
        menuIcon.Glyph(L"\xE70D");
        _menuButton.Content(menuIcon);

        const auto accessibleName = RS_(L"TabNewButtonText");
        Automation::AutomationProperties::SetName(_menuButton, accessibleName);
        ToolTipService::SetToolTip(_menuButton, box_value(accessibleName));

        _menuClickToken = _menuButton.Click([menuRequested = std::move(menuRequested)](const auto&, const auto&) {
            menuRequested();
        });

        _elevationShield = FontIcon{};
        _elevationShield.Margin({ 10, 0, 0, 0 });
        _elevationShield.FontFamily(Media::FontFamily{ L"Segoe Fluent Icons, Segoe MDL2 Assets" });
        _elevationShield.FontSize(16.0);
        _elevationShield.Glyph(L"\xEA18");
        _elevationShield.Opacity(0.72);
        _elevationShield.IsHitTestVisible(false);
        _elevationShield.Visibility(Visibility::Collapsed);
        Automation::AutomationProperties::SetAccessibilityView(_elevationShield, Automation::Peers::AccessibilityView::Raw);

        _title = TextBlock{};
        _title.Margin({ 8, 0, 12, 0 });
        _title.MaxWidth(560.0);
        _title.VerticalAlignment(VerticalAlignment::Center);
        _title.FontSize(12.0);
        _title.TextTrimming(TextTrimming::CharacterEllipsis);
        _title.IsHitTestVisible(false);

        Grid::SetColumn(_menuButton, 0);
        Grid::SetColumn(_elevationShield, 1);
        Grid::SetColumn(_title, 2);
        _root.Children().Append(_menuButton);
        _root.Children().Append(_elevationShield);
        _root.Children().Append(_title);
    }

    EdgeTitlebarContent::~EdgeTitlebarContent()
    {
        if (_menuButton && _menuClickToken)
        {
            _menuButton.Click(_menuClickToken);
        }
    }

    UIElement EdgeTitlebarContent::Root() const noexcept
    {
        return _root;
    }

    Button EdgeTitlebarContent::MenuButton() const noexcept
    {
        return _menuButton;
    }

    void EdgeTitlebarContent::ShowElevationShield(const bool visible)
    {
        _elevationShield.Visibility(visible ? Visibility::Visible : Visibility::Collapsed);
    }

    void EdgeTitlebarContent::Title(const hstring& title)
    {
        _title.Text(title);
        Automation::AutomationProperties::SetName(_title, title);
    }

    void EdgeTitlebarContent::Background(const winrt::Windows::UI::Xaml::Media::Brush& brush)
    {
        _root.Background(brush);
    }
}
