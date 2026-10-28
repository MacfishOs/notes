// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "header_bar.h"
#include <cmath>
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "ui/icon_loader.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace cutefish::notes {

HeaderBar::HeaderBar()
{
    m_searchField = addChild<ui::SearchField>();
    m_searchField->setPlaceholder("Search notes, tags, or todos...");
    m_searchField->onTextChanged = [this]() {
        searchChanged.emit(m_searchField->text());
    };
}

void HeaderBar::layout()
{
    double w = localRect().width;
    double h = localRect().height;
    if (w <= 0 || h <= 0) return;

    double searchW = std::clamp(w * 0.42, 220.0, 380.0);
    double searchX = std::max(120.0, (w - searchW) / 2.0);
    m_searchField->setGeometry({searchX, (h - 32.0) / 2.0, searchW, 32.0});

    // Right action buttons: "+ New Note" and Theme Toggle
    double rightPad = 18.0;
    m_themeBtnRect = {w - rightPad - 34.0, (h - 32.0) / 2.0, 34.0, 32.0};
    m_newNoteBtnRect = {m_themeBtnRect.x - 124.0, (h - 34.0) / 2.0, 114.0, 34.0};
}

void HeaderBar::pointerMove(const ui::PointerEvent& event)
{
    bool hovNew = m_newNoteBtnRect.contains(event.position);
    bool hovTheme = m_themeBtnRect.contains(event.position);

    if (hovNew != m_newNoteHovered || hovTheme != m_themeHovered) {
        m_newNoteHovered = hovNew;
        m_themeHovered = hovTheme;
        requestPaint();
    }
}

void HeaderBar::pointerPress(const ui::PointerEvent& event)
{
    if (m_newNoteBtnRect.contains(event.position)) {
        newNoteClicked.emit();
        return;
    }
    if (m_themeBtnRect.contains(event.position)) {
        themeToggled.emit();
        return;
    }
}

void HeaderBar::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;
    double w = localRect().width;
    double h = localRect().height;

    // Header translucent wash
    ui::Color headerBg = dark ? ui::Color::fromArgb(0x3514151C)
                              : ui::Color::fromArgb(0x35F6F7FA);
    painter.fillRect(localRect(), headerBg);

    // Bottom subtle divider
    ui::Color divider = dark ? ui::Color::fromArgb(0x20FFFFFF)
                             : ui::Color::fromArgb(0x15000000);
    painter.drawHorizontalLine(0, h - 1.0, w, divider);

    SkCanvas* canvas = painter.canvas();
    double s = painter.scale();

    // 1. App Title with Pastel Flower icon
    if (canvas) {
        canvas->save();
        SkFont titleFont(nullptr, SkDoubleToScalar(15.0 * s));
        SkPaint titlePaint;
        titlePaint.setAntiAlias(true);
        titlePaint.setColor((dark ? ui::Color::rgb(0xF5F5F7) : ui::Color::rgb(0x1C1C1E)).argb());

        canvas->drawString("🌸 Notes", SkDoubleToScalar(22.0 * s),
                           SkDoubleToScalar((h / 2.0 + 5.5) * s), titleFont, titlePaint);
        canvas->restore();
    }

    // 2. "+ New Note" Pill Button (Pastel Accent)
    ui::Color btnBg = dark ? (m_newNoteHovered ? ui::Color::rgb(0x2080E0) : ui::Color::rgb(0x0A84FF))
                           : (m_newNoteHovered ? ui::Color::rgb(0x0066D6) : ui::Color::rgb(0x007AFF));

    painter.drawShadow(m_newNoteBtnRect, 12.0, 8.0, btnBg.withAlpha(0.35), {0, 2.0});
    painter.fillRoundedRect(m_newNoteBtnRect, 17.0, btnBg);

    if (canvas) {
        canvas->save();
        SkFont btnFont(nullptr, SkDoubleToScalar(13.0 * s));
        SkPaint btnPaint;
        btnPaint.setAntiAlias(true);
        btnPaint.setColor(ui::Color::rgb(0xFFFFFF).argb());

        canvas->drawString("+ New Note", SkDoubleToScalar((m_newNoteBtnRect.x + 18.0) * s),
                           SkDoubleToScalar((m_newNoteBtnRect.y + 21.5) * s), btnFont, btnPaint);
        canvas->restore();
    }

    // 3. Theme Toggle Button
    ui::Color themeBg = m_themeHovered ? (dark ? ui::Color::fromArgb(0x35FFFFFF) : ui::Color::fromArgb(0x20000000))
                                       : (dark ? ui::Color::fromArgb(0x20FFFFFF) : ui::Color::fromArgb(0x12000000));
    painter.fillRoundedRect(m_themeBtnRect, 10.0, themeBg);

    if (canvas) {
        canvas->save();
        SkFont iconFont(nullptr, SkDoubleToScalar(14.0 * s));
        SkPaint iconPaint;
        iconPaint.setAntiAlias(true);
        iconPaint.setColor((dark ? ui::Color::rgb(0xFFD60A) : ui::Color::rgb(0x1C1C1E)).argb());

        const char* iconGlyph = dark ? "☀️" : "🌙";
        canvas->drawString(iconGlyph, SkDoubleToScalar((m_themeBtnRect.x + 9.0) * s),
                           SkDoubleToScalar((m_themeBtnRect.y + 21.0) * s), iconFont, iconPaint);
        canvas->restore();
    }
}

} // namespace cutefish::notes
