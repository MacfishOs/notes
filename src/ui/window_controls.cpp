// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "window_controls.h"
#include <cmath>
#include "include/core/SkImage.h"
#include "ui/application.h"
#include "ui/icon_loader.h"
#include "ui/painter.h"
#include "ui/window.h"

namespace cutefish::notes {

namespace {

class TrafficLightButton : public ui::HeaderButton {
public:
    TrafficLightButton(const std::string& artwork, ui::Color plateColor, ui::Color glyphColor)
        : ui::HeaderButton(artwork, 20)
        , m_plateColor(plateColor)
        , m_glyphColor(glyphColor)
    {
    }

    void paint(ui::Painter& painter) override
    {
        const ui::Window* win = window();
        const bool active = win && win->isActive();

        const double radius = 7.0;
        const double size = radius * 2.0;
        const ui::Rect icon = painter.snappedRect({
            std::round((localRect().width - size) / 2.0),
            std::round((localRect().height - size) / 2.0),
            size, size
        });

        const bool pressed = m_pressed || m_checked;
        const double opacity = (active || m_hovered || pressed) ? 1.0 : 0.45;
        ui::Color color = m_plateColor;
        if (pressed) color = color.darker(1.2);

        painter.fillDeviceRoundedRect({icon.x, icon.y, icon.width, icon.height},
                                     icon.width / 2.0, color.withAlpha(opacity));

        if (m_hovered || pressed) {
            const std::string art = currentArtwork();
            if (art.empty()) return;
            ui::Color ink = m_glyphColor;
            if (pressed) ink = ink.darker(1.2);
            sk_sp<SkImage> img = ui::IconLoader::instance().glyph("window-" + art, icon.width, ink);
            if (img) {
                painter.drawImage(img.get(), icon.x, icon.y);
            }
        }
    }

protected:
    ui::Color m_plateColor;
    ui::Color m_glyphColor;
};

class CloseBtn final : public TrafficLightButton {
public:
    CloseBtn() : TrafficLightButton("close", ui::Color::rgb(0xFF5F56), ui::Color::rgb(0x4B0000)) {}
};

class MinBtn final : public TrafficLightButton {
public:
    MinBtn() : TrafficLightButton("minimize", ui::Color::rgb(0xFFBD2E), ui::Color::rgb(0x965000)) {}
};

class MaxBtn final : public TrafficLightButton {
public:
    MaxBtn() : TrafficLightButton("maximize", ui::Color::rgb(0x27C93F), ui::Color::rgb(0x005F00)) {}

protected:
    std::string currentArtwork() const override
    {
        const ui::Window* win = window();
        return win && win->states().maximized ? "restore" : "maximize";
    }
};

} // namespace

WindowControls::WindowControls()
{
    m_close = addChild<CloseBtn>();
    m_minimize = addChild<MinBtn>();
    m_maximize = addChild<MaxBtn>();

    m_close->onClicked = [this]() {
        ui::Window* win = window();
        if (win && win->onCloseRequested) {
            win->onCloseRequested();
        } else if (ui::Application::instance()) {
            ui::Application::instance()->quit();
        }
    };

    m_minimize->onClicked = [this]() {
        if (ui::Window* win = window()) {
            win->minimize();
        }
    };

    m_maximize->onClicked = [this]() {
        if (ui::Window* win = window()) {
            win->setMaximized(!win->states().maximized);
        }
    };
}

void WindowControls::layout()
{
    double h = localRect().height;
    double btnSize = 20.0;
    double btnY = (h - btnSize) / 2.0;

    m_close->setGeometry({0.0, btnY, btnSize, btnSize});
    m_minimize->setGeometry({22.0, btnY, btnSize, btnSize});
    m_maximize->setGeometry({44.0, btnY, btnSize, btnSize});
}

} // namespace cutefish::notes
