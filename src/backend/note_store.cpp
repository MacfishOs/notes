// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "note_store.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <random>
#include <sstream>

namespace fs = std::filesystem;

namespace cutefish::notes {

static std::string generateUUID()
{
    static std::random_device rd;
    static std::mt19937_64 gen(rd());
    static std::uniform_int_distribution<uint64_t> dis;

    uint64_t part1 = dis(gen);
    uint64_t part2 = dis(gen);

    std::stringstream ss;
    ss << std::hex << part1 << "-" << part2;
    return ss.str();
}

static uint64_t currentTimestamp()
{
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

static std::string toLower(std::string_view s)
{
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    }
    return result;
}

NoteStore& NoteStore::instance()
{
    static NoteStore store;
    return store;
}

NoteStore::NoteStore()
{
    load();
}

NoteStore::~NoteStore()
{
    save();
}

std::string NoteStore::storagePath() const
{
    const char* home = std::getenv("HOME");
    std::string dir = home ? std::string(home) + "/.local/share/cutefish-notes" : "/tmp/cutefish-notes";
    std::error_code ec;
    fs::create_directories(dir, ec);
    return dir + "/notes.json";
}

void NoteStore::load()
{
    std::string path = storagePath();
    if (!fs::exists(path)) {
        populateDefaultNotes();
        save();
        return;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        populateDefaultNotes();
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    auto jsonOpt = base::json::Value::parse(content);
    if (!jsonOpt || !jsonOpt->isArray()) {
        populateDefaultNotes();
        return;
    }

    m_notes.clear();
    for (const auto& itemVal : jsonOpt->toArray()) {
        m_notes.push_back(NoteItem::fromJson(itemVal));
    }

    if (m_notes.empty()) {
        populateDefaultNotes();
    }
}

void NoteStore::save()
{
    std::string path = storagePath();
    base::json::Value::Array arr;
    arr.reserve(m_notes.size());

    for (const auto& note : m_notes) {
        arr.push_back(note.toJson());
    }

    base::json::Value root(std::move(arr));
    std::string serialized = root.serialize(false);

    std::ofstream file(path);
    if (file.is_open()) {
        file << serialized;
    }
}

void NoteStore::populateDefaultNotes()
{
    m_notes.clear();
    uint64_t now = currentTimestamp();

    NoteItem n1;
    n1.id = generateUUID();
    n1.title = "🌸 Welcome to Cutefish Notes";
    n1.content = "# Botanical Aesthetics\n\n"
                 "Welcome to your personal notes garden! Cutefish Notes blends frosted acrylic glass "
                 "with graceful vines and delicate pastel flowers.\n\n"
                 "### Key Features:\n"
                 "- 🎨 **Pastel Palettes**: Choose from Blossom Pink, Lilac, Mint, Butter, Sky, or Peach.\n"
                 "- 📌 **Pin Important Notes**: Keep top priorities always in view.\n"
                 "- ☑️ **Interactive Checklists**: Seamlessly manage todos right inside your notes.\n"
                 "- 🌓 **Adaptive Themes**: Beautifully crafted for both Day & Midnight modes.\n"
                 "- 💾 **Live Auto-Save**: Your thoughts are saved instantly as you write.";
    n1.color = PastelColor::Pink;
    n1.pinned = true;
    n1.createdAt = now - 3600;
    n1.modifiedAt = now;
    m_notes.push_back(n1);

    NoteItem n2;
    n2.id = generateUUID();
    n2.title = "📋 Weekend Garden & Reading List";
    n2.content = "Tasks for this weekend:\n\n"
                 "- [x] Water the cherry blossom bonsai\n"
                 "- [x] Pick fresh lavender for the tea set\n"
                 "- [ ] Read chapter 4 of The Secret Garden\n"
                 "- [ ] Sketch botanical vine patterns\n"
                 "- [ ] Brew soothing chamomile mint tea";
    n2.color = PastelColor::Mint;
    n2.pinned = true;
    n2.createdAt = now - 7200;
    n2.modifiedAt = now - 1800;
    m_notes.push_back(n2);

    NoteItem n3;
    n3.id = generateUUID();
    n3.title = "💡 Creative Workspace Ideas";
    n3.content = "Inspirations for a serene minimal desktop:\n\n"
                 "> \"Simplicity is the ultimate sophistication.\"\n\n"
                 "1. Frosted acrylic windows with subtle drop elevation.\n"
                 "2. Soft organic pastel hues instead of harsh neon colors.\n"
                 "3. Clean typography with generous breathing space.\n"
                 "4. Ambient dark mode with bioluminescent night garden glow.";
    n3.color = PastelColor::Lavender;
    n3.pinned = false;
    n3.createdAt = now - 14400;
    n3.modifiedAt = now - 3600;
    m_notes.push_back(n3);

    NoteItem n4;
    n4.id = generateUUID();
    n4.title = "☀️ Morning Reflections";
    n4.content = "Sunshine filtering through the window this morning.\n"
                 "Started the day with warm lemon water and a ten minute walk.\n\n"
                 "Remember to pause and take deep breaths during busy moments.";
    n4.color = PastelColor::Butter;
    n4.pinned = false;
    n4.createdAt = now - 86400;
    n4.modifiedAt = now - 86400;
    m_notes.push_back(n4);

    NoteItem n5;
    n5.id = generateUUID();
    n5.title = "🌊 Coastal Travel Itinerary";
    n5.content = "- [ ] Book seaside cottage with garden view\n"
                 "- [ ] Pack watercolor sketchpad and brushes\n"
                 "- [ ] Check sunrise tides schedule\n"
                 "- [ ] Download offline maps";
    n5.color = PastelColor::Sky;
    n5.pinned = false;
    n5.createdAt = now - 172800;
    n5.modifiedAt = now - 100000;
    m_notes.push_back(n5);
}

std::vector<const NoteItem*> NoteStore::filteredNotes() const
{
    std::vector<const NoteItem*> result;
    std::string lowerQuery = toLower(m_searchQuery);

    for (const auto& note : m_notes) {
        // Trash filter
        if (m_filter == ViewFilter::Trash) {
            if (!note.trashed) continue;
        } else {
            if (note.trashed) continue;

            if (m_filter == ViewFilter::Pinned && !note.pinned) {
                continue;
            } else if (m_filter == ViewFilter::Todos && note.todoCount() == 0) {
                continue;
            } else if (m_filter == ViewFilter::ByColor && note.color != m_filterColor) {
                continue;
            }
        }

        // Search query filter
        if (!lowerQuery.empty()) {
            std::string t = toLower(note.title);
            std::string c = toLower(note.content);
            if (t.find(lowerQuery) == std::string::npos && c.find(lowerQuery) == std::string::npos) {
                continue;
            }
        }

        result.push_back(&note);
    }

    // Sort: pinned first, then by modifiedAt descending
    std::sort(result.begin(), result.end(), [](const NoteItem* a, const NoteItem* b) {
        if (a->pinned != b->pinned) {
            return a->pinned > b->pinned;
        }
        return a->modifiedAt > b->modifiedAt;
    });

    return result;
}

NoteItem* NoteStore::findNote(const std::string& id)
{
    for (auto& note : m_notes) {
        if (note.id == id) return &note;
    }
    return nullptr;
}

const NoteItem* NoteStore::findNote(const std::string& id) const
{
    for (const auto& note : m_notes) {
        if (note.id == id) return &note;
    }
    return nullptr;
}

void NoteStore::setCurrentNoteId(const std::string& id)
{
    if (m_currentId != id) {
        m_currentId = id;
        currentNoteChanged.emit(m_currentId);
    }
}

void NoteStore::setFilter(ViewFilter filter, PastelColor color)
{
    m_filter = filter;
    m_filterColor = color;
    notesChanged.emit();
}

void NoteStore::setSearchQuery(const std::string& query)
{
    m_searchQuery = query;
    notesChanged.emit();
}

std::string NoteStore::createNote(const std::string& title, const std::string& content, PastelColor color)
{
    NoteItem note;
    note.id = generateUUID();
    note.title = title;
    note.content = content;
    note.color = color;
    note.pinned = false;
    note.trashed = false;
    note.createdAt = currentTimestamp();
    note.modifiedAt = note.createdAt;

    m_notes.insert(m_notes.begin(), note);
    setCurrentNoteId(note.id);
    save();
    notesChanged.emit();
    return note.id;
}

void NoteStore::updateNoteContent(const std::string& id, const std::string& title, const std::string& content)
{
    NoteItem* note = findNote(id);
    if (!note) return;

    if (note->title != title || note->content != content) {
        note->title = title;
        note->content = content;
        note->modifiedAt = currentTimestamp();
        save();
        notesChanged.emit();
    }
}

void NoteStore::setNoteColor(const std::string& id, PastelColor color)
{
    NoteItem* note = findNote(id);
    if (!note) return;

    if (note->color != color) {
        note->color = color;
        note->modifiedAt = currentTimestamp();
        save();
        notesChanged.emit();
    }
}

void NoteStore::togglePin(const std::string& id)
{
    NoteItem* note = findNote(id);
    if (!note) return;

    note->pinned = !note->pinned;
    note->modifiedAt = currentTimestamp();
    save();
    notesChanged.emit();
}

void NoteStore::deleteNote(const std::string& id)
{
    auto it = std::find_if(m_notes.begin(), m_notes.end(), [&](const NoteItem& n) { return n.id == id; });
    if (it == m_notes.end()) return;

    if (it->trashed) {
        // Permanently delete
        m_notes.erase(it);
        if (m_currentId == id) {
            m_currentId.clear();
            currentNoteChanged.emit("");
        }
    } else {
        // Move to trash
        it->trashed = true;
        it->pinned = false;
        it->modifiedAt = currentTimestamp();
        if (m_currentId == id) {
            m_currentId.clear();
            currentNoteChanged.emit("");
        }
    }

    save();
    notesChanged.emit();
}

void NoteStore::permanentlyDeleteNote(const std::string& id)
{
    auto it = std::find_if(m_notes.begin(), m_notes.end(), [&](const NoteItem& n) { return n.id == id; });
    if (it == m_notes.end()) return;

    m_notes.erase(it);
    if (m_currentId == id) {
        m_currentId.clear();
        currentNoteChanged.emit("");
    }
    save();
    notesChanged.emit();
}

void NoteStore::restoreNote(const std::string& id)
{
    NoteItem* note = findNote(id);
    if (!note || !note->trashed) return;

    note->trashed = false;
    note->modifiedAt = currentTimestamp();
    save();
    notesChanged.emit();
}

void NoteStore::emptyTrash()
{
    m_notes.erase(std::remove_if(m_notes.begin(), m_notes.end(), [](const NoteItem& n) {
        return n.trashed;
    }), m_notes.end());

    save();
    notesChanged.emit();
}

void NoteStore::toggleTodoItem(const std::string& id, size_t todoIndex)
{
    NoteItem* note = findNote(id);
    if (!note) return;

    size_t foundCount = 0;
    size_t pos = 0;

    while (pos < note->content.size()) {
        size_t nextBox = note->content.find("- [", pos);
        if (nextBox == std::string::npos || nextBox + 4 >= note->content.size()) {
            break;
        }

        if (note->content[nextBox + 4] == ']' || note->content[nextBox + 3] == ']') {
            if (foundCount == todoIndex) {
                // Toggle between ' ' and 'x'
                if (note->content[nextBox + 3] == ' ') {
                    note->content[nextBox + 3] = 'x';
                } else {
                    note->content[nextBox + 3] = ' ';
                }
                note->modifiedAt = currentTimestamp();
                save();
                notesChanged.emit();
                return;
            }
            foundCount++;
        }
        pos = nextBox + 4;
    }
}

int NoteStore::totalCount() const
{
    int count = 0;
    for (const auto& n : m_notes) {
        if (!n.trashed) count++;
    }
    return count;
}

int NoteStore::pinnedCount() const
{
    int count = 0;
    for (const auto& n : m_notes) {
        if (!n.trashed && n.pinned) count++;
    }
    return count;
}

int NoteStore::todoNotesCount() const
{
    int count = 0;
    for (const auto& n : m_notes) {
        if (!n.trashed && n.todoCount() > 0) count++;
    }
    return count;
}

int NoteStore::trashCount() const
{
    int count = 0;
    for (const auto& n : m_notes) {
        if (n.trashed) count++;
    }
    return count;
}

int NoteStore::countByColor(PastelColor color) const
{
    int count = 0;
    for (const auto& n : m_notes) {
        if (!n.trashed && n.color == color) count++;
    }
    return count;
}

} // namespace cutefish::notes
