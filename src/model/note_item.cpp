// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "note_item.h"
#include <ctime>
#include <sstream>

namespace cutefish::notes {

std::string NoteItem::excerpt(size_t maxChars) const
{
    if (content.empty()) {
        return "No additional text";
    }

    std::string clean;
    clean.reserve(content.size());
    bool inHeading = false;

    for (size_t i = 0; i < content.size(); ++i) {
        char ch = content[i];
        if (ch == '#' && (i == 0 || content[i - 1] == '\n')) {
            inHeading = true;
            continue;
        }
        if (inHeading) {
            if (ch == '\n') inHeading = false;
            else if (ch == '#') continue;
        }
        if (ch == '\r') continue;
        if (ch == '\n') {
            if (!clean.empty() && clean.back() != ' ') {
                clean.push_back(' ');
            }
            continue;
        }
        clean.push_back(ch);
        if (clean.size() >= maxChars) {
            clean.append("...");
            break;
        }
    }
    return clean.empty() ? "Empty note" : clean;
}

int NoteItem::todoCount() const
{
    int count = 0;
    size_t pos = 0;
    while ((pos = content.find("- [", pos)) != std::string::npos) {
        if (pos + 4 < content.size() && (content[pos + 3] == ']' || content[pos + 4] == ']')) {
            count++;
        }
        pos += 3;
    }
    return count;
}

int NoteItem::doneTodoCount() const
{
    int count = 0;
    size_t pos = 0;
    while ((pos = content.find("- [x]", pos)) != std::string::npos ||
           (pos = content.find("- [X]", pos)) != std::string::npos) {
        count++;
        pos += 5;
    }
    return count;
}

int NoteItem::wordCount() const
{
    int words = 0;
    bool inWord = false;
    for (char c : content) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            inWord = false;
        } else if (!inWord) {
            inWord = true;
            words++;
        }
    }
    return words;
}

int NoteItem::charCount() const
{
    return static_cast<int>(content.size());
}

std::string NoteItem::formattedDate() const
{
    if (modifiedAt == 0) return "";

    std::time_t t = static_cast<std::time_t>(modifiedAt);
    std::tm tmNow{};
    std::time_t now = std::time(nullptr);
    localtime_r(&now, &tmNow);

    std::tm tmMod{};
    localtime_r(&t, &tmMod);

    char buf[64];
    if (tmNow.tm_year == tmMod.tm_year && tmNow.tm_yday == tmMod.tm_yday) {
        std::strftime(buf, sizeof(buf), "Today %l:%M %p", &tmMod);
    } else if (tmNow.tm_year == tmMod.tm_year && tmNow.tm_yday == tmMod.tm_yday + 1) {
        std::strftime(buf, sizeof(buf), "Yesterday %l:%M %p", &tmMod);
    } else if (tmNow.tm_year == tmMod.tm_year) {
        std::strftime(buf, sizeof(buf), "%b %e, %l:%M %p", &tmMod);
    } else {
        std::strftime(buf, sizeof(buf), "%b %e, %Y", &tmMod);
    }
    return std::string(buf);
}

base::json::Value NoteItem::toJson() const
{
    base::json::Value obj = base::json::Value::object();
    obj.insert("id", id);
    obj.insert("title", title);
    obj.insert("content", content);
    obj.insert("category", category);
    obj.insert("color", static_cast<int>(color));
    obj.insert("pinned", pinned);
    obj.insert("trashed", trashed);
    obj.insert("createdAt", static_cast<int64_t>(createdAt));
    obj.insert("modifiedAt", static_cast<int64_t>(modifiedAt));
    return obj;
}

NoteItem NoteItem::fromJson(const base::json::Value& val)
{
    NoteItem note;
    note.id = val["id"].toString();
    note.title = val["title"].toString();
    note.content = val["content"].toString();
    note.category = val["category"].toString("Notes");
    note.color = static_cast<PastelColor>(val["color"].toInt(0));
    note.pinned = val["pinned"].toBool(false);
    note.trashed = val["trashed"].toBool(false);
    note.createdAt = static_cast<uint64_t>(val["createdAt"].toInt(0));
    note.modifiedAt = static_cast<uint64_t>(val["modifiedAt"].toInt(0));
    return note;
}

const char* NoteItem::colorName(PastelColor c)
{
    switch (c) {
    case PastelColor::Pink: return "Blossom Pink";
    case PastelColor::Lavender: return "Lilac Violet";
    case PastelColor::Mint: return "Sage Mint";
    case PastelColor::Butter: return "Buttercup";
    case PastelColor::Sky: return "Sky Blue";
    case PastelColor::Peach: return "Apricot Peach";
    default: return "Natural Glass";
    }
}

ui::Color NoteItem::cardBgColor(PastelColor c, bool dark)
{
    if (dark) {
        switch (c) {
        case PastelColor::Pink: return ui::Color::fromArgb(0xDC36222C);
        case PastelColor::Lavender: return ui::Color::fromArgb(0xDC2D223B);
        case PastelColor::Mint: return ui::Color::fromArgb(0xDC1E342B);
        case PastelColor::Butter: return ui::Color::fromArgb(0xDC362E1E);
        case PastelColor::Sky: return ui::Color::fromArgb(0xDC1C2D3B);
        case PastelColor::Peach: return ui::Color::fromArgb(0xDC37231E);
        default: return ui::Color::fromArgb(0xDD23232A);
        }
    } else {
        switch (c) {
        case PastelColor::Pink: return ui::Color::fromArgb(0xEEFFF2F6);
        case PastelColor::Lavender: return ui::Color::fromArgb(0xEEF8F3FD);
        case PastelColor::Mint: return ui::Color::fromArgb(0xEEF1FAF5);
        case PastelColor::Butter: return ui::Color::fromArgb(0xEEFFFCEF);
        case PastelColor::Sky: return ui::Color::fromArgb(0xEEF1F8FF);
        case PastelColor::Peach: return ui::Color::fromArgb(0xEEFFF4EF);
        default: return ui::Color::fromArgb(0xEEFFFFFF);
        }
    }
}

ui::Color NoteItem::cardBorderColor(PastelColor c, bool dark)
{
    if (dark) {
        switch (c) {
        case PastelColor::Pink: return ui::Color::fromArgb(0x55F7B7C8);
        case PastelColor::Lavender: return ui::Color::fromArgb(0x55D6C0E7);
        case PastelColor::Mint: return ui::Color::fromArgb(0x55B8E8D0);
        case PastelColor::Butter: return ui::Color::fromArgb(0x55FDE49E);
        case PastelColor::Sky: return ui::Color::fromArgb(0x55A8D8EA);
        case PastelColor::Peach: return ui::Color::fromArgb(0x55FFC3A0);
        default: return ui::Color::fromArgb(0x35FFFFFF);
        }
    } else {
        switch (c) {
        case PastelColor::Pink: return ui::Color::fromArgb(0x80F7B7C8);
        case PastelColor::Lavender: return ui::Color::fromArgb(0x80D6C0E7);
        case PastelColor::Mint: return ui::Color::fromArgb(0x80B8E8D0);
        case PastelColor::Butter: return ui::Color::fromArgb(0x80FDE49E);
        case PastelColor::Sky: return ui::Color::fromArgb(0x80A8D8EA);
        case PastelColor::Peach: return ui::Color::fromArgb(0x80FFC3A0);
        default: return ui::Color::fromArgb(0x28000000);
        }
    }
}

ui::Color NoteItem::accentColor(PastelColor c, bool dark)
{
    switch (c) {
    case PastelColor::Pink: return dark ? ui::Color::rgb(0xFFAEC9) : ui::Color::rgb(0xE87293);
    case PastelColor::Lavender: return dark ? ui::Color::rgb(0xD6C0E7) : ui::Color::rgb(0x9E72C3);
    case PastelColor::Mint: return dark ? ui::Color::rgb(0x9DE1BE) : ui::Color::rgb(0x3D9B6E);
    case PastelColor::Butter: return dark ? ui::Color::rgb(0xFCE38A) : ui::Color::rgb(0xD49E22);
    case PastelColor::Sky: return dark ? ui::Color::rgb(0x97D2EC) : ui::Color::rgb(0x2B8BB8);
    case PastelColor::Peach: return dark ? ui::Color::rgb(0xFFB188) : ui::Color::rgb(0xD96C3B);
    default: return dark ? ui::Color::rgb(0x0A84FF) : ui::Color::rgb(0x007AFF);
    }
}

ui::Color NoteItem::flowerColor(PastelColor c)
{
    switch (c) {
    case PastelColor::Pink: return ui::Color::rgb(0xF7B7C8);
    case PastelColor::Lavender: return ui::Color::rgb(0xD6C0E7);
    case PastelColor::Mint: return ui::Color::rgb(0xB8E8D0);
    case PastelColor::Butter: return ui::Color::rgb(0xFDE49E);
    case PastelColor::Sky: return ui::Color::rgb(0xA8D8EA);
    case PastelColor::Peach: return ui::Color::rgb(0xFFC3A0);
    default: return ui::Color::rgb(0xE2E4E9);
    }
}

} // namespace cutefish::notes
