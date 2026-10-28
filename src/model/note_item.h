// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "base/json.h"
#include "ui/geometry.h"

namespace cutefish::notes {

enum class PastelColor {
    Natural = 0,
    Pink,
    Lavender,
    Mint,
    Butter,
    Sky,
    Peach
};

struct NoteItem {
    std::string id;
    std::string title;
    std::string content;
    std::string category = "Notes";
    PastelColor color = PastelColor::Natural;
    bool pinned = false;
    bool trashed = false;
    uint64_t createdAt = 0;
    uint64_t modifiedAt = 0;

    std::string excerpt(size_t maxChars = 140) const;
    int todoCount() const;
    int doneTodoCount() const;
    int wordCount() const;
    int charCount() const;
    std::string formattedDate() const;

    base::json::Value toJson() const;
    static NoteItem fromJson(const base::json::Value& val);

    // Color helpers
    static const char* colorName(PastelColor c);
    static ui::Color cardBgColor(PastelColor c, bool dark);
    static ui::Color cardBorderColor(PastelColor c, bool dark);
    static ui::Color accentColor(PastelColor c, bool dark);
    static ui::Color flowerColor(PastelColor c);
};

} // namespace cutefish::notes
