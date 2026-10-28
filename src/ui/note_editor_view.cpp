// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "note_editor_view.h"
#include <algorithm>
#include <cmath>
#include "backend/note_store.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "ui/application.h"
#include "ui/clipboard.h"
#include "ui/icon_loader.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/text_context.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace cutefish::notes {

NoteEditorView::NoteEditorView()
{
    m_titleField = addChild<ui::TextField>();
    m_titleField->setFontSize(20.0);
    m_titleField->setFontWeight(700);
    m_titleField->setPlaceholder("Note Title...");
    m_titleField->setFrameVisible(false);
    m_titleField->onTextChanged = [this]() {
        if (!m_loading) saveCurrent();
    };

    m_contentArea = addChild<ui::TextArea>();
    m_contentArea->setFontSize(14.0);
    m_contentArea->setPlaceholder("Write your thoughts here, use markdown, or add todos with - [ ] task...");
    m_contentArea->setFrameVisible(false);
    m_contentArea->setWrapMode(ui::TextArea::WrapMode::WordWrap);
    m_contentArea->setPadding({8, 8, 8, 8});
    m_contentArea->onTextChanged = [this]() {
        if (!m_loading) saveCurrent();
    };
}

void NoteEditorView::setNote(const std::string& noteId, bool isNewNote)
{
    m_noteId = noteId;
    m_isNewNote = isNewNote;
    m_loading = true;

    const NoteItem* note = NoteStore::instance().findNote(noteId);
    if (note) {
        m_note = *note;
        m_originalTitle = m_note.title;
        m_originalContent = m_note.content;
        m_originalColor = m_note.color;
        m_originalPinned = m_note.pinned;

        if (m_isNewNote) {
            m_titleField->setText("");
            m_contentArea->setText("");
        } else {
            m_titleField->setText(m_note.title);
            m_contentArea->setText(m_note.content);
        }
    } else {
        m_note = NoteItem();
        m_originalTitle.clear();
        m_originalContent.clear();
        m_originalColor = PastelColor::Natural;
        m_originalPinned = false;
        m_titleField->setText("");
        m_contentArea->setText("");
    }

    m_loading = false;
    requestLayout();
    requestPaint();
}

void NoteEditorView::cancelCurrent()
{
    if (m_isNewNote) {
        // Discard new note draft completely so no ghost note remains in the garden
        NoteStore::instance().permanentlyDeleteNote(m_noteId);
    } else {
        // Revert any modifications made during editing session
        NoteStore::instance().updateNoteContent(m_noteId, m_originalTitle, m_originalContent);
        NoteStore::instance().setNoteColor(m_noteId, m_originalColor);
        if (m_note.pinned != m_originalPinned) {
            NoteStore::instance().togglePin(m_noteId);
        }
    }
    closeRequested.emit();
}

void NoteEditorView::deleteCurrent()
{
    if (m_isNewNote) {
        NoteStore::instance().permanentlyDeleteNote(m_noteId);
    } else {
        NoteStore::instance().deleteNote(m_noteId);
    }
    closeRequested.emit();
}

void NoteEditorView::saveAndDone()
{
    saveCurrent();
    m_isNewNote = false;
    closeRequested.emit();
}

bool NoteEditorView::keyPress(const ui::KeyEvent& event)
{
    if (event.key == ui::key::Escape) {
        cancelCurrent();
        return true;
    }
    return View::keyPress(event);
}

void NoteEditorView::saveCurrent()
{
    if (m_noteId.empty()) return;
    m_note.title = m_titleField->text();
    m_note.content = m_contentArea->text();
    NoteStore::instance().updateNoteContent(m_noteId, m_note.title, m_note.content);
    requestPaint();
}

void NoteEditorView::insertFormatting(const std::string& prefix, const std::string& suffix)
{
    std::string text = m_contentArea->text();
    size_t cursor = m_contentArea->cursorPosition();

    if (m_contentArea->hasSelection()) {
        std::string sel = m_contentArea->selectedText();
        std::string replacement = prefix + sel + suffix;
        // Simple insertion
        m_contentArea->cut();
        m_contentArea->paste();
    } else {
        std::string inserted = prefix + (suffix.empty() ? "" : suffix);
        text.insert(cursor, inserted);
        m_contentArea->setText(text);
        m_contentArea->setCursorPosition(cursor + prefix.size());
    }
    saveCurrent();
}

void NoteEditorView::layout()
{
    double w = localRect().width;
    double h = localRect().height;
    if (w <= 0 || h <= 0) return;

    // Floating card insets
    double pad = 16.0;
    double innerW = w - (pad * 2.0);

    // Top action bar: back, colors, pin, copy, delete (height 46)
    double topBarH = 46.0;

    // Title field
    m_titleField->setGeometry({pad + 16.0, pad + topBarH + 6.0, innerW - 32.0, 36.0});

    // Formatting toolbar (height 32)
    double formatBarH = 34.0;
    double formatBarY = pad + topBarH + 46.0;

    // Bottom status bar
    double bottomBarH = 36.0;

    // Content area
    double contentY = formatBarY + formatBarH + 8.0;
    double contentH = (h - pad) - bottomBarH - contentY;
    if (contentH < 80.0) contentH = 80.0;

    m_contentArea->setGeometry({pad + 14.0, contentY, innerW - 28.0, contentH});

    // Compute interactive button bounds
    m_buttons.clear();

    // 10: Cancel button ("✕ Cancel" pill on left)
    m_buttons.push_back({{pad + 14.0, pad + 10.0, 82.0, 28.0}, 10});

    // 0..6: Pastel Color Dots
    double colorStartX = pad + 108.0;
    for (int c = 0; c < 7; ++c) {
        m_buttons.push_back({{colorStartX + c * 24.0, pad + 14.0, 20.0, 20.0}, c});
    }

    // Right-aligned actions:
    // 14: Circular Check Save Button in corner (Save & Done ✓)
    // 13: Delete Note (🗑)
    // 12: Copy
    // 11: Pin
    double rightX = w - pad - 42.0;
    m_buttons.push_back({{rightX, pad + 8.0, 32.0, 32.0}, 14});         // Checkmark Save & Done
    m_buttons.push_back({{rightX - 44.0, pad + 9.0, 30.0, 30.0}, 13});  // Delete Note
    m_buttons.push_back({{rightX - 80.0, pad + 9.0, 30.0, 30.0}, 12});  // Copy
    m_buttons.push_back({{rightX - 116.0, pad + 9.0, 30.0, 30.0}, 11}); // Pin

    // Formatting Toolbar Buttons (20: Bold, 21: Italic, 22: Heading, 23: Todo, 24: Bullet)
    double fX = pad + 16.0;
    m_buttons.push_back({{fX, formatBarY + 3.0, 28.0, 26.0}, 20});
    m_buttons.push_back({{fX + 32.0, formatBarY + 3.0, 28.0, 26.0}, 21});
    m_buttons.push_back({{fX + 64.0, formatBarY + 3.0, 28.0, 26.0}, 22});
    m_buttons.push_back({{fX + 96.0, formatBarY + 3.0, 32.0, 26.0}, 23});
    m_buttons.push_back({{fX + 132.0, formatBarY + 3.0, 28.0, 26.0}, 24});
}

void NoteEditorView::pointerMove(const ui::PointerEvent& event)
{
    int oldHover = m_hoveredAction;
    m_hoveredAction = -1;
    for (const auto& btn : m_buttons) {
        if (btn.rect.contains(event.position)) {
            m_hoveredAction = btn.action;
            break;
        }
    }
    if (oldHover != m_hoveredAction) {
        requestPaint();
    }
}

void NoteEditorView::pointerPress(const ui::PointerEvent& event)
{
    for (const auto& btn : m_buttons) {
        if (btn.rect.contains(event.position)) {
            if (btn.action >= 0 && btn.action <= 6) {
                // Color changed
                m_note.color = static_cast<PastelColor>(btn.action);
                NoteStore::instance().setNoteColor(m_noteId, m_note.color);
                requestPaint();
                return;
            } else if (btn.action == 10) {
                // Cancel (go back without saving / discard new note draft)
                cancelCurrent();
                return;
            } else if (btn.action == 11) {
                // Pin
                m_note.pinned = !m_note.pinned;
                NoteStore::instance().togglePin(m_noteId);
                requestPaint();
                return;
            } else if (btn.action == 12) {
                // Copy to clipboard
                std::string fullText = m_note.title + "\n\n" + m_note.content;
                ui::Clipboard::setText(fullText);
                return;
            } else if (btn.action == 13) {
                // Delete Note
                deleteCurrent();
                return;
            } else if (btn.action == 14) {
                // Save and Done (Circular Check Button in corner)
                saveAndDone();
                return;
            } else if (btn.action == 20) {
                insertFormatting("**", "**");
                return;
            } else if (btn.action == 21) {
                insertFormatting("*", "*");
                return;
            } else if (btn.action == 22) {
                insertFormatting("# ");
                return;
            } else if (btn.action == 23) {
                insertFormatting("- [ ] ");
                return;
            } else if (btn.action == 24) {
                insertFormatting("- ");
                return;
            }
        }
    }

    if (window() && event.button == ui::MouseButton::Left && event.position.y <= 60.0) {
        window()->startMove();
    }
}

void NoteEditorView::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;
    double w = localRect().width;
    double h = localRect().height;

    double pad = 16.0;
    ui::RectF cardRect{pad, pad, w - pad * 2.0, h - pad * 2.0};

    // 1. Floating Elevation Shadow
    ui::Color shadowColor = dark ? ui::Color::fromArgb(0x80000000)
                                 : ui::Color::fromArgb(0x22000000);
    painter.drawShadow(cardRect, 20.0, 18.0, shadowColor, {0, 8.0});

    // 2. Pure White Card Background in light mode & Crisp Minimal Border
    ui::Color bg = dark ? ui::Color::rgb(0x23232A) : ui::Color::rgb(0xFFFFFF);
    ui::Color border = dark ? ui::Color::fromArgb(0x25FFFFFF) : ui::Color::fromArgb(0x14000000);
    painter.fillRoundedRect(cardRect, 20.0, bg);
    painter.strokeRoundedRect(cardRect, 20.0, border, 1);

    // 3. Top Action Buttons (Back, Colors, Pin, Copy, Delete)
    ui::Color iconColor = dark ? ui::Color::rgb(0xEDEDED) : ui::Color::rgb(0x3A3A3C);
    ui::Color accent = NoteItem::accentColor(m_note.color, dark);

    for (const auto& btn : m_buttons) {
        if (btn.action == 10) {
            // Cancel Button ("✕ Cancel" pill)
            bool hov = (m_hoveredAction == 10);
            double r = btn.rect.height / 2.0;

            // Apple macOS Sheet Style Neutral Frosted Pill
            ui::Color pillBg = dark ? (hov ? ui::Color::rgb(0x3B3D4A) : ui::Color::rgb(0x2B2C35))
                                    : (hov ? ui::Color::rgb(0xE4E6EE) : ui::Color::rgb(0xF0F1F5));
            ui::Color pillBorder = dark ? (hov ? ui::Color::rgb(0x525668) : ui::Color::rgb(0x3E404E))
                                        : (hov ? ui::Color::rgb(0xC4C7D2) : ui::Color::rgb(0xD6D9E2));
            ui::Color txtColor = dark ? (hov ? ui::Color::rgb(0xFFFFFF) : ui::Color::rgb(0xF2F2F7))
                                      : (hov ? ui::Color::rgb(0x111215) : ui::Color::rgb(0x26272C));
            ui::Color iconColor = dark ? (hov ? ui::Color::rgb(0xD2D5E0) : ui::Color::rgb(0x9CA0B0))
                                       : (hov ? ui::Color::rgb(0x3A3C43) : ui::Color::rgb(0x6A6D78));

            if (hov) {
                painter.drawShadow(btn.rect, r, 6.0,
                    dark ? ui::Color::fromArgb(0x40000000) : ui::Color::fromArgb(0x18000000), {0, 1.5});
            }

            painter.fillRoundedRect(btn.rect, r, pillBg);
            painter.strokeRoundedRect(btn.rect, r, pillBorder, 1.0);

            // Crisp vector "✕" icon
            double cx = btn.rect.x + 16.0;
            double cy = btn.rect.y + r;
            double arm = 3.6;
            ui::Path xPath;
            xPath.moveTo(cx - arm, cy - arm);
            xPath.lineTo(cx + arm, cy + arm);
            xPath.moveTo(cx + arm, cy - arm);
            xPath.lineTo(cx - arm, cy + arm);
            painter.strokePath(xPath, iconColor, 1.8, ui::LineCap::Round, ui::LineJoin::Round);

            // "Cancel" text using real system font from TextContext
            SkCanvas* canvas = painter.canvas();
            double s = painter.scale();
            double ox = painter.offset().x;
            double oy = painter.offset().y;
            if (canvas) {
                canvas->save();
                sk_sp<SkTypeface> tf = ui::Application::instance()->text().typeface(600);
                SkFont cancelFont(tf, SkDoubleToScalar(12.5 * s));
                SkPaint cancelPaint;
                cancelPaint.setAntiAlias(true);
                cancelPaint.setColor(txtColor.argb());
                const char* txt = "Cancel";
                canvas->drawString(txt, SkDoubleToScalar((ox + btn.rect.x + 25.5) * s),
                                        SkDoubleToScalar((oy + btn.rect.y + 18.5) * s), cancelFont, cancelPaint);
                canvas->restore();
            }
        } else if (btn.action >= 0 && btn.action <= 6) {
            // Pastel Color Dots
            PastelColor pc = static_cast<PastelColor>(btn.action);
            ui::Color dotCol = NoteItem::flowerColor(pc);
            bool isSelected = (m_note.color == pc);

            painter.fillEllipse(btn.rect, dotCol);
            if (isSelected) {
                painter.strokeRoundedRect(btn.rect.adjusted(-2.0, -2.0, 2.0, 2.0), 12.0, accent, 2);
            } else if (m_hoveredAction == btn.action) {
                painter.strokeRoundedRect(btn.rect.adjusted(-1.0, -1.0, 1.0, 1.0), 11.0, dotCol.darker(1.3), 1);
            }
        } else if (btn.action == 11) {
            // Pin (Bookmark Vector)
            bool hov = (m_hoveredAction == 11);
            if (m_note.pinned) {
                painter.fillRoundedRect(btn.rect, 14.0, accent.withAlpha(0.25));
                painter.strokeRoundedRect(btn.rect, 14.0, accent, 1);
            } else if (hov) {
                painter.fillRoundedRect(btn.rect, 14.0, dark ? ui::Color::fromArgb(0x20FFFFFF) : ui::Color::fromArgb(0x15000000));
            }
            double cx = btn.rect.x + btn.rect.width / 2.0;
            double cy = btn.rect.y + btn.rect.height / 2.0;
            ui::Path bm;
            bm.moveTo(cx - 5.0, cy - 6.5);
            bm.lineTo(cx + 5.0, cy - 6.5);
            bm.lineTo(cx + 5.0, cy + 6.5);
            bm.lineTo(cx, cy + 2.8);
            bm.lineTo(cx - 5.0, cy + 6.5);
            bm.close();
            if (m_note.pinned) {
                painter.fillPath(bm, accent);
            } else {
                painter.strokePath(bm, hov ? (dark ? ui::Color::rgb(0xFFFFFF) : ui::Color::rgb(0x1C1C1E)) : iconColor, 1.4, ui::LineCap::Round, ui::LineJoin::Round);
            }
        } else if (btn.action == 12) {
            // Copy (Dual Sheets Vector)
            bool hov = (m_hoveredAction == 12);
            if (hov) {
                painter.fillRoundedRect(btn.rect, 14.0, dark ? ui::Color::fromArgb(0x20FFFFFF) : ui::Color::fromArgb(0x15000000));
            }
            ui::Color cColor = hov ? (dark ? ui::Color::rgb(0xFFFFFF) : ui::Color::rgb(0x1C1C1E)) : iconColor;
            double cx = btn.rect.x + btn.rect.width / 2.0;
            double cy = btn.rect.y + btn.rect.height / 2.0;
            // Back sheet
            ui::Path backSheet;
            backSheet.moveTo(cx - 2.5, cy - 5.5);
            backSheet.lineTo(cx + 4.5, cy - 5.5);
            backSheet.lineTo(cx + 4.5, cy + 2.5);
            painter.strokePath(backSheet, cColor, 1.3, ui::LineCap::Round, ui::LineJoin::Round);
            // Front sheet
            ui::RectF frontSheet{cx - 5.5, cy - 2.5, 7.5, 9.0};
            painter.strokeRoundedRect(frontSheet, 1.5, cColor, 1.3);
        } else if (btn.action == 13) {
            // Delete Note (Vector Trash Can with soft red on hover)
            bool hov = (m_hoveredAction == 13);
            double r = btn.rect.height / 2.0;

            ui::Color delBg = dark ? (hov ? ui::Color::fromArgb(0x40FF453A) : ui::Color::rgb(0x282830))
                                   : (hov ? ui::Color::rgb(0xFFEBE9) : ui::Color::rgb(0xF2F3F7));
            ui::Color delBorder = dark ? (hov ? ui::Color::rgb(0xFF453A) : ui::Color::rgb(0x3E3E48))
                                       : (hov ? ui::Color::rgb(0xFFB3AD) : ui::Color::rgb(0xDCDEE6));
            ui::Color delIconCol = hov ? ui::Color::rgb(0xFF3B30)
                                       : (dark ? ui::Color::rgb(0xD1D1D6) : ui::Color::rgb(0x6E6E78));

            if (hov) {
                painter.drawShadow(btn.rect, r, 6.0,
                    dark ? ui::Color::fromArgb(0x30FF453A) : ui::Color::fromArgb(0x18FF3B30), {0, 1.5});
            }

            painter.fillRoundedRect(btn.rect, r, delBg);
            painter.strokeRoundedRect(btn.rect, r, delBorder, 1.0);

            // Draw crisp vector trash can
            double cx = btn.rect.x + btn.rect.width / 2.0;
            double cy = btn.rect.y + btn.rect.height / 2.0;

            // Lid handle
            ui::Path lidHandle;
            lidHandle.moveTo(cx - 2.0, cy - 6.0);
            lidHandle.lineTo(cx + 2.0, cy - 6.0);
            painter.strokePath(lidHandle, delIconCol, 1.3, ui::LineCap::Round, ui::LineJoin::Round);

            // Lid horizontal bar
            ui::Path lidBar;
            lidBar.moveTo(cx - 5.5, cy - 4.2);
            lidBar.lineTo(cx + 5.5, cy - 4.2);
            painter.strokePath(lidBar, delIconCol, 1.4, ui::LineCap::Round, ui::LineJoin::Round);

            // Can body
            ui::Path canBody;
            canBody.moveTo(cx - 4.2, cy - 2.8);
            canBody.lineTo(cx - 3.6, cy + 5.0);
            canBody.quadTo(cx - 3.3, cy + 6.5, cx - 1.8, cy + 6.5);
            canBody.lineTo(cx + 1.8, cy + 6.5);
            canBody.quadTo(cx + 3.3, cy + 6.5, cx + 3.6, cy + 5.0);
            canBody.lineTo(cx + 4.2, cy - 2.8);
            canBody.close();
            painter.strokePath(canBody, delIconCol, 1.3, ui::LineCap::Round, ui::LineJoin::Round);

            // 2 vertical slats
            ui::Path slats;
            slats.moveTo(cx - 1.5, cy - 1.0);
            slats.lineTo(cx - 1.5, cy + 4.5);
            slats.moveTo(cx + 1.5, cy - 1.0);
            slats.lineTo(cx + 1.5, cy + 4.5);
            painter.strokePath(slats, delIconCol, 1.1, ui::LineCap::Round, ui::LineJoin::Round);
        } else if (btn.action == 14) {
            // Circular Check Button (Save & Done in corner)
            bool hov = (m_hoveredAction == 14);
            double r = btn.rect.width / 2.0;

            painter.drawShadow(btn.rect, r, hov ? 12.0 : 7.0,
                               dark ? ui::Color::fromArgb(0x80000000) : ui::Color::fromArgb(0x30000000),
                               {0, hov ? 4.0 : 2.0});

            ui::Color checkBg = dark ? (hov ? ui::Color::rgb(0x36D960) : ui::Color::rgb(0x30D158))
                                     : (hov ? ui::Color::rgb(0x28B84C) : ui::Color::rgb(0x34C759));
            painter.fillRoundedRect(btn.rect, r, checkBg);

            double cx = btn.rect.x + r;
            double cy = btn.rect.y + r;
            ui::Path checkPath;
            checkPath.moveTo(cx - 5.5, cy - 0.5);
            checkPath.lineTo(cx - 1.5, cy + 3.5);
            checkPath.lineTo(cx + 5.5, cy - 3.5);
            painter.strokePath(checkPath, ui::Color::rgb(0xFFFFFF), 2.2, ui::LineCap::Round, ui::LineJoin::Round);
        } else if (btn.action >= 20 && btn.action <= 24) {
            // Formatting toolbar buttons
            bool hov = (m_hoveredAction == btn.action);
            if (hov) {
                painter.fillRoundedRect(btn.rect, 6.0, dark ? ui::Color::fromArgb(0x28FFFFFF) : ui::Color::fromArgb(0x20000000));
            }
            SkCanvas* canvas = painter.canvas();
            double s = painter.scale();
            if (canvas) {
                canvas->save();
                SkFont fFont(nullptr, SkDoubleToScalar(12.0 * s));
                SkPaint fPaint;
                fPaint.setAntiAlias(true);
                fPaint.setColor((hov ? accent : iconColor).argb());

                const char* label = (btn.action == 20) ? "B" :
                                    (btn.action == 21) ? "I" :
                                    (btn.action == 22) ? "H1" :
                                    (btn.action == 23) ? "☑" : "•";
                double ox = painter.offset().x;
                double oy = painter.offset().y;
                canvas->drawString(label, SkDoubleToScalar((ox + btn.rect.x + 8.0) * s),
                                   SkDoubleToScalar((oy + btn.rect.y + 18.0) * s), fFont, fPaint);
                canvas->restore();
            }
        }
    }

    // 4. Divider Line under Toolbar
    double divY = pad + 128.0;
    ui::Color divColor = dark ? ui::Color::fromArgb(0x20FFFFFF) : ui::Color::fromArgb(0x15000000);
    painter.drawHorizontalLine(pad + 16.0, divY, cardRect.width - 32.0, divColor);

    // 5. Bottom Status Bar Info
    double botY = (h - pad) - 24.0;
    SkCanvas* canvas = painter.canvas();
    double s = painter.scale();
    double ox = painter.offset().x;
    double oy = painter.offset().y;
    if (canvas) {
        canvas->save();
        SkFont statFont(nullptr, SkDoubleToScalar(11.0 * s));
        SkPaint statPaint;
        statPaint.setAntiAlias(true);
        statPaint.setColor((dark ? ui::Color::rgb(0x8E8E93) : ui::Color::rgb(0x6E6E73)).argb());

        std::string stats = "Words: " + std::to_string(m_note.wordCount()) +
                            "  •  Chars: " + std::to_string(m_note.charCount()) +
                            "  •  " + m_note.formattedDate();
        canvas->drawString(stats.c_str(), SkDoubleToScalar((ox + pad + 18.0) * s), SkDoubleToScalar((oy + botY) * s), statFont, statPaint);

        // Auto-saved badge
        SkPaint savePaint;
        savePaint.setAntiAlias(true);
        savePaint.setColor(accent.argb());
        const char* savedStr = "✓ Auto-saved";
        canvas->drawString(savedStr, SkDoubleToScalar((ox + w - pad - 95.0) * s), SkDoubleToScalar((oy + botY) * s), statFont, savePaint);

        canvas->restore();
    }
}

} // namespace cutefish::notes
