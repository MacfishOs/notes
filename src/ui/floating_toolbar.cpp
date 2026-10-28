// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "floating_toolbar.h"
#include <cmath>
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "ui/icon_loader.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace cutefish::notes {

FloatingToolbar::FloatingToolbar()
{
}

void FloatingToolbar::setEditorMode(bool inEditor, PastelColor currentColor)
{
    m_inEditor = inEditor;
    m_currentColor = currentColor;
    requestLayout();
    requestPaint();
}

void FloatingToolbar::layout()
{
    m_slots.clear();
    double w = localRect().width;
    double h = localRect().height;
    if (w <= 0 || h <= 0) return;

    if (!m_inEditor) {
        // Normal Mode:
        // Pure Apple-Style White Circular "+ Add Note" Hero Button (58x58)
        double addBtnSize = 58.0;
        double startX = (w - addBtnSize) / 2.0;
        double startY = (h - addBtnSize) / 2.0;

        m_slots.push_back({{startX, startY, addBtnSize, addBtnSize}, 1}); // Hero Add Button
    } else {
        // Editor Mode:
        // [ ← Back ] [ 6 Flower Colors ] [ ☑ Todo ] [ 📌 Pin ] [ 🗑 Delete ]
        double pad = 6.0;
        double btnH = h - (pad * 2.0);

        double curX = pad;
        m_slots.push_back({{curX, pad, 36.0, btnH}, 10});
        curX += 36.0 + 8.0;

        for (int c = 0; c < 6; ++c) {
            m_slots.push_back({{curX, (h - 22.0) / 2.0, 22.0, 22.0}, 20 + c});
            curX += 26.0;
        }
        curX += 8.0;

        m_slots.push_back({{curX, pad, 36.0, btnH}, 12});
        curX += 36.0 + 4.0;

        m_slots.push_back({{curX, pad, 36.0, btnH}, 11});
        curX += 36.0 + 4.0;

        m_slots.push_back({{curX, pad, 36.0, btnH}, 13});
        curX += 36.0 + 8.0;

        // Circular Check Save Button (14)
        m_slots.push_back({{curX, pad + (btnH - 32.0) / 2.0, 32.0, 32.0}, 14});
    }
}

void FloatingToolbar::pointerEnter()
{
    requestPaint();
}

void FloatingToolbar::pointerLeave()
{
    m_hoveredAction = -1;
    requestPaint();
}

void FloatingToolbar::pointerMove(const ui::PointerEvent& event)
{
    int oldHover = m_hoveredAction;
    m_hoveredAction = -1;

    for (const auto& slot : m_slots) {
        if (slot.rect.contains(event.position)) {
            m_hoveredAction = slot.action;
            break;
        }
    }

    if (oldHover != m_hoveredAction) {
        requestPaint();
    }
}

void FloatingToolbar::pointerPress(const ui::PointerEvent& event)
{
    for (const auto& slot : m_slots) {
        if (slot.rect.contains(event.position)) {
            if (slot.action == 1) {
                addNoteClicked.emit();
                return;
            } else if (slot.action == 10) {
                backClicked.emit();
                return;
            } else if (slot.action >= 20 && slot.action <= 25) {
                PastelColor pc = static_cast<PastelColor>(slot.action - 20);
                m_currentColor = pc;
                colorSelected.emit(pc);
                requestPaint();
                return;
            } else if (slot.action == 11) {
                pinToggled.emit();
                return;
            } else if (slot.action == 12) {
                addTodoClicked.emit();
                return;
            } else if (slot.action == 13) {
                deleteClicked.emit();
                return;
            } else if (slot.action == 14) {
                backClicked.emit();
                return;
            }
        }
    }
}

void FloatingToolbar::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;
    double w = localRect().width;
    double h = localRect().height;

    SkCanvas* canvas = painter.canvas();
    double s = painter.scale();

    if (!m_inEditor) {
        // =====================================================================
        // NORMAL MODE: White Circular Apple-Style Floating Add Button
        // =====================================================================
        for (const auto& slot : m_slots) {
            bool hovered = (m_hoveredAction == slot.action);

            if (slot.action == 1) {
                // Circular White Add Button (Hero Action)
                double lift = hovered ? -3.5 : 0.0;
                ui::RectF circleRect{slot.rect.x, slot.rect.y + lift, slot.rect.width, slot.rect.height};
                double r = circleRect.width / 2.0;

                // 1. Dual-Layer Apple-Style High-Visibility Elevation Shadows
                ui::Color shadowColor = dark ? ui::Color::fromArgb(0xA0000000)
                                             : ui::Color::fromArgb(0x35000000);
                double shadowBlur = hovered ? 26.0 : 18.0;
                ui::PointF shadowOffset{0, hovered ? 9.0 : 6.0};
                painter.drawShadow(circleRect, r, shadowBlur, shadowColor, shadowOffset);

                // Near sharp ambient shadow
                painter.drawShadow(circleRect, r, 6.0,
                                   dark ? ui::Color::fromArgb(0x50000000) : ui::Color::fromArgb(0x18000000),
                                   {0, 2.0});

                // 2. Crisp White Circular Plate with high contrast border (Apple Floating Hero Button)
                ui::Color plateBg = hovered ? ui::Color::rgb(0xF6F7FB) : ui::Color::rgb(0xFFFFFF);
                ui::Color plateBorder = dark ? ui::Color::fromArgb(0x35000000) : ui::Color::rgb(0xCCD0DA);
                painter.fillRoundedRect(circleRect, r, plateBg);
                painter.strokeRoundedRect(circleRect, r, plateBorder, 1.5);

                // 3. Apple-Style Clean "+" Icon
                ui::Color plusColor = hovered ? ui::Color::rgb(0x007AFF) : ui::Color::rgb(0x1C1C1E);

                double cx = circleRect.x + r;
                double cy = circleRect.y + r;
                double arm = 10.5;

                ui::Path plusPath;
                plusPath.moveTo(cx - arm, cy);
                plusPath.lineTo(cx + arm, cy);
                plusPath.moveTo(cx, cy - arm);
                plusPath.lineTo(cx, cy + arm);
                painter.strokePath(plusPath, plusColor, 3.0, ui::LineCap::Round, ui::LineJoin::Round);
            }
        }
    } else {
        // =====================================================================
        // EDITOR MODE: Floating White Island Pill Toolbar
        // =====================================================================
        ui::RectF barRect{0, 0, w, h};
        double r = h / 2.0;

        ui::Color shadowColor = dark ? ui::Color::fromArgb(0x90000000)
                                     : ui::Color::fromArgb(0x2C000000);
        painter.drawShadow(barRect, r, 20.0, shadowColor, {0, 7.0});
        painter.drawShadow(barRect, r, 6.0, dark ? ui::Color::fromArgb(0x40000000) : ui::Color::fromArgb(0x14000000), {0, 2.0});

        ui::Color barBg = dark ? ui::Color::rgb(0x24242A) : ui::Color::rgb(0xFFFFFF);
        ui::Color barBorder = dark ? ui::Color::fromArgb(0x35FFFFFF) : ui::Color::rgb(0xCCD0DA);
        painter.fillRoundedRect(barRect, r, barBg);
        painter.strokeRoundedRect(barRect, r, barBorder, 1.5);

        for (const auto& slot : m_slots) {
            bool hovered = (m_hoveredAction == slot.action);

            if (slot.action == 10) {
                // Back Button (←)
                if (hovered) {
                    painter.fillRoundedRect(slot.rect, slot.rect.height / 2.0,
                        dark ? ui::Color::fromArgb(0x28FFFFFF) : ui::Color::fromArgb(0x18000000));
                }
                ui::Color iconCol = dark ? ui::Color::rgb(0xEDEDED) : ui::Color::rgb(0x333333);
                sk_sp<SkImage> img = ui::IconLoader::instance().glyph("go-previous", 16, iconCol);
                if (img) {
                    painter.drawImage(img.get(), slot.rect.x + (slot.rect.width - 16.0) / 2.0,
                                                 slot.rect.y + (slot.rect.height - 16.0) / 2.0);
                }
            } else if (slot.action >= 20 && slot.action <= 25) {
                // Flower Color Dots
                PastelColor pc = static_cast<PastelColor>(slot.action - 20);
                ui::Color dotCol = NoteItem::flowerColor(pc);
                bool isCurrent = (m_currentColor == pc);

                painter.fillEllipse(slot.rect, dotCol);
                if (isCurrent) {
                    ui::Color ring = dark ? ui::Color::rgb(0xFFFFFF) : ui::Color::rgb(0x1C1C1E);
                    painter.strokeRoundedRect(slot.rect.adjusted(-2.0, -2.0, 2.0, 2.0), 12.0, ring, 2);
                } else if (hovered) {
                    painter.strokeRoundedRect(slot.rect.adjusted(-1.0, -1.0, 1.0, 1.0), 11.0, dotCol.darker(1.3), 1);
                }
            } else if (slot.action == 11) {
                // Pin Toggle Button
                if (hovered) {
                    painter.fillRoundedRect(slot.rect, slot.rect.height / 2.0,
                        dark ? ui::Color::fromArgb(0x28FFFFFF) : ui::Color::fromArgb(0x18000000));
                }
                ui::Color iconCol = dark ? ui::Color::rgb(0xEDEDED) : ui::Color::rgb(0x333333);
                sk_sp<SkImage> img = ui::IconLoader::instance().glyph("bookmark-new", 16, iconCol);
                if (img) {
                    painter.drawImage(img.get(), slot.rect.x + (slot.rect.width - 16.0) / 2.0,
                                                 slot.rect.y + (slot.rect.height - 16.0) / 2.0);
                }
            } else if (slot.action == 12) {
                // Todo Button ("☑" Vector Checkbox)
                if (hovered) {
                    painter.fillRoundedRect(slot.rect, slot.rect.height / 2.0,
                        dark ? ui::Color::fromArgb(0x28FFFFFF) : ui::Color::fromArgb(0x18000000));
                }
                ui::Color iconCol = hovered ? (dark ? ui::Color::rgb(0x3385FF) : ui::Color::rgb(0x007AFF))
                                            : (dark ? ui::Color::rgb(0xEDEDED) : ui::Color::rgb(0x333333));
                double boxSize = 14.0;
                double bx = slot.rect.x + (slot.rect.width - boxSize) / 2.0;
                double by = slot.rect.y + (slot.rect.height - boxSize) / 2.0;
                ui::RectF boxRect{bx, by, boxSize, boxSize};
                painter.strokeRoundedRect(boxRect, 3.5, iconCol, 1.8);

                ui::Path check;
                check.moveTo(bx + 3.0, by + 7.0);
                check.lineTo(bx + 6.0, by + 10.5);
                check.lineTo(bx + 11.5, by + 4.0);
                painter.strokePath(check, iconCol, 2.0, ui::LineCap::Round, ui::LineJoin::Round);
            } else if (slot.action == 13) {
                // Delete Button (Trash)
                if (hovered) {
                    painter.fillRoundedRect(slot.rect, slot.rect.height / 2.0,
                        dark ? ui::Color::fromArgb(0x35FF453A) : ui::Color::fromArgb(0x20FF3B30));
                }
                ui::Color delCol = hovered ? ui::Color::rgb(0xFF3B30)
                                           : (dark ? ui::Color::rgb(0xEDEDED) : ui::Color::rgb(0x333333));
                sk_sp<SkImage> img = ui::IconLoader::instance().glyph("edit-delete", 16, delCol);
                if (img) {
                    painter.drawImage(img.get(), slot.rect.x + (slot.rect.width - 16.0) / 2.0,
                                                 slot.rect.y + (slot.rect.height - 16.0) / 2.0);
                }
            } else if (slot.action == 14) {
                // Circular Check Save Button (Save & Close)
                bool hov = (m_hoveredAction == slot.action);
                double r = slot.rect.width / 2.0;

                painter.drawShadow(slot.rect, r, hov ? 10.0 : 6.0,
                                   dark ? ui::Color::fromArgb(0x60000000) : ui::Color::fromArgb(0x20000000),
                                   {0, hov ? 3.0 : 1.5});

                ui::Color checkBg = dark ? (hov ? ui::Color::rgb(0x36D960) : ui::Color::rgb(0x30D158))
                                         : (hov ? ui::Color::rgb(0x28B84C) : ui::Color::rgb(0x34C759));
                painter.fillRoundedRect(slot.rect, r, checkBg);

                double cx = slot.rect.x + r;
                double cy = slot.rect.y + r;
                ui::Path checkPath;
                checkPath.moveTo(cx - 5.5, cy - 0.5);
                checkPath.lineTo(cx - 1.5, cy + 3.5);
                checkPath.lineTo(cx + 5.5, cy - 3.5);
                painter.strokePath(checkPath, ui::Color::rgb(0xFFFFFF), 2.2, ui::LineCap::Round, ui::LineJoin::Round);
            }
        }
    }
}

} // namespace cutefish::notes
