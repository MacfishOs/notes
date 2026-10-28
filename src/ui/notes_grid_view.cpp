// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "notes_grid_view.h"
#include <algorithm>
#include <cmath>
#include "backend/note_store.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace cutefish::notes {

NotesGridView::NotesGridView()
{
    m_subs.push_back(NoteStore::instance().notesChanged.connect([this]() {
        reload();
    }));
    reload();
}

void NotesGridView::reload()
{
    // Remove old cards
    while (!children().empty()) {
        takeChild(children().back().get());
    }
    m_cards.clear();

    const auto notes = NoteStore::instance().filteredNotes();
    for (const auto* note : notes) {
        auto card = addChild<NoteCardView>(*note);
        card->onClicked = [this](const std::string& id) {
            noteSelected.emit(id);
        };
        card->onPinClicked = [](const std::string& id) {
            NoteStore::instance().togglePin(id);
        };
        card->onDeleteClicked = [](const std::string& id) {
            NoteStore::instance().deleteNote(id);
        };
        m_cards.push_back(card);
    }

    requestLayout();
    requestPaint();
}

void NotesGridView::layout()
{
    double w = localRect().width;
    double h = localRect().height;
    if (w <= 0 || h <= 0) return;

    double paddingX = 42.0;
    double paddingTop = 58.0;
    double paddingBottom = 96.0;
    double gap = 20.0;
    double cardH = 135.0;

    double availW = w - (paddingX * 2.0);
    int cols = std::max(1, static_cast<int>((availW + gap) / (260.0 + gap)));
    double cardW = (availW - (cols - 1) * gap) / cols;

    for (size_t i = 0; i < m_cards.size(); ++i) {
        int r = static_cast<int>(i / cols);
        int c = static_cast<int>(i % cols);

        double x = paddingX + c * (cardW + gap);
        double y = paddingTop + r * (cardH + gap) - m_scrollOffset;

        m_cards[i]->setGeometry({x, y, cardW, cardH});
    }

    int totalRows = static_cast<int>((m_cards.size() + cols - 1) / cols);
    double totalContentH = paddingTop + paddingBottom + totalRows * (cardH + gap);
    m_maxScroll = std::max(0.0, totalContentH - h);

    if (m_scrollOffset > m_maxScroll) {
        m_scrollOffset = m_maxScroll;
    }
}

bool NotesGridView::scroll(const ui::ScrollEvent& event)
{
    double delta = event.deltaY;
    double newOffset = std::clamp(m_scrollOffset + delta, 0.0, m_maxScroll);
    if (newOffset != m_scrollOffset) {
        m_scrollOffset = newOffset;
        requestLayout();
        requestPaint();
        return true;
    }
    return false;
}

void NotesGridView::pointerPress(const ui::PointerEvent& event)
{
    if (m_cards.empty()) {
        double w = localRect().width;
        double h = localRect().height;
        ui::RectF btnRect{w / 2.0 - 90.0, h / 2.0 + 35.0, 180.0, 38.0};
        if (btnRect.contains(event.position)) {
            newNoteRequested.emit();
            return;
        }
    }

    // Clicking anywhere on the empty canvas/margins/gaps moves the window
    if (window() && event.button == ui::MouseButton::Left) {
        if (event.clickCount == 2) {
            window()->setMaximized(!window()->states().maximized);
            return;
        }
        window()->startMove();
    }
}

void NotesGridView::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;
    double w = localRect().width;
    double h = localRect().height;

    // Empty state when no cards
    if (m_cards.empty()) {
        ui::RectF cardRect{w / 2.0 - 160.0, h / 2.0 - 110.0, 320.0, 210.0};
        ui::Color cardBg = dark ? ui::Color::rgb(0x23232A) : ui::Color::rgb(0xFFFFFF);
        ui::Color cardBorder = dark ? ui::Color::fromArgb(0x25FFFFFF) : ui::Color::fromArgb(0x14000000);

        if (dark) {
            painter.drawShadow(cardRect, 20.0, 8.0, ui::Color::fromArgb(0x80000000), {0, 3.0});
            painter.drawShadow(cardRect, 20.0, 20.0, ui::Color::fromArgb(0x9A000000), {0, 8.0});
        } else {
            painter.drawShadow(cardRect, 20.0, 6.0, ui::Color::fromArgb(0x22000000), {0, 2.5});
            painter.drawShadow(cardRect, 20.0, 24.0, ui::Color::fromArgb(0x26000000), {0, 8.0});
        }
        painter.fillRoundedRect(cardRect, 20.0, cardBg);
        painter.strokeRoundedRect(cardRect, 20.0, cardBorder, 1);

        SkCanvas* canvas = painter.canvas();
        double s = painter.scale();
        double ox = painter.offset().x;
        double oy = painter.offset().y;
        if (canvas) {
            canvas->save();
            // Draw adorable pastel blossom in center of card
            double fx = (ox + cardRect.x + cardRect.width / 2.0) * s;
            double fy = (oy + cardRect.y + 45.0) * s;
            double r = 16.0 * s;

            SkPaint petalPaint;
            petalPaint.setAntiAlias(true);
            petalPaint.setColor((dark ? ui::Color::rgb(0xFFAEC9) : ui::Color::rgb(0xF8B6C7)).argb());
            for (int i = 0; i < 5; ++i) {
                double th = (i * 2.0 * 3.14159) / 5.0;
                canvas->drawCircle(SkDoubleToScalar(fx + std::cos(th) * r * 0.7),
                                   SkDoubleToScalar(fy + std::sin(th) * r * 0.7),
                                   SkDoubleToScalar(r * 0.6), petalPaint);
            }
            SkPaint centerPaint;
            centerPaint.setAntiAlias(true);
            centerPaint.setColor(ui::Color::rgb(0xFFE285).argb());
            canvas->drawCircle(SkDoubleToScalar(fx), SkDoubleToScalar(fy), SkDoubleToScalar(r * 0.4), centerPaint);

            // Empty state text
            SkFont titleFont(nullptr, SkDoubleToScalar(15.0 * s));
            SkPaint textPaint;
            textPaint.setAntiAlias(true);
            textPaint.setColor((dark ? ui::Color::rgb(0xF0F0F5) : ui::Color::rgb(0x1C1C1E)).argb());
            const char* msg = "No notes found";
            canvas->drawString(msg, SkDoubleToScalar(ox + cardRect.x + 105.0) * s, SkDoubleToScalar(oy + cardRect.y + 98.0) * s, titleFont, textPaint);

            SkFont subFont(nullptr, SkDoubleToScalar(12.0 * s));
            SkPaint subPaint;
            subPaint.setAntiAlias(true);
            subPaint.setColor((dark ? ui::Color::rgb(0x8E8E93) : ui::Color::rgb(0x6E6E73)).argb());
            const char* sub = "Plant a new idea in your garden";
            canvas->drawString(sub, SkDoubleToScalar(ox + cardRect.x + 65.0) * s, SkDoubleToScalar(oy + cardRect.y + 120.0) * s, subFont, subPaint);

            canvas->restore();
        }

        // "+ Create Note" pill button
        ui::RectF btnRect{cardRect.x + cardRect.width / 2.0 - 85.0, cardRect.y + 145.0, 170.0, 36.0};
        ui::Color btnColor = dark ? ui::Color::rgb(0x0A84FF) : ui::Color::rgb(0x007AFF);
        painter.fillRoundedRect(btnRect, 18.0, btnColor);

        SkCanvas* btnCanvas = painter.canvas();
        if (btnCanvas) {
            btnCanvas->save();
            SkFont btnFont(nullptr, SkDoubleToScalar(13.0 * s));
            SkPaint btnTextPaint;
            btnTextPaint.setAntiAlias(true);
            btnTextPaint.setColor(ui::Color::rgb(0xFFFFFF).argb());
            btnCanvas->drawString("+ Create Note", SkDoubleToScalar((ox + btnRect.x + 38.0) * s),
                                  SkDoubleToScalar((oy + btnRect.y + 22.0) * s), btnFont, btnTextPaint);
            btnCanvas->restore();
        }
    }
}

} // namespace cutefish::notes
