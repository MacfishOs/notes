// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <vector>
#include "base/signal.h"
#include "model/note_item.h"
#include "ui/event.h"
#include "ui/view.h"

namespace cutefish::notes {

class FloatingToolbar : public ui::View {
public:
    FloatingToolbar();
    ~FloatingToolbar() override = default;

    void setEditorMode(bool inEditor, PastelColor currentColor = PastelColor::Pink);
    bool acceptsPointer() const override { return true; }

    base::Signal<> addNoteClicked;
    base::Signal<> backClicked;
    base::Signal<PastelColor> colorSelected;
    base::Signal<> addTodoClicked;
    base::Signal<> pinToggled;
    base::Signal<> deleteClicked;

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerMove(const ui::PointerEvent& event) override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    struct ButtonSlot {
        ui::RectF rect;
        int action = -1; // 1: Add, 2: Theme, 10: Back, 11: Pin, 12: Todo, 13: Delete, 20..25: Colors
    };

    bool m_inEditor = false;
    PastelColor m_currentColor = PastelColor::Pink;
    std::vector<ButtonSlot> m_slots;
    int m_hoveredAction = -1;
};

} // namespace cutefish::notes
