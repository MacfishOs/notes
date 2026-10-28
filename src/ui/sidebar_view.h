// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <vector>
#include "backend/note_store.h"
#include "base/signal.h"
#include "ui/event.h"
#include "ui/view.h"
#include "window_controls.h"

namespace cutefish::notes {

struct SidebarItem {
    std::string title;
    std::string icon;
    ViewFilter filter;
    PastelColor color = PastelColor::Natural;
    bool isColor = false;
    bool isHeader = false;
    ui::RectF rect;
};

class SidebarView : public ui::View {
public:
    SidebarView();
    ~SidebarView() override = default;

    base::Signal<> themeToggled;

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;
    void pointerMove(const ui::PointerEvent& event) override;

private:
    WindowControls* m_windowControls = nullptr;
    std::vector<SidebarItem> m_items;
    size_t m_selectedIndex = 0;
    int m_hoveredIndex = -1;
    ui::RectF m_themeToggleRect;
    std::vector<base::Subscription> m_subs;
};

} // namespace cutefish::notes
