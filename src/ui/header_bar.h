// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include "base/signal.h"
#include "ui/event.h"
#include "ui/search_field.h"
#include "ui/view.h"

namespace cutefish::notes {

class HeaderBar : public ui::View {
public:
    HeaderBar();
    ~HeaderBar() override = default;

    base::Signal<> newNoteClicked;
    base::Signal<> themeToggled;
    base::Signal<std::string> searchChanged;

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;
    void pointerMove(const ui::PointerEvent& event) override;

private:
    ui::SearchField* m_searchField = nullptr;
    ui::RectF m_newNoteBtnRect;
    ui::RectF m_themeBtnRect;

    bool m_newNoteHovered = false;
    bool m_themeHovered = false;
};

} // namespace cutefish::notes
