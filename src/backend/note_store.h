// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <memory>
#include <string>
#include <vector>
#include "base/signal.h"
#include "model/note_item.h"

namespace cutefish::notes {

enum class ViewFilter {
    All = 0,
    Pinned,
    Todos,
    Trash,
    ByColor
};

class NoteStore {
public:
    static NoteStore& instance();

    NoteStore();
    ~NoteStore();

    void load();
    void save();

    // Notes access
    const std::vector<NoteItem>& allNotes() const { return m_notes; }
    std::vector<const NoteItem*> filteredNotes() const;
    NoteItem* findNote(const std::string& id);
    const NoteItem* findNote(const std::string& id) const;

    // Active Selection & Filter
    std::string currentNoteId() const { return m_currentId; }
    void setCurrentNoteId(const std::string& id);

    ViewFilter activeFilter() const { return m_filter; }
    void setFilter(ViewFilter filter, PastelColor color = PastelColor::Natural);
    PastelColor filterColor() const { return m_filterColor; }

    const std::string& searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const std::string& query);

    // Operations
    std::string createNote(const std::string& title = "Untitled Note",
                           const std::string& content = "",
                           PastelColor color = PastelColor::Natural);
    void updateNoteContent(const std::string& id, const std::string& title, const std::string& content);
    void setNoteColor(const std::string& id, PastelColor color);
    void togglePin(const std::string& id);
    void deleteNote(const std::string& id);
    void permanentlyDeleteNote(const std::string& id);
    void restoreNote(const std::string& id);
    void emptyTrash();
    void toggleTodoItem(const std::string& id, size_t todoIndex);

    // Counts
    int totalCount() const;
    int pinnedCount() const;
    int todoNotesCount() const;
    int trashCount() const;
    int countByColor(PastelColor color) const;

    // Signals
    base::Signal<> notesChanged;
    base::Signal<std::string> currentNoteChanged;

private:
    void populateDefaultNotes();
    std::string storagePath() const;

    std::vector<NoteItem> m_notes;
    std::string m_currentId;
    ViewFilter m_filter = ViewFilter::All;
    PastelColor m_filterColor = PastelColor::Natural;
    std::string m_searchQuery;
};

} // namespace cutefish::notes
