// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <string>
#include "model/note_item.h"
#include "ui/event.h"
#include "ui/label.h"
#include "ui/view.h"

namespace cutefish::notes {

class NoteCardView : public ui::View {
public:
    explicit NoteCardView(const NoteItem& item);
    ~NoteCardView() override = default;

    const NoteItem& note() const { return m_note; }
    void updateNote(const NoteItem& item);
    bool acceptsPointer() const override { return true; }
    ui::RectF visualRect() const override;

    std::function<void(const std::string& id)> onClicked;
    std::function<void(const std::string& id)> onPinClicked;
    std::function<void(const std::string& id)> onDeleteClicked;

protected:
    void layout() override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerPress(const ui::PointerEvent& event) override;
    void pointerRelease(const ui::PointerEvent& event) override;
    void paint(ui::Painter& painter) override;

private:
    ui::RectF pinButtonRect() const;
    ui::RectF deleteButtonRect() const;

    NoteItem m_note;
    bool m_hovered = false;
    bool m_pressed = false;
    bool m_pinHovered = false;
    bool m_deleteHovered = false;

    ui::Label* m_titleLabel = nullptr;
    ui::Label* m_excerptLabel = nullptr;
    ui::Label* m_dateLabel = nullptr;
    ui::Label* m_todoCountLabel = nullptr;
};

} // namespace cutefish::notes
