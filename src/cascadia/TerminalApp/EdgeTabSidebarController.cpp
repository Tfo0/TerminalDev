// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

#include "pch.h"
#include "EdgeTabSidebarController.h"

using namespace std::chrono_literals;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Core;
using namespace winrt::Windows::UI::Xaml;

namespace winrt::TerminalApp::implementation
{
    EdgeTabSidebarController::EdgeTabSidebarController(
        Controls::Grid host,
        Controls::Border splitter,
        const HWND hostingHwnd,
        const bool isOnLeft,
        const bool animationsEnabled,
        ExpandedChanged expandedChanged) :
        _host{ std::move(host) },
        _splitter{ std::move(splitter) },
        _hostingHwnd{ hostingHwnd },
        _expandedChanged{ std::move(expandedChanged) },
        _isOnLeft{ isOnLeft },
        _animationsEnabled{ animationsEnabled }
    {
        _host.Width(CollapsedWidth);
        _splitter.Opacity(0.0);
        _splitter.IsHitTestVisible(false);

        _collapseTimer.Interval(350ms);
        _collapseTimer.Tick([this](const auto&, const auto&) {
            _collapseTimer.Stop();
            if (!_splitterDragging && !_pointerInside && !_pinned)
            {
                _collapse();
            }
        });

        _hostSizeChangedToken = _host.SizeChanged([this](const auto&, const auto&) {
            _updateClip();
        });

        if (_hostingHwnd)
        {
            _pointerTrackingTimer.Interval(16ms);
            _pointerTrackingTimer.Tick([this](const auto&, const auto&) {
                POINT position{};
                if (!GetCursorPos(&position) || !ScreenToClient(_hostingHwnd, &position))
                {
                    return;
                }

                const auto xamlRoot = _host.XamlRoot();
                const auto scale = xamlRoot ? static_cast<float>(xamlRoot.RasterizationScale()) : 1.0f;
                _updatePointerPosition({ position.x / scale, position.y / scale });
            });
            _pointerTrackingTimer.Start();
        }

        _splitterPointerEnteredToken = _splitter.PointerEntered([this](const auto&, const auto&) {
            _setResizeCursor(_isExpanded);
        });
        _splitterPointerExitedToken = _splitter.PointerExited([this](const auto&, const auto&) {
            if (!_splitterDragging)
            {
                _setResizeCursor(false);
            }
        });
        _splitterPointerPressedToken = _splitter.PointerPressed([this](const auto& sender, const Input::PointerRoutedEventArgs& args) {
            if (!_isExpanded)
            {
                return;
            }

            const auto border = sender.as<Controls::Border>();
            border.CapturePointer(args.Pointer());
            _collapseTimer.Stop();
            _splitterDragging = true;
            _splitterDragStartX = args.GetCurrentPoint(_host).Position().X;
            _splitterDragStartWidth = _host.ActualWidth();
            args.Handled(true);
        });
        _splitterPointerMovedToken = _splitter.PointerMoved([this](const auto&, const Input::PointerRoutedEventArgs& args) {
            if (!_splitterDragging)
            {
                return;
            }

            const auto currentX = args.GetCurrentPoint(_host).Position().X;
            const auto delta = currentX - _splitterDragStartX;
            const auto proposedWidth = _isOnLeft ? _splitterDragStartWidth + delta : _splitterDragStartWidth - delta;
            _expandedWidth = std::clamp(proposedWidth, MinimumExpandedWidth, MaximumExpandedWidth);
            _host.Width(_expandedWidth);
            args.Handled(true);
        });
        _splitterPointerReleasedToken = _splitter.PointerReleased([this](const auto& sender, const Input::PointerRoutedEventArgs& args) {
            if (!_splitterDragging)
            {
                return;
            }

            _splitterDragging = false;
            sender.as<Controls::Border>().ReleasePointerCapture(args.Pointer());
            args.Handled(true);
            if (!_pointerInside)
            {
                _scheduleCollapse();
            }
        });
        _splitterPointerCaptureLostToken = _splitter.PointerCaptureLost([this](const auto&, const auto&) {
            _splitterDragging = false;
            _setResizeCursor(false);
            if (!_pointerInside)
            {
                _scheduleCollapse();
            }
        });

        _updateClip();
        if (_expandedChanged)
        {
            _expandedChanged(false);
        }
    }

    EdgeTabSidebarController::~EdgeTabSidebarController()
    {
        _pointerTrackingTimer.Destroy();
        _collapseTimer.Destroy();
        if (_widthStoryboard)
        {
            _widthStoryboard.Stop();
        }

        if (_host)
        {
            _host.SizeChanged(_hostSizeChangedToken);
        }
        if (_splitter)
        {
            _splitter.PointerEntered(_splitterPointerEnteredToken);
            _splitter.PointerExited(_splitterPointerExitedToken);
            _splitter.PointerPressed(_splitterPointerPressedToken);
            _splitter.PointerMoved(_splitterPointerMovedToken);
            _splitter.PointerReleased(_splitterPointerReleasedToken);
            _splitter.PointerCaptureLost(_splitterPointerCaptureLostToken);
        }
    }

    void EdgeTabSidebarController::SetVisible(const bool visible)
    {
        if (_isVisible == visible)
        {
            return;
        }

        _isVisible = visible;
        _collapseTimer.Stop();
        if (_hostingHwnd)
        {
            if (visible)
            {
                _pointerTrackingTimer.Start();
            }
            else
            {
                _pointerTrackingTimer.Stop();
                _pointerInside = false;
            }
        }
        _host.Visibility(visible ? Visibility::Visible : Visibility::Collapsed);
        if (visible)
        {
            _setExpanded(false, false);
        }
    }

    bool EdgeTabSidebarController::IsExpanded() const noexcept
    {
        return _isExpanded;
    }

    void EdgeTabSidebarController::SetPinned(const bool pinned)
    {
        if (_pinned == pinned)
        {
            return;
        }

        _pinned = pinned;
        if (pinned)
        {
            _collapseTimer.Stop();
        }
        else if (_isExpanded && !_pointerInside)
        {
            _scheduleCollapse();
        }
    }

    void EdgeTabSidebarController::Expand()
    {
        _expand();
    }

    void EdgeTabSidebarController::Collapse()
    {
        _setExpanded(false, false);
    }

    void EdgeTabSidebarController::_expand()
    {
        if (_isVisible)
        {
            _setExpanded(true, true);
        }
    }

    void EdgeTabSidebarController::_scheduleCollapse()
    {
        if (_isVisible && !_splitterDragging)
        {
            _collapseTimer.Stop();
            _collapseTimer.Start();
        }
    }

    void EdgeTabSidebarController::_collapse()
    {
        _setExpanded(false, true);
    }

    void EdgeTabSidebarController::_setExpanded(const bool expanded, const bool animate)
    {
        if (_isExpanded == expanded && animate)
        {
            return;
        }

        _isExpanded = expanded;
        _splitter.Opacity(expanded ? 1.0 : 0.0);
        _splitter.IsHitTestVisible(expanded);
        if (_expandedChanged)
        {
            _expandedChanged(expanded);
        }

        const auto targetWidth = expanded ? _expandedWidth : CollapsedWidth;
        if (animate && _animationsEnabled)
        {
            _animateWidth(targetWidth, expanded);
        }
        else
        {
            ++_animationGeneration;
            if (_widthStoryboard)
            {
                _widthStoryboard.Stop();
                _widthStoryboard = nullptr;
            }
            _host.Width(targetWidth);
        }
    }

    void EdgeTabSidebarController::_animateWidth(const double targetWidth, const bool expanded)
    {
        const auto generation = ++_animationGeneration;
        if (_widthStoryboard)
        {
            _widthStoryboard.Stop();
        }

        Media::Animation::DoubleAnimation animation{};
        animation.Duration(DurationHelper::FromTimeSpan(TimeSpan{ 180ms }));
        animation.From(_host.ActualWidth());
        animation.To(targetWidth);
        animation.EasingFunction(Media::Animation::QuadraticEase{});
        animation.EnableDependentAnimation(true);

        _widthStoryboard = Media::Animation::Storyboard{};
        _widthStoryboard.Duration(animation.Duration());
        _widthStoryboard.Children().Append(animation);
        _widthStoryboard.SetTarget(animation, _host);
        _widthStoryboard.SetTargetProperty(animation, L"Width");

        animation.Completed([this, generation, expanded](const auto&, const auto&) {
            if (generation != _animationGeneration)
            {
                return;
            }

            _host.Width(expanded ? _expandedWidth : CollapsedWidth);
        });
        _widthStoryboard.Begin();
    }

    void EdgeTabSidebarController::_updatePointerPosition(const Point position)
    {
        if (!_isVisible || !_host)
        {
            return;
        }

        const auto origin = _host.TransformToVisual(nullptr).TransformPoint({});
        const auto inside = position.X >= origin.X &&
                            position.X < origin.X + _host.ActualWidth() &&
                            position.Y >= origin.Y &&
                            position.Y < origin.Y + _host.ActualHeight();

        if (inside == _pointerInside)
        {
            return;
        }

        _pointerInside = inside;
        if (inside)
        {
            _collapseTimer.Stop();
            _expand();
        }
        else if (!_pinned)
        {
            _scheduleCollapse();
        }
    }

    void EdgeTabSidebarController::_updateClip()
    {
        if (!_host)
        {
            return;
        }

        Media::RectangleGeometry clip{};
        clip.Rect(Rect{ 0.0f, 0.0f, static_cast<float>(_host.ActualWidth()), static_cast<float>(_host.ActualHeight()) });
        _host.Clip(clip);
    }

    void EdgeTabSidebarController::_setResizeCursor(const bool enabled) const
    {
        if (const auto coreWindow = CoreWindow::GetForCurrentThread())
        {
            coreWindow.PointerCursor(CoreCursor{ enabled ? CoreCursorType::SizeWestEast : CoreCursorType::Arrow, 0 });
        }
    }
}
