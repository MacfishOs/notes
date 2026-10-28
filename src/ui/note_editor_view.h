// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>
#include "base/signal.h"
#include "model/note_item.h"
#include "ui/event.h"
#include "ui/text_area.h"
#include "ui/text_field.h"
#include "ui/view.h"

namespace cutefish::notes {

class NoteEditorView : public ui::View {
public:
    NoteEditorView();
    ~NoteEditorView() override = default;

    void setNote(const std::string& noteId, bool isNewNote = false);
    std::string currentNoteId() const { return m_noteId; }
    void insertFormatting(const std::string& prefix, const std::string& suffix = "");
    bool acceptsPointer() const override { return true; }

    void cancelCurrent();
    void deleteCurrent();
    void saveAndDone();

    base::Signal<> closeRequested;

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;
    void pointerMove(const ui::PointerEvent& event) override;
    bool keyPress(const ui::KeyEvent& event) override;

private:
    void saveCurrent();

    std::string m_noteId;
    NoteItem m_note;
    bool m_loading = false;
    bool m_isNewNote = false;

    // Snapshot for discarding edits on cancel
    std::string m_originalTitle;
    std::string m_originalContent;
    PastelColor m_originalColor = PastelColor::Natural;
    bool m_originalPinned = false;

    ui::TextField* m_titleField = nullptr;
    ui::TextArea* m_contentArea = nullptr;

    // Palette & toolbar button bounds
    struct ButtonHit {
        ui::RectF rect;
        int action; // 0..6: colors, 10: back, 11: pin, 12: copy, 13: delete, 20: bold, 21: italic, 22: heading, 23: todo, 24: bullet
    };
    std::vector<ButtonHit> m_buttons;
    std::vector<ui::RectF> m_todoBoxes;

    int m_hoveredAction = -1;
};

} // namespace cutefish::notes
