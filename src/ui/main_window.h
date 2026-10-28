// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "base/signal.h"
#include "botanical_background.h"
#include "floating_toolbar.h"
#include "note_editor_view.h"
#include "notes_grid_view.h"
#include "ui/view.h"
#include "ui/window.h"
#include "window_controls.h"

namespace cutefish::notes {

class NotesRootView : public ui::View {
public:
    NotesRootView();
    ~NotesRootView() override = default;

    void openEditor(const std::string& noteId, bool isNewNote = false);
    void closeEditor();
    void toggleTheme();
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void pointerPress(const ui::PointerEvent& event) override;

private:
    BotanicalBackgroundView* m_background = nullptr;
    WindowControls* m_windowControls = nullptr;
    NotesGridView* m_gridView = nullptr;
    NoteEditorView* m_editorView = nullptr;
    FloatingToolbar* m_floatingToolbar = nullptr;

    bool m_editorOpen = false;
    std::string m_currentNoteId;
    std::vector<base::Subscription> m_subs;
};

class MainWindow : public ui::Window {
public:
    MainWindow();
    ~MainWindow() override = default;

    void updateBackground();
    void toggleTheme();
    void openNewNoteEditor();

private:
    NotesRootView* m_rootView = nullptr;
};

} // namespace cutefish::notes
