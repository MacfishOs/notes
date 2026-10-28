// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "note_card_view.h"
#include <cmath>
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "ui/icon_loader.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace cutefish::notes {

static const char* kFontFamily = "SF Pro Text, Adwaita Sans, Cantarell, Noto Sans, sans-serif";

NoteCardView::NoteCardView(const NoteItem& item)
    : m_note(item)
{
    m_titleLabel = addChild<ui::Label>();
    m_titleLabel->setFontFamily(kFontFamily);
    m_titleLabel->setFontSize(14.0);
    m_titleLabel->setWeight(600);
    m_titleLabel->setElide(ui::Label::Elide::Right);

    m_excerptLabel = addChild<ui::Label>();
    m_excerptLabel->setFontFamily(kFontFamily);
    m_excerptLabel->setFontSize(12.0);
    m_excerptLabel->setWrap(true);
    m_excerptLabel->setMaximumLineCount(2);
    m_excerptLabel->setElide(ui::Label::Elide::Right);

    m_dateLabel = addChild<ui::Label>();
    m_dateLabel->setFontFamily(kFontFamily);
    m_dateLabel->setFontSize(10.5);

    m_todoCountLabel = addChild<ui::Label>();
    m_todoCountLabel->setFontFamily(kFontFamily);
    m_todoCountLabel->setFontSize(10.5);
    m_todoCountLabel->setWeight(600);
    m_todoCountLabel->setTextAlignment(ui::Label::TextAlignment::Right);

    updateNote(item);
}

void NoteCardView::updateNote(const NoteItem& item)
{
    m_note = item;

    std::string displayTitle = m_note.title.empty() ? "Untitled Note" : m_note.title;
    m_titleLabel->setText(displayTitle);

    std::string excerptText = m_note.excerpt(120);
    m_excerptLabel->setText(excerptText);

    m_dateLabel->setText(m_note.formattedDate());

    int totalTodos = m_note.todoCount();
    if (totalTodos > 0) {
        int doneTodos = m_note.doneTodoCount();
        m_todoCountLabel->setText("✓ " + std::to_string(doneTodos) + "/" + std::to_string(totalTodos));
        m_todoCountLabel->setVisible(true);
    } else {
        m_todoCountLabel->setVisible(false);
    }

    requestLayout();
    requestPaint();
}

void NoteCardView::layout()
{
    double w = localRect().width;
    double h = localRect().height;
    if (w <= 0 || h <= 0) return;

    double lift = m_hovered ? -3.0 : 0.0;
    // Dot is at x=16, y=18+lift. Title starts at x=32
    double rightPadding = m_note.pinned ? 54.0 : 36.0;
    m_titleLabel->setGeometry({32.0, 11.0 + lift, std::max(50.0, w - 32.0 - rightPadding), 20.0});

    int totalTodos = m_note.todoCount();
    if (totalTodos > 0) {
        m_excerptLabel->setGeometry({16.0, 35.0 + lift, std::max(50.0, w - 32.0), 38.0});
        m_todoCountLabel->setGeometry({w - 76.0, h - 39.0 + lift, 60.0, 16.0});
        m_dateLabel->setGeometry({16.0, h - 22.0 + lift, std::max(50.0, w - 32.0), 16.0});
    } else {
        m_excerptLabel->setGeometry({16.0, 38.0 + lift, std::max(50.0, w - 32.0), 42.0});
        m_dateLabel->setGeometry({16.0, h - 24.0 + lift, std::max(50.0, w - 32.0), 16.0});
    }
}

ui::RectF NoteCardView::visualRect() const
{
    double reach = 32.0;
    return localRect().adjusted(-reach, -reach - 8.0, reach, reach + 20.0);
}

ui::RectF NoteCardView::pinButtonRect() const
{
    double w = localRect().width;
    double lift = m_hovered ? -3.0 : 0.0;
    return {w - 32.0, 10.0 + lift, 22.0, 22.0};
}

ui::RectF NoteCardView::deleteButtonRect() const
{
    double w = localRect().width;
    double lift = m_hovered ? -3.0 : 0.0;
    return {w - 58.0, 10.0 + lift, 22.0, 22.0};
}

void NoteCardView::pointerEnter()
{
    m_hovered = true;
    requestLayout();
    requestPaint();
}

void NoteCardView::pointerLeave()
{
    m_hovered = false;
    m_pinHovered = false;
    m_deleteHovered = false;
    m_pressed = false;
    requestLayout();
    requestPaint();
}

void NoteCardView::pointerPress(const ui::PointerEvent& event)
{
    m_pressed = true;
    if (pinButtonRect().contains(event.position)) {
        m_pinHovered = true;
    } else if (deleteButtonRect().contains(event.position)) {
        m_deleteHovered = true;
    }
    requestPaint();
}

void NoteCardView::pointerRelease(const ui::PointerEvent& event)
{
    bool wasPressed = m_pressed;
    m_pressed = false;
    requestPaint();

    if (!wasPressed) return;

    if (pinButtonRect().contains(event.position)) {
        if (onPinClicked) onPinClicked(m_note.id);
        return;
    }

    if (m_hovered && deleteButtonRect().contains(event.position)) {
        if (onDeleteClicked) onDeleteClicked(m_note.id);
        return;
    }

    if (localRect().contains(event.position)) {
        if (onClicked) onClicked(m_note.id);
    }
}

void NoteCardView::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;
    double w = localRect().width;
    double h = localRect().height;

    // Floating lift effect on hover (-3px vertical shift)
    double lift = m_hovered ? -3.0 : 0.0;
    ui::RectF cardRect{0, lift, w, h};

    // 1. Apple-Style Dual-Stage Visible Floating Elevation Shadows
    if (dark) {
        // Dark Mode: Deep atmospheric dark shadows with high contrast against the canvas
        painter.drawShadow(cardRect, 16.0, 8.0, ui::Color::fromArgb(0x80000000), {0, 3.0});
        double farBlur = m_hovered ? 26.0 : 18.0;
        double farOffset = m_hovered ? 10.0 : 6.0;
        painter.drawShadow(cardRect, 16.0, farBlur, ui::Color::fromArgb(0x9A000000), {0, farOffset});
    } else {
        // Light Mode: Clear, soft, visible Apple card elevation shadows on white canvas
        painter.drawShadow(cardRect, 16.0, 6.0, ui::Color::fromArgb(0x22000000), {0, 2.5});
        double farBlur = m_hovered ? 28.0 : 20.0;
        double farOffset = m_hovered ? 12.0 : 7.0;
        ui::Color farColor = m_hovered ? ui::Color::fromArgb(0x32000000) : ui::Color::fromArgb(0x24000000);
        painter.drawShadow(cardRect, 16.0, farBlur, farColor, {0, farOffset});
    }

    // 2. Pure White Card Background in light mode & Crisp Minimal Border
    ui::Color bg = dark ? (m_hovered ? ui::Color::rgb(0x282933) : ui::Color::rgb(0x22232A))
                        : ui::Color::rgb(0xFFFFFF);
    ui::Color border = dark ? (m_hovered ? ui::Color::fromArgb(0x3EFFFFFF) : ui::Color::fromArgb(0x22FFFFFF))
                            : (m_hovered ? ui::Color::rgb(0xCFD3DC) : ui::Color::rgb(0xDFE2E8));
    painter.fillRoundedRect(cardRect, 16.0, bg);
    painter.strokeRoundedRect(cardRect, 16.0, border, 1);

    // 3. Pastel Color Indicator Dot
    ui::Color accent = NoteItem::accentColor(m_note.color, dark);
    painter.fillEllipse({cardRect.x + 16.0, cardRect.y + 16.0, 9.0, 9.0}, accent);

    // 4. Update child labels typography & theme colors
    m_titleLabel->setColor(dark ? ui::Color::rgb(0xF5F5F7) : ui::Color::rgb(0x1C1C1E));
    m_excerptLabel->setColor(dark ? ui::Color::rgb(0xA2A2A8) : ui::Color::rgb(0x4A4A52));
    m_dateLabel->setColor(dark ? ui::Color::rgb(0x75757C) : ui::Color::rgb(0x8E8E93));
    m_todoCountLabel->setColor(accent);

    // 5. Todo checklist progress bar (if note has todos)
    int totalTodos = m_note.todoCount();
    if (totalTodos > 0) {
        int doneTodos = m_note.doneTodoCount();
        double progress = double(doneTodos) / totalTodos;

        double barY = cardRect.y + h - 28.0;
        double barW = w - 32.0;
        double barH = 3.5;

        // Track background
        ui::RectF trackRect{cardRect.x + 16.0, barY, barW, barH};
        painter.fillRoundedRect(trackRect, 1.8, dark ? ui::Color::fromArgb(0x35FFFFFF) : ui::Color::fromArgb(0x1A000000));

        // Filled progress
        if (progress > 0) {
            ui::RectF fillRect{cardRect.x + 16.0, barY, barW * std::clamp(progress, 0.0, 1.0), barH};
            painter.fillRoundedRect(fillRect, 1.8, accent);
        }
    }

    // 6. Pin Button / Badge
    ui::RectF pRect = pinButtonRect();
    if (m_note.pinned || m_hovered) {
        if (m_note.pinned) {
            painter.fillRoundedRect(pRect, 11.0, accent.withAlpha(0.2));
            painter.strokeRoundedRect(pRect, 11.0, accent.withAlpha(0.6), 1);
        } else if (m_pinHovered) {
            painter.fillRoundedRect(pRect, 11.0, dark ? ui::Color::fromArgb(0x25FFFFFF) : ui::Color::fromArgb(0x15000000));
        }
        // Crisp vector bookmark ribbon
        double pcx = pRect.x + pRect.width / 2.0;
        double pcy = pRect.y + pRect.height / 2.0;
        ui::Path bm;
        bm.moveTo(pcx - 4.5, pcy - 5.5);
        bm.lineTo(pcx + 4.5, pcy - 5.5);
        bm.lineTo(pcx + 4.5, pcy + 5.5);
        bm.lineTo(pcx, pcy + 2.2);
        bm.lineTo(pcx - 4.5, pcy + 5.5);
        bm.close();
        if (m_note.pinned) {
            painter.fillPath(bm, accent);
        } else {
            painter.strokePath(bm, dark ? ui::Color::rgb(0x9E9EA8) : ui::Color::rgb(0x707078), 1.3, ui::LineCap::Round, ui::LineJoin::Round);
        }
    }

    // 7. Delete Button on hover
    if (m_hovered) {
        ui::RectF dRect = deleteButtonRect();
        bool delHov = m_deleteHovered;
        if (delHov) {
            painter.fillRoundedRect(dRect, 11.0, dark ? ui::Color::fromArgb(0x40FF453A) : ui::Color::rgb(0xFFEAEA));
            painter.strokeRoundedRect(dRect, 11.0, dark ? ui::Color::rgb(0xFF453A) : ui::Color::rgb(0xFFB3AD), 1);
        }
        double dcx = dRect.x + dRect.width / 2.0;
        double dcy = dRect.y + dRect.height / 2.0;
        ui::Color delCol = delHov ? ui::Color::rgb(0xFF3B30) : (dark ? ui::Color::rgb(0x9E9EA8) : ui::Color::rgb(0x707078));

        // Crisp vector trash can
        ui::Path lidHandle;
        lidHandle.moveTo(dcx - 1.8, dcy - 5.0);
        lidHandle.lineTo(dcx + 1.8, dcy - 5.0);
        painter.strokePath(lidHandle, delCol, 1.2, ui::LineCap::Round, ui::LineJoin::Round);

        ui::Path lidBar;
        lidBar.moveTo(dcx - 4.8, dcy - 3.5);
        lidBar.lineTo(dcx + 4.8, dcy - 3.5);
        painter.strokePath(lidBar, delCol, 1.3, ui::LineCap::Round, ui::LineJoin::Round);

        ui::Path canBody;
        canBody.moveTo(dcx - 3.6, dcy - 2.2);
        canBody.lineTo(dcx - 3.0, dcy + 4.5);
        canBody.quadTo(dcx - 2.8, dcy + 5.5, dcx - 1.5, dcy + 5.5);
        canBody.lineTo(dcx + 1.5, dcy + 5.5);
        canBody.quadTo(dcx + 2.8, dcy + 5.5, dcx + 3.0, dcy + 4.5);
        canBody.lineTo(dcx + 3.6, dcy - 2.2);
        canBody.close();
        painter.strokePath(canBody, delCol, 1.2, ui::LineCap::Round, ui::LineJoin::Round);
    }
}

} // namespace cutefish::notes
