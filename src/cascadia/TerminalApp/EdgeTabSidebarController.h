// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#pragma once

#include "EdgeTabLayoutConstants.h"

namespace winrt::TerminalApp::implementation
{
    class EdgeTabSidebarController final
    {
    public:
        using ExpandedChanged = std::function<void(bool)>;

        EdgeTabSidebarController(
            Windows::UI::Xaml::Controls::Grid host,
            Windows::UI::Xaml::Controls::Border splitter,
            HWND hostingHwnd,
            bool isOnLeft,
            bool animationsEnabled,
            ExpandedChanged expandedChanged);
        ~EdgeTabSidebarController();

        EdgeTabSidebarController(const EdgeTabSidebarController&) = delete;
        EdgeTabSidebarController& operator=(const EdgeTabSidebarController&) = delete;

        void SetVisible(bool visible);
        bool IsExpanded() const noexcept;

        // Keyboard-driven tab switching (e.g. the tab switcher chord) pins
        // the sidebar open: it expands immediately and stays expanded until
        // unpinned, regardless of where the pointer is.
        void SetPinned(bool pinned);
        void Expand();
        void Collapse();

    private:
        static constexpr double CollapsedWidth{ EdgeTabLayout::CollapsedWidth };
        static constexpr double DefaultExpandedWidth{ 240.0 };
        static constexpr double MinimumExpandedWidth{ 160.0 };
        static constexpr double MaximumExpandedWidth{ 400.0 };

        void _expand();
        void _scheduleCollapse();
        void _collapse();
        void _setExpanded(bool expanded, bool animate);
        void _animateWidth(double targetWidth, bool expanded);
        void _updatePointerPosition(Windows::Foundation::Point position);
        void _updateClip();
        void _setResizeCursor(bool enabled) const;

        Windows::UI::Xaml::Controls::Grid _host{ nullptr };
        Windows::UI::Xaml::Controls::Border _splitter{ nullptr };
        Windows::UI::Xaml::Media::Animation::Storyboard _widthStoryboard{ nullptr };
        HWND _hostingHwnd{ nullptr };
        SafeDispatcherTimer _pointerTrackingTimer;
        SafeDispatcherTimer _collapseTimer;
        ExpandedChanged _expandedChanged;

        event_token _hostSizeChangedToken{};
        event_token _splitterPointerEnteredToken{};
        event_token _splitterPointerExitedToken{};
        event_token _splitterPointerPressedToken{};
        event_token _splitterPointerMovedToken{};
        event_token _splitterPointerReleasedToken{};
        event_token _splitterPointerCaptureLostToken{};

        bool _isOnLeft{ true };
        bool _animationsEnabled{ true };
        bool _isExpanded{ false };
        bool _isVisible{ true };
        bool _pointerInside{ false };
        bool _splitterDragging{ false };
        bool _pinned{ false };
        double _expandedWidth{ DefaultExpandedWidth };
        double _splitterDragStartX{ 0.0 };
        double _splitterDragStartWidth{ 0.0 };
        uint64_t _animationGeneration{ 0 };
    };
}
