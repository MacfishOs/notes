// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "sidebar_view.h"
#include <cmath>
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "ui/icon_loader.h"
#include "ui/painter.h"
#include "ui/theme.h"

namespace cutefish::notes {

SidebarView::SidebarView()
{
    m_windowControls = addChild<WindowControls>();

    // 1. Library Section
    m_items.push_back({"LIBRARY", "", ViewFilter::All, PastelColor::Natural, false, true, {}});
    m_items.push_back({"All Notes", "document-new", ViewFilter::All, PastelColor::Natural, false, false, {}});
    m_items.push_back({"Pinned", "bookmark-new", ViewFilter::Pinned, PastelColor::Natural, false, false, {}});
    m_items.push_back({"To-Do & Tasks", "emblem-default", ViewFilter::Todos, PastelColor::Natural, false, false, {}});
    m_items.push_back({"Trash", "edit-delete", ViewFilter::Trash, PastelColor::Natural, false, false, {}});

    // 2. Pastel Palette Colors Section
    m_items.push_back({"PASTEL GARDEN", "", ViewFilter::ByColor, PastelColor::Natural, false, true, {}});
    m_items.push_back({"Blossom Pink", "", ViewFilter::ByColor, PastelColor::Pink, true, false, {}});
    m_items.push_back({"Lilac Violet", "", ViewFilter::ByColor, PastelColor::Lavender, true, false, {}});
    m_items.push_back({"Sage Mint", "", ViewFilter::ByColor, PastelColor::Mint, true, false, {}});
    m_items.push_back({"Buttercup", "", ViewFilter::ByColor, PastelColor::Butter, true, false, {}});
    m_items.push_back({"Sky Blue", "", ViewFilter::ByColor, PastelColor::Sky, true, false, {}});
    m_items.push_back({"Apricot Peach", "", ViewFilter::ByColor, PastelColor::Peach, true, false, {}});

    m_subs.push_back(NoteStore::instance().notesChanged.connect([this]() {
        requestPaint();
    }));
}

void SidebarView::layout()
{
    double w = localRect().width;
    double h = localRect().height;
    if (w <= 0 || h <= 0) return;

    // Window controls at top-left
    m_windowControls->setGeometry({16.0, 14.0, 60.0, 20.0});

    double y = 48.0;
    double itemH = 30.0;
    double itemPadX = 10.0;
    double itemW = w - (itemPadX * 2.0);

    for (size_t i = 0; i < m_items.size(); ++i) {
        auto& it = m_items[i];
        if (it.isHeader) {
            it.rect = {itemPadX + 6.0, y + 6.0, itemW, 18.0};
            y += 26.0;
        } else {
            it.rect = {itemPadX, y, itemW, itemH};
            y += itemH + 2.0;
        }
    }

    // Theme toggle button at bottom
    m_themeToggleRect = {itemPadX, h - 44.0, itemW, 32.0};
}

void SidebarView::pointerMove(const ui::PointerEvent& event)
{
    int oldHover = m_hoveredIndex;
    m_hoveredIndex = -1;

    for (size_t i = 0; i < m_items.size(); ++i) {
        if (!m_items[i].isHeader && m_items[i].rect.contains(event.position)) {
            m_hoveredIndex = static_cast<int>(i);
            break;
        }
    }

    if (oldHover != m_hoveredIndex) {
        requestPaint();
    }
}

void SidebarView::pointerPress(const ui::PointerEvent& event)
{
    if (m_themeToggleRect.contains(event.position)) {
        themeToggled.emit();
        return;
    }

    for (size_t i = 0; i < m_items.size(); ++i) {
        if (!m_items[i].isHeader && m_items[i].rect.contains(event.position)) {
            m_selectedIndex = i;
            NoteStore::instance().setFilter(m_items[i].filter, m_items[i].color);
            requestPaint();
            return;
        }
    }
}

void SidebarView::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;
    double w = localRect().width;
    double h = localRect().height;

    // 1. Translucent Acrylic Sidebar Background
    ui::Color sidebarBg = dark ? ui::Color::fromArgb(0x8014151C)
                               : ui::Color::fromArgb(0x85F6F7FA);
    painter.fillRect(localRect(), sidebarBg);

    // Right subtle divider
    ui::Color divider = dark ? ui::Color::fromArgb(0x25FFFFFF)
                             : ui::Color::fromArgb(0x18000000);
    painter.drawVerticalLine(w - 1.0, 0, h, divider);

    SkCanvas* canvas = painter.canvas();
    double s = painter.scale();

    // 2. Render Navigation Items
    for (size_t i = 0; i < m_items.size(); ++i) {
        const auto& it = m_items[i];
        if (it.isHeader) {
            // Section Header
            if (canvas) {
                canvas->save();
                SkFont hFont(nullptr, SkDoubleToScalar(10.5 * s));
                SkPaint hPaint;
                hPaint.setAntiAlias(true);
                hPaint.setColor((dark ? ui::Color::rgb(0x7C7C84) : ui::Color::rgb(0x8E8E93)).argb());
                canvas->drawString(it.title.c_str(),
                                   SkDoubleToScalar((it.rect.x + 4.0) * s),
                                   SkDoubleToScalar((it.rect.y + 13.0) * s),
                                   hFont, hPaint);
                canvas->restore();
            }
        } else {
            bool isSelected = (i == m_selectedIndex);
            bool isHovered = (static_cast<int>(i) == m_hoveredIndex);

            // Capsule highlight
            if (isSelected) {
                ui::Color selBg = dark ? ui::Color::rgb(0x0A84FF) : ui::Color::rgb(0x007AFF);
                painter.fillRoundedRect(it.rect, 8.0, selBg);
            } else if (isHovered) {
                ui::Color hovBg = dark ? ui::Color::fromArgb(0x20FFFFFF)
                                       : ui::Color::fromArgb(0x15000000);
                painter.fillRoundedRect(it.rect, 8.0, hovBg);
            }

            // Icon or Color Dot
            if (it.isColor) {
                ui::Color dotCol = NoteItem::flowerColor(it.color);
                painter.fillEllipse({it.rect.x + 10.0, it.rect.y + 9.0, 12.0, 12.0}, dotCol);
                painter.strokeRoundedRect({it.rect.x + 10.0, it.rect.y + 9.0, 12.0, 12.0}, 6.0, dotCol.darker(1.2), 1);
            } else if (!it.icon.empty()) {
                ui::Color iconColor = isSelected ? ui::Color::rgb(0xFFFFFF)
                                                 : (dark ? ui::Color::rgb(0xD1D1D6) : ui::Color::rgb(0x3A3A3C));
                sk_sp<SkImage> img = ui::IconLoader::instance().glyph(it.icon, 16, iconColor);
                if (img) {
                    painter.drawImage(img.get(), it.rect.x + 8.0, it.rect.y + 7.0);
                }
            }

            // Title Label
            if (canvas) {
                canvas->save();
                SkFont itemFont(nullptr, SkDoubleToScalar(13.0 * s));
                SkPaint itemPaint;
                itemPaint.setAntiAlias(true);
                ui::Color txtCol = isSelected ? ui::Color::rgb(0xFFFFFF)
                                              : (dark ? ui::Color::rgb(0xF0F0F5) : ui::Color::rgb(0x1C1C1E));
                itemPaint.setColor(txtCol.argb());

                double tx = it.rect.x + 30.0;
                canvas->drawString(it.title.c_str(), SkDoubleToScalar(tx * s),
                                   SkDoubleToScalar((it.rect.y + 19.5) * s), itemFont, itemPaint);

                // Count badge
                int count = 0;
                if (it.filter == ViewFilter::All) count = NoteStore::instance().totalCount();
                else if (it.filter == ViewFilter::Pinned) count = NoteStore::instance().pinnedCount();
                else if (it.filter == ViewFilter::Todos) count = NoteStore::instance().todoNotesCount();
                else if (it.filter == ViewFilter::Trash) count = NoteStore::instance().trashCount();
                else if (it.filter == ViewFilter::ByColor) count = NoteStore::instance().countByColor(it.color);

                if (count > 0) {
                    std::string cStr = std::to_string(count);
                    SkFont badgeFont(nullptr, SkDoubleToScalar(11.0 * s));
                    SkPaint badgePaint;
                    badgePaint.setAntiAlias(true);
                    badgePaint.setColor((isSelected ? ui::Color::rgb(0xFFFFFF).withAlpha(0.85) :
                                        (dark ? ui::Color::rgb(0x8E8E93) : ui::Color::rgb(0x8E8E93))).argb());

                    double bw = cStr.size() * 7.0;
                    canvas->drawString(cStr.c_str(), SkDoubleToScalar((it.rect.x + it.rect.width - bw - 8.0) * s),
                                       SkDoubleToScalar((it.rect.y + 19.5) * s), badgeFont, badgePaint);
                }

                canvas->restore();
            }
        }
    }

    // 3. Theme Toggle Button at Bottom
    ui::Color toggleBg = dark ? ui::Color::fromArgb(0x30FFFFFF)
                              : ui::Color::fromArgb(0x20000000);
    painter.fillRoundedRect(m_themeToggleRect, 10.0, toggleBg);

    if (canvas) {
        canvas->save();
        SkFont themeFont(nullptr, SkDoubleToScalar(12.5 * s));
        SkPaint themePaint;
        themePaint.setAntiAlias(true);
        themePaint.setColor((dark ? ui::Color::rgb(0xF0F0F5) : ui::Color::rgb(0x1C1C1E)).argb());

        const char* modeText = dark ? "☀️ Light Mode" : "🌙 Dark Mode";
        canvas->drawString(modeText, SkDoubleToScalar((m_themeToggleRect.x + 28.0) * s),
                           SkDoubleToScalar((m_themeToggleRect.y + 20.0) * s), themeFont, themePaint);
        canvas->restore();
    }
}

} // namespace cutefish::notes
