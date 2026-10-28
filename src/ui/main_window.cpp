// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "main_window.h"
#include <algorithm>
#include "backend/note_store.h"
#include "ui/theme.h"

namespace cutefish::notes {

NotesRootView::NotesRootView()
{
    // Layer 0: Flat watercolor painted canvas background
    m_background = addChild<BotanicalBackgroundView>();

    // Layer 1: Floating note cards grid
    m_gridView = addChild<NotesGridView>();

    // Layer 2: Floating note editor card
    m_editorView = addChild<NoteEditorView>();
    m_editorView->setVisible(false);

    // Layer 3: Left-side Cutefish window controls (Close, Minimize, Maximize)
    m_windowControls = addChild<WindowControls>();

    // Layer 4: Floating toolbar in bottom center
    m_floatingToolbar = addChild<FloatingToolbar>();

    // Wire signals
    m_subs.push_back(m_floatingToolbar->addNoteClicked.connect([this]() {
        std::string newId = NoteStore::instance().createNote("", "", PastelColor::Pink);
        openEditor(newId, true);
    }));

    m_subs.push_back(m_floatingToolbar->backClicked.connect([this]() {
        closeEditor();
    }));

    m_subs.push_back(m_floatingToolbar->colorSelected.connect([this](PastelColor pc) {
        if (!m_currentNoteId.empty()) {
            NoteStore::instance().setNoteColor(m_currentNoteId, pc);
            m_editorView->setNote(m_currentNoteId);
        }
    }));

    m_subs.push_back(m_floatingToolbar->addTodoClicked.connect([this]() {
        m_editorView->insertFormatting("- [ ] ");
    }));

    m_subs.push_back(m_floatingToolbar->pinToggled.connect([this]() {
        if (!m_currentNoteId.empty()) {
            NoteStore::instance().togglePin(m_currentNoteId);
            m_editorView->setNote(m_currentNoteId);
        }
    }));

    m_subs.push_back(m_floatingToolbar->deleteClicked.connect([this]() {
        if (!m_currentNoteId.empty()) {
            NoteStore::instance().deleteNote(m_currentNoteId);
            closeEditor();
        }
    }));

    m_subs.push_back(m_gridView->noteSelected.connect([this](const std::string& id) {
        openEditor(id, false);
    }));

    m_subs.push_back(m_gridView->newNoteRequested.connect([this]() {
        std::string newId = NoteStore::instance().createNote("", "", PastelColor::Lavender);
        openEditor(newId, true);
    }));

    m_subs.push_back(m_editorView->closeRequested.connect([this]() {
        closeEditor();
    }));
}

void NotesRootView::openEditor(const std::string& noteId, bool isNewNote)
{
    m_currentNoteId = noteId;
    m_editorOpen = true;
    m_editorView->setNote(noteId, isNewNote);
    m_editorView->setVisible(true);

    const NoteItem* note = NoteStore::instance().findNote(noteId);
    m_floatingToolbar->setEditorMode(true, note ? note->color : PastelColor::Pink);

    requestLayout();
    requestPaint();
}

void NotesRootView::closeEditor()
{
    m_editorOpen = false;
    m_currentNoteId.clear();
    m_editorView->setVisible(false);
    m_floatingToolbar->setEditorMode(false);
    m_gridView->reload();

    requestLayout();
    requestPaint();
}

void NotesRootView::toggleTheme()
{
    if (auto* win = dynamic_cast<MainWindow*>(window())) {
        win->toggleTheme();
    }
}

void NotesRootView::pointerPress(const ui::PointerEvent& event)
{
    if (m_windowControls && m_windowControls->geometry().contains(event.position)) {
        return;
    }
    if (m_floatingToolbar && m_floatingToolbar->geometry().contains(event.position)) {
        return;
    }
    if (m_editorOpen && m_editorView && m_editorView->geometry().contains(event.position)) {
        return;
    }
    // If user clicked directly on canvas background, allow interactive dragging of window
    if (window() && event.button == ui::MouseButton::Left) {
        if (event.clickCount == 2) {
            window()->setMaximized(!window()->states().maximized);
            return;
        }
        window()->startMove();
    }
}

void NotesRootView::layout()
{
    double w = localRect().width;
    double h = localRect().height;
    if (w <= 0 || h <= 0) return;

    // Background covers entire window canvas
    m_background->setGeometry(localRect());

    // Left-side Cutefish window controls (Close, Minimize, Maximize)
    m_windowControls->setGeometry({18.0, 16.0, 66.0, 20.0});

    if (m_editorOpen) {
        m_gridView->setVisible(false);
        m_floatingToolbar->setVisible(false);

        // Centered floating card editor with graceful botanical margins
        double edW = std::clamp(w - 72.0, 400.0, 860.0);
        double edH = h - 64.0;
        double edX = (w - edW) / 2.0;
        double edY = 46.0;

        m_editorView->setGeometry({edX, edY, edW, edH});
        m_editorView->setVisible(true);
    } else {
        m_editorView->setVisible(false);

        // Floating note cards grid covers the canvas
        m_gridView->setGeometry(localRect());
        m_gridView->setVisible(true);

        // Floating Apple-style circular Add Note button centered at bottom
        double tbW = 80.0;
        double tbH = 80.0;
        double tbX = (w - tbW) / 2.0;
        double tbY = h - tbH - 18.0;
        m_floatingToolbar->setGeometry({tbX, tbY, tbW, tbH});
        m_floatingToolbar->setVisible(true);
    }
}

// ============================================================================
// MainWindow
// ============================================================================

MainWindow::MainWindow()
{
    setTitle("Notes");
    setMinimumSize({720, 480});
    resize({1040, 680});

    updateBackground();

    auto root = std::make_unique<NotesRootView>();
    m_rootView = root.get();
    setContentView(std::move(root));
}

void MainWindow::updateBackground()
{
    bool dark = ui::Theme::current().darkMode;

    // Pure flat canvas: White in light mode, dark slate in dark mode
    ui::Color winBg = dark ? ui::Color::rgb(0x18181C) : ui::Color::rgb(0xFFFFFF);
    setBackgroundColor(winBg);

    // No blur, no glass - pure flat canvas as requested
    setBackgroundBlur({}, 0.0);
    setBackgroundGlass({});
}

void MainWindow::toggleTheme()
{
    bool dark = ui::Theme::current().darkMode;
    ui::Theme::setCurrent(dark ? ui::Theme::light() : ui::Theme::dark());
    updateBackground();
    requestPaint();
}

void MainWindow::openNewNoteEditor()
{
    std::string newId = NoteStore::instance().createNote("", "", PastelColor::Pink);
    m_rootView->openEditor(newId, true);
}

} // namespace cutefish::notes
