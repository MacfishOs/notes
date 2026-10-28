// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <memory>
#include <vector>
#include "base/signal.h"
#include "note_card_view.h"
#include "ui/event.h"
#include "ui/view.h"

namespace cutefish::notes {

class NotesGridView : public ui::View {
public:
    NotesGridView();
    ~NotesGridView() override = default;

    void reload();
    bool acceptsPointer() const override { return true; }

    base::Signal<std::string> noteSelected;
    base::Signal<> newNoteRequested;

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    bool scroll(const ui::ScrollEvent& event) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    double m_scrollOffset = 0;
    double m_maxScroll = 0;
    std::vector<NoteCardView*> m_cards;
    std::vector<base::Subscription> m_subs;
};

} // namespace cutefish::notes
