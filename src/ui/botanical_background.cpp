// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "botanical_background.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include "ui/painter.h"
#include "ui/theme.h"

namespace cutefish::notes {

static constexpr double kPi = 3.14159265358979323846;

BotanicalBackgroundView::BotanicalBackgroundView()
{
}

void BotanicalBackgroundView::layout()
{
    double w = localRect().width;
    double h = localRect().height;
    if (std::abs(w - m_lastW) > 8.0 || std::abs(h - m_lastH) > 8.0) {
        m_lastW = w;
        m_lastH = h;
        generateBotanicalGarden(w, h);
    }
}

void BotanicalBackgroundView::generateBotanicalGarden(double w, double h)
{
    m_vines.clear();
    m_leaves.clear();
    m_tendrils.clear();
    m_buds.clear();
    m_flowers.clear();

    if (w < 100 || h < 100) return;

    // Procedural pseudo-random organic variance for natural living botanical growth
    static uint32_t s_sessionSeed = 0;
    if (s_sessionSeed == 0) {
        s_sessionSeed = static_cast<uint32_t>(time(nullptr)) ^ 0x5A827999u;
    }
    uint32_t seed = s_sessionSeed ^ (static_cast<uint32_t>(w * 100.0) << 4) ^ static_cast<uint32_t>(h * 100.0);
    auto randF = [&seed](double minV, double maxV) {
        seed = seed * 1664525u + 1013904223u;
        double t = static_cast<double>(seed & 0xFFFF) / 65535.0;
        return minV + t * (maxV - minV);
    };

    // Helper to add a cubic bezier vine segment and organically populate leaves along its true mathematical tangent
    auto addVineSegment = [this, &randF](double x1, double y1, double cx1, double cy1, double cx2, double cy2, double x2, double y2, double width = 2.4, int leafCount = 3) {
        double jcx1 = cx1 + randF(-8.0, 8.0);
        double jcy1 = cy1 + randF(-8.0, 8.0);
        double jcx2 = cx2 + randF(-8.0, 8.0);
        double jcy2 = cy2 + randF(-8.0, 8.0);
        double jw = width + randF(-0.3, 0.3);
        m_vines.push_back({x1, y1, jcx1, jcy1, jcx2, jcy2, x2, y2, jw});

        if (leafCount > 0) {
            for (int i = 0; i < leafCount; ++i) {
                double t = (i + 1.0) / (leafCount + 1.0) + randF(-0.06, 0.06);
                t = std::clamp(t, 0.12, 0.88);
                double omt = 1.0 - t;

                // Position on cubic bezier curve B(t)
                double lx = omt*omt*omt*x1 + 3.0*omt*omt*t*jcx1 + 3.0*omt*t*t*jcx2 + t*t*t*x2;
                double ly = omt*omt*omt*y1 + 3.0*omt*omt*t*jcy1 + 3.0*omt*t*t*jcy2 + t*t*t*y2;

                // Curve tangent B'(t) for authentic biological growth alignment
                double dx = 3.0*omt*omt*(jcx1 - x1) + 6.0*omt*t*(jcx2 - jcx1) + 3.0*t*t*(x2 - jcx2);
                double dy = 3.0*omt*omt*(jcy1 - y1) + 6.0*omt*t*(jcy2 - jcy1) + 3.0*t*t*(y2 - jcy2);
                double tangent = std::atan2(dy, dx);

                // Alternating leaf direction with natural variation
                double side = (i % 2 == 0) ? 1.0 : -1.0;
                double leafAngle = tangent + side * (kPi * 0.38 + randF(-0.12, 0.12));
                double leafLen = randF(11.5, 16.5);

                m_leaves.push_back({lx, ly, leafLen, leafAngle});
            }
        }
    };

    // Helper to spawn a natural branching offshoot shoot
    auto addBranch = [this, &randF, &addVineSegment](double startX, double startY, double angle, double length, double width, int flowerType = -1, int budType = -1) {
        double cpDist = length * 0.55;
        double a1 = angle + randF(-0.25, 0.25);
        double a2 = angle + randF(-0.35, 0.35);
        double cx1 = startX + std::cos(a1) * (cpDist * 0.6);
        double cy1 = startY + std::sin(a1) * (cpDist * 0.6);
        double cx2 = startX + std::cos(a2) * cpDist;
        double cy2 = startY + std::sin(a2) * cpDist;
        double endX = startX + std::cos(angle) * length;
        double endY = startY + std::sin(angle) * length;

        addVineSegment(startX, startY, cx1, cy1, cx2, cy2, endX, endY, width, 1);

        if (flowerType >= 0) {
            double fSize = (flowerType == 1) ? randF(15.5, 19.5) : (flowerType == 0 ? randF(14.5, 18.0) : randF(12.5, 15.5));
            m_flowers.push_back({endX, endY, fSize, flowerType, randF(0.0, 2.0 * kPi)});
        } else if (budType >= 0) {
            m_buds.push_back({endX, endY, randF(8.5, 10.5), budType, angle - kPi * 0.5 + randF(-0.2, 0.2)});
        } else {
            // Spiral tendril curl at branch end
            m_tendrils.push_back({endX, endY, randF(9.5, 12.0), angle, randF(0, 1) > 0.5});
        }
    };

    // Helper to add a botanical flower with natural species sizing and full 360° rotation
    auto addFlower = [this, &randF](double x, double y, int type, double size = -1.0) {
        double jx = x + randF(-4.0, 4.0);
        double jy = y + randF(-4.0, 4.0);
        double s = size;
        if (s <= 0.0) {
            if (type == 1) s = randF(15.5, 20.0);       // 🌻 Golden Sunflower (hero, large)
            else if (type == 0) s = randF(14.5, 18.0);  // 🌹 Scarlet Poppy (hero)
            else if (type == 2) s = randF(12.5, 15.0);  // 💙 Cobalt Blue Floret
            else if (type == 3) s = randF(13.5, 16.5);  // 🌹 Crimson Rose
            else if (type == 4) s = randF(12.0, 14.5);  // 🌺 Profile Poppy
            else s = randF(11.0, 13.5);                 // 🌼 Chameli
        }
        double ja = randF(0.0, 2.0 * kPi);
        m_flowers.push_back({jx, jy, s, type, ja});
    };

    auto addBud = [this, &randF](double x, double y, int type, double angle) {
        double jx = x + randF(-3.0, 3.0);
        double jy = y + randF(-3.0, 3.0);
        double js = randF(8.5, 11.0);
        double ja = angle + randF(-0.25, 0.25);
        m_buds.push_back({jx, jy, js, type, ja});
    };

    // =========================================================================
    // SYSTEM 1: TOP-LEFT GARLAND CANOPY & BRANCHING SHOOTS
    // =========================================================================
    {
        addVineSegment(88.0, -8.0, 140.0, 32.0, 185.0, 28.0, 240.0, 54.0, 2.5, 3);
        addBranch(200.0, 44.0, 1.25, 68.0, 1.8, 4, -1); // Offshoot with profile poppy
        addBranch(240.0, 54.0, -0.60, 55.0, 1.8, -1, -1); // Offshoot with curly tendril

        addVineSegment(240.0, 54.0, 295.0, 76.0, 355.0, 65.0, 415.0, 46.0, 2.3, 3);
        addBranch(330.0, 70.0, 1.10, 58.0, 1.8, -1, 0); // Offshoot with pointed scarlet bud

        addFlower(150.0, 28.0, 0);  // 🌹 Scarlet Poppy
        addFlower(240.0, 54.0, 1);  // 🌻 Golden Sunflower
        addFlower(335.0, 68.0, 2);  // 💙 Cobalt Blue Floret
        addFlower(415.0, 46.0, 3);  // 🌹 Crimson Rose
    }

    // =========================================================================
    // SYSTEM 2: TOP-RIGHT GARLAND CANOPY & BRANCHING SHOOTS
    // =========================================================================
    {
        addVineSegment(w - 10.0, -8.0, w - 65.0, 30.0, w - 130.0, 25.0, w - 195.0, 54.0, 2.5, 3);
        addBranch(w - 155.0, 38.0, 2.10, 68.0, 1.8, 4, -1); // Offshoot with profile poppy
        addBranch(w - 195.0, 54.0, -2.45, 52.0, 1.8, -1, -1); // Offshoot with tendril

        addVineSegment(w - 195.0, 54.0, w - 265.0, 78.0, w - 335.0, 66.0, w - 410.0, 48.0, 2.3, 3);
        addBranch(w - 310.0, 72.0, 1.95, 62.0, 1.8, -1, 0); // Offshoot with scarlet bud

        addFlower(w - 85.0, 28.0, 2);               // 💙 Cobalt Blue Floret
        addFlower(w - 195.0, 54.0, 0);              // 🌹 Scarlet Poppy
        addFlower(w - 285.0, 74.0, 1, 19.5);        // 🌻 Radiant Golden Sunflower (Hero from photo!)
        addFlower(w - 410.0, 48.0, 3);              // 🌹 Crimson Rose
    }

    // Top-Center Connecting Bridge
    if (w > 720) {
        double midX = w * 0.5;
        addVineSegment(415.0, 46.0, midX - 80.0, 32.0, midX - 40.0, 56.0, midX, 50.0, 2.2, 2);
        addVineSegment(midX, 50.0, midX + 40.0, 44.0, midX + 80.0, 60.0, w - 410.0, 48.0, 2.2, 2);
        addFlower(midX - 35.0, 50.0, 0); // 🌹 Scarlet Poppy
        addFlower(midX + 45.0, 48.0, 1); // 🌻 Golden Sunflower
    }

    // =========================================================================
    // SYSTEM 3: LEFT CLIMBING TRELLIS
    // =========================================================================
    if (h > 380) {
        double lBaseY = h * 0.38;

        // Upper climbing branch
        addVineSegment(-6.0, lBaseY, 32.0, lBaseY - 45.0, 42.0, lBaseY - 95.0, 56.0, lBaseY - 140.0, 2.4, 3);
        addBranch(42.0, lBaseY - 95.0, 0.35, 52.0, 1.8, 2, -1); // Branch with Cobalt Blue floret
        addFlower(36.0, lBaseY - 60.0, 0); // 🌹 Scarlet Poppy
        addBud(56.0, lBaseY - 140.0, 0, -0.4); // Scarlet pointed bud

        // Lower climbing branch
        addVineSegment(-6.0, lBaseY, 28.0, lBaseY + 45.0, 38.0, lBaseY + 95.0, 50.0, lBaseY + 145.0, 2.4, 3);
        addBranch(38.0, lBaseY + 95.0, 0.45, 54.0, 1.8, 1, -1); // Branch with Golden Sunflower
        addFlower(32.0, lBaseY + 68.0, 2);  // 💙 Cobalt Blue Floret
        addFlower(50.0, lBaseY + 145.0, 1); // 🌻 Golden Sunflower
    }

    // =========================================================================
    // SYSTEM 4: RIGHT CLIMBING TRELLIS
    // =========================================================================
    if (h > 380) {
        double rBaseY = h * 0.42;

        // Upper climbing branch (prominent sunflowers matching photo)
        addVineSegment(w + 6.0, rBaseY, w - 32.0, rBaseY - 45.0, w - 42.0, rBaseY - 95.0, w - 56.0, rBaseY - 140.0, 2.4, 3);
        addBranch(w - 42.0, rBaseY - 95.0, 2.75, 54.0, 1.8, 1, -1); // Branch with large Golden Sunflower
        addFlower(w - 36.0, rBaseY - 60.0, 1, 18.5); // 🌻 Golden Sunflower
        addFlower(w - 56.0, rBaseY - 140.0, 2);      // 💙 Cobalt Blue Floret

        // Lower climbing branch
        addVineSegment(w + 6.0, rBaseY, w - 28.0, rBaseY + 45.0, w - 38.0, rBaseY + 95.0, w - 50.0, rBaseY + 145.0, 2.4, 3);
        addBranch(w - 38.0, rBaseY + 95.0, 2.65, 52.0, 1.8, 3, -1); // Branch with Crimson Rose
        addFlower(w - 34.0, rBaseY + 68.0, 0);  // 🌹 Scarlet Poppy
        addFlower(w - 50.0, rBaseY + 145.0, 2); // 💙 Cobalt Blue Floret
    }

    // =========================================================================
    // SYSTEM 5: CENTER INTER-CARD TRELLIS (meandering through gap between card rows)
    // =========================================================================
    if (h > 420) {
        double lBaseY = h * 0.38;
        double rBaseY = h * 0.42;
        double midY = std::clamp(h * 0.31, 195.0, 225.0);

        addVineSegment(26.0, lBaseY - 15.0, w * 0.12, midY + 14.0, w * 0.22, midY - 14.0, w * 0.35, midY + 8.0, 2.2, 3);
        addBranch(w * 0.35, midY + 8.0, -1.65, 65.0, 1.8, -1, 0); // Vertical branch peeking up between cols with scarlet bud

        addVineSegment(w * 0.35, midY + 8.0, w * 0.44, midY + 22.0, w * 0.54, midY - 12.0, w * 0.66, midY + 10.0, 2.2, 3);
        addBranch(w * 0.66, midY + 10.0, 1.55, 65.0, 1.8, -1, 1); // Vertical branch peeking down between cols with nodding calyx

        addVineSegment(w * 0.66, midY + 10.0, w * 0.76, midY + 20.0, w * 0.88, midY - 15.0, w - 24.0, rBaseY - 20.0, 2.2, 3);

        addFlower(w * 0.19, midY + 14.0, 4); // 🌺 Profile Poppy
        addFlower(w * 0.35, midY + 8.0, 0);  // 🌹 Scarlet Poppy
        addFlower(w * 0.50, midY - 6.0, 1);  // 🌻 Golden Sunflower
        addFlower(w * 0.66, midY + 10.0, 0); // 🌹 Scarlet Poppy
        addFlower(w * 0.80, midY + 12.0, 3); // 🌹 Crimson Rose
    }

    // =========================================================================
    // SYSTEM 6: CONTINUOUS BOTTOM FRAMING GARLAND
    // =========================================================================
    {
        double midX = w * 0.5;

        // Bottom-Left sweep inward toward floating button
        addVineSegment(-4.0, h - 30.0, w * 0.08, h - 55.0, w * 0.22, h - 68.0, midX - 58.0, h - 42.0, 2.2, 3);
        addBranch(w * 0.16, h - 62.0, -1.25, 52.0, 1.8, -1, 0); // Branch with scarlet bud

        addFlower(w * 0.10, h - 48.0, 0);    // 🌹 Scarlet Poppy
        addFlower(w * 0.22, h - 65.0, 1);    // 🌻 Golden Sunflower
        addFlower(midX - 90.0, h - 50.0, 2); // 💙 Cobalt Blue Floret

        // Gentle connecting loop dipping under the floating button
        addVineSegment(midX - 58.0, h - 42.0, midX - 32.0, h - 12.0, midX + 32.0, h - 12.0, midX + 58.0, h - 42.0, 1.8, 2);
        addFlower(midX, h - 12.0, 5); // 🌼 Dainty Chameli star floret centered under Add button

        // Bottom-Right sweep outward to right corner
        addVineSegment(midX + 58.0, h - 42.0, w - w * 0.22, h - 68.0, w - w * 0.08, h - 55.0, w + 4.0, h - 30.0, 2.2, 3);
        addBranch(w - w * 0.16, h - 62.0, -1.85, 52.0, 1.8, -1, 1); // Branch with nodding calyx

        addFlower(midX + 90.0, h - 50.0, 0);  // 🌹 Scarlet Poppy
        addFlower(w - w * 0.22, h - 65.0, 2); // 💙 Cobalt Blue Floret
        addFlower(w - w * 0.10, h - 48.0, 1); // 🌻 Golden Sunflower
    }
}

void BotanicalBackgroundView::drawVine(ui::Painter& painter, const VinePoint& vp, ui::Color vineColor)
{
    ui::Path path;
    path.moveTo(vp.x1, vp.y1);
    path.cubicTo(vp.cx1, vp.cy1, vp.cx2, vp.cy2, vp.x2, vp.y2);
    painter.strokePath(path, vineColor, vp.width, ui::LineCap::Round, ui::LineJoin::Round);
}

void BotanicalBackgroundView::drawTendril(ui::Painter& painter, const TendrilNode& tendril, ui::Color vineColor)
{
    painter.save();
    painter.translate({tendril.x, tendril.y});
    painter.rotate(tendril.angle * 180.0 / kPi, {0, 0});

    double s = tendril.size;
    double flip = tendril.flip ? -1.0 : 1.0;

    ui::Path spiral;
    spiral.moveTo(0, 0);
    spiral.cubicTo(s * 0.45, flip * (s * 0.05), s * 0.85, flip * (s * 0.35), s * 0.85, flip * (s * 0.75));
    spiral.cubicTo(s * 0.85, flip * (s * 1.15), s * 0.35, flip * (s * 1.25), s * 0.10, flip * (s * 0.85));
    spiral.cubicTo(-s * 0.05, flip * (s * 0.55), s * 0.20, flip * (s * 0.35), s * 0.45, flip * (s * 0.50));
    painter.strokePath(spiral, vineColor, 1.4, ui::LineCap::Round, ui::LineJoin::Round);

    painter.restore();
}

void BotanicalBackgroundView::drawLeaf(ui::Painter& painter, double x, double y, double length, double angle, ui::Color leafColor, bool dark)
{
    painter.save();
    painter.translate({x, y});
    painter.rotate(angle * 180.0 / kPi, {0, 0});

    double w = length * 0.46;

    // Dual-tone leaf shading: left half in deeper botanical green, right half in lighter sunlit green
    ui::Color deepGreen = leafColor;
    ui::Color lightGreen = dark ? ui::Color::rgb(0x62D67C) : ui::Color::rgb(0x48A45E);

    // Left half
    ui::Path leftHalf;
    leftHalf.moveTo(0, 0);
    leftHalf.quadTo(-w, length * 0.48, 0, length);
    leftHalf.lineTo(0, 0);
    leftHalf.close();
    painter.fillPath(leftHalf, deepGreen);

    // Right half
    ui::Path rightHalf;
    rightHalf.moveTo(0, 0);
    rightHalf.quadTo(w, length * 0.48, 0, length);
    rightHalf.lineTo(0, 0);
    rightHalf.close();
    painter.fillPath(rightHalf, lightGreen);

    // Central leaf midrib vein
    ui::Color veinColor = dark ? ui::Color::rgb(0x286E3C) : ui::Color::rgb(0x1B5A2C);
    ui::Path vein;
    vein.moveTo(0, 0);
    vein.lineTo(0, length * 0.92);
    painter.strokePath(vein, veinColor, 1.0, ui::LineCap::Round, ui::LineJoin::Round);

    // 2 pairs of fine lateral secondary veins
    ui::Path lateralVeins;
    lateralVeins.moveTo(0, length * 0.32);
    lateralVeins.lineTo(-w * 0.55, length * 0.44);
    lateralVeins.moveTo(0, length * 0.32);
    lateralVeins.lineTo(w * 0.55, length * 0.44);

    lateralVeins.moveTo(0, length * 0.60);
    lateralVeins.lineTo(-w * 0.48, length * 0.70);
    lateralVeins.moveTo(0, length * 0.60);
    lateralVeins.lineTo(w * 0.48, length * 0.70);
    painter.strokePath(lateralVeins, veinColor.withAlpha(0.6), 0.7, ui::LineCap::Round, ui::LineJoin::Round);

    painter.restore();
}

void BotanicalBackgroundView::drawBud(ui::Painter& painter, const BudNode& bud, bool dark)
{
    painter.save();
    painter.translate({bud.x, bud.y});
    painter.rotate(bud.angle * 180.0 / kPi, {0, 0});

    double r = bud.size;
    ui::Color sepalColor = dark ? ui::Color::rgb(0x3E9B4C) : ui::Color::rgb(0x2E7D3A);

    if (bud.type == 0) {
        // Pointed scarlet bud with clasping green sepals (from bottom-left of reference photo)
        ui::Color budRed = dark ? ui::Color::rgb(0xF03A4B) : ui::Color::rgb(0xE02636);
        ui::Color budGlow = dark ? ui::Color::rgb(0xFF5A6B) : ui::Color::rgb(0xEB4252);

        // Pointed scarlet cone
        ui::Path budTip;
        budTip.moveTo(-r * 0.28, -r * 0.1);
        budTip.quadTo(-r * 0.2, -r * 0.6, 0, -r * 1.05);
        budTip.quadTo(r * 0.2, -r * 0.6, r * 0.28, -r * 0.1);
        budTip.close();
        painter.fillPath(budTip, budRed);

        // Highlight line
        ui::Path hl;
        hl.moveTo(0, -r * 0.2);
        hl.lineTo(0, -r * 0.95);
        painter.strokePath(hl, budGlow, 1.0, ui::LineCap::Round, ui::LineJoin::Round);

        // Left clasping sepal
        ui::Path leftSepal;
        leftSepal.moveTo(0, r * 0.35);
        leftSepal.quadTo(-r * 0.52, -r * 0.15, -r * 0.14, -r * 0.72);
        leftSepal.quadTo(-r * 0.18, -r * 0.05, 0, r * 0.35);
        leftSepal.close();
        painter.fillPath(leftSepal, sepalColor);

        // Right clasping sepal
        ui::Path rightSepal;
        rightSepal.moveTo(0, r * 0.35);
        rightSepal.quadTo(r * 0.52, -r * 0.15, r * 0.14, -r * 0.72);
        rightSepal.quadTo(r * 0.18, -r * 0.05, 0, r * 0.35);
        rightSepal.close();
        painter.fillPath(rightSepal, sepalColor);

        // Pedicel stem base
        ui::Path stem;
        stem.moveTo(0, r * 0.35);
        stem.lineTo(0, r * 0.75);
        painter.strokePath(stem, sepalColor, 1.8, ui::LineCap::Round, ui::LineJoin::Round);
    } else {
        // Nodding teardrop green calyx bud (from left of reference photo)
        ui::Color budLight = dark ? ui::Color::rgb(0x56B864) : ui::Color::rgb(0x42964E);

        // Arched pedicel
        ui::Path pedicel;
        pedicel.moveTo(0, -r * 0.85);
        pedicel.quadTo(r * 0.3, -r * 0.45, 0, 0);
        painter.strokePath(pedicel, sepalColor, 1.6, ui::LineCap::Round, ui::LineJoin::Round);

        // Drooping teardrop calyx bud
        ui::Path teardrop;
        teardrop.moveTo(0, 0);
        teardrop.cubicTo(-r * 0.45, r * 0.3, -r * 0.35, r * 0.9, 0, r * 1.05);
        teardrop.cubicTo(r * 0.35, r * 0.9, r * 0.45, r * 0.3, 0, 0);
        teardrop.close();
        painter.fillPath(teardrop, sepalColor);

        // Light calyx stripe
        ui::Path stripe;
        stripe.moveTo(0, r * 0.15);
        stripe.lineTo(0, r * 0.9);
        painter.strokePath(stripe, budLight, 1.2, ui::LineCap::Round, ui::LineJoin::Round);
    }

    painter.restore();
}

void BotanicalBackgroundView::drawFlower(ui::Painter& painter, const FlowerNode& flower, bool dark)
{
    painter.save();
    painter.translate({flower.x, flower.y});
    painter.rotate(flower.angle * 180.0 / kPi, {0, 0});

    double r = flower.size;

    switch (flower.type) {
    case 0: {
        // =====================================================================
        // 🌹 VIVID SCARLET WILD POPPY (Hero flower from reference photo)
        // 5 broad overlapping scarlet petals, dark navy-black core, golden stamen ring
        // =====================================================================
        ui::Color petalBase = dark ? ui::Color::rgb(0xF03A4B) : ui::Color::rgb(0xE02636);
        ui::Color petalLight = dark ? ui::Color::rgb(0xFF5A6B) : ui::Color::rgb(0xEB4252);
        ui::Color creaseCol = dark ? ui::Color::fromArgb(0x60800814) : ui::Color::fromArgb(0x45800814);
        ui::Color centerDisc = dark ? ui::Color::rgb(0x121420) : ui::Color::rgb(0x1B1E30);
        ui::Color stamenCol = ui::Color::rgb(0xFFD000);

        for (int i = 0; i < 5; ++i) {
            double th = (i * 2.0 * kPi / 5.0) - (kPi / 2.0);
            double px = std::cos(th) * (r * 0.44);
            double py = std::sin(th) * (r * 0.44);
            double pr = r * 0.62;

            painter.save();
            painter.translate({px, py});
            painter.rotate(th * 180.0 / kPi + 90.0, {0, 0});

            // Broad rounded petal with gentle wave crest
            ui::Path petal;
            petal.moveTo(-pr * 0.55, -pr * 0.35);
            petal.cubicTo(-pr * 0.85, pr * 0.3, -pr * 0.5, pr * 0.95, 0, pr);
            petal.cubicTo(pr * 0.5, pr * 0.95, pr * 0.85, pr * 0.3, pr * 0.55, -pr * 0.35);
            petal.close();
            painter.fillPath(petal, petalBase);

            // Inner petal highlight
            ui::Path innerGlow;
            innerGlow.moveTo(-pr * 0.35, -pr * 0.2);
            innerGlow.cubicTo(-pr * 0.55, pr * 0.25, -pr * 0.3, pr * 0.75, 0, pr * 0.8);
            innerGlow.cubicTo(pr * 0.3, pr * 0.75, pr * 0.55, pr * 0.25, pr * 0.35, -pr * 0.2);
            innerGlow.close();
            painter.fillPath(innerGlow, petalLight);

            // Delicate radial petal crease lines
            ui::Path crease;
            crease.moveTo(0, -pr * 0.2);
            crease.lineTo(0, pr * 0.85);
            painter.strokePath(crease, creaseCol, 0.9, ui::LineCap::Round, ui::LineJoin::Round);

            painter.restore();
        }

        // Distinctive dark circular navy/indigo-black button core
        double cr = r * 0.30;
        painter.fillEllipse({-cr, -cr, cr * 2.0, cr * 2.0}, centerDisc);

        // Ring of 12 bright golden-yellow stamen dots (anthers)
        int numDots = 12;
        for (int i = 0; i < numDots; ++i) {
            double th = i * (2.0 * kPi / numDots);
            double dx = std::cos(th) * (cr * 0.95);
            double dy = std::sin(th) * (cr * 0.95);
            painter.fillEllipse({dx - 1.4, dy - 1.4, 2.8, 2.8}, stamenCol);
        }

        // Center speck inside the dark button
        painter.fillEllipse({-1.2, -1.2, 2.4, 2.4}, dark ? ui::Color::rgb(0x282C40) : ui::Color::rgb(0x2E3350));
        break;
    }

    case 1: {
        // =====================================================================
        // 🌻 RADIANT GOLDEN SUNFLOWER / BLACK-EYED SUSAN (From reference photo)
        // 14 pointed golden petals, large deep burnt-umber seed disc
        // =====================================================================
        ui::Color petalGold = dark ? ui::Color::rgb(0xFFC300) : ui::Color::rgb(0xFFBA08);
        ui::Color petalDark = dark ? ui::Color::rgb(0xFAA307) : ui::Color::rgb(0xF48C06);
        ui::Color centerDisc = dark ? ui::Color::rgb(0x2E1608) : ui::Color::rgb(0x3D2010);
        ui::Color ringColor = dark ? ui::Color::rgb(0x603215) : ui::Color::rgb(0x6B3818);
        ui::Color pollenGold = ui::Color::rgb(0xEAA221);

        int petals = 14;
        for (int i = 0; i < petals; ++i) {
            double th = i * (2.0 * kPi / petals);
            double length = r * 1.05;
            double width = r * 0.22;

            painter.save();
            painter.rotate(th * 180.0 / kPi, {0, 0});

            // Pointed golden petal
            ui::Path p;
            p.moveTo(0, r * 0.25);
            p.quadTo(-width, length * 0.55, 0, length);
            p.quadTo(width, length * 0.55, 0, r * 0.25);
            p.close();
            painter.fillPath(p, (i % 2 == 0) ? petalGold : petalDark);

            // Center rib
            ui::Path rib;
            rib.moveTo(0, r * 0.32);
            rib.lineTo(0, length * 0.88);
            painter.strokePath(rib, ui::Color::fromArgb(0x357A3C00), 0.8, ui::LineCap::Round, ui::LineJoin::Round);

            painter.restore();
        }

        // Large deep burnt-umber seed disc
        double discR = r * 0.38;
        painter.fillEllipse({-discR, -discR, discR * 2.0, discR * 2.0}, centerDisc);

        // Inner textured ring
        double innerR = discR * 0.72;
        painter.strokeRoundedRect({-innerR, -innerR, innerR * 2.0, innerR * 2.0}, innerR, ringColor, 2.0);

        // Stippled golden pollen ring
        for (int i = 0; i < 14; ++i) {
            double th = i * (2.0 * kPi / 14.0) + 0.12;
            double px = std::cos(th) * (discR * 0.82);
            double py = std::sin(th) * (discR * 0.82);
            painter.fillEllipse({px - 1.0, py - 1.0, 2.0, 2.0}, pollenGold);
        }
        break;
    }

    case 2: {
        // =====================================================================
        // 💙 COBALT / ROYAL BLUE 5-PETAL FLORET (From reference photo)
        // 5 notched royal blue petals, crisp glowing white star eye
        // =====================================================================
        ui::Color blueCol = dark ? ui::Color::rgb(0x4C6EF5) : ui::Color::rgb(0x3A5BD9);
        ui::Color darkBlue = dark ? ui::Color::rgb(0x364FC7) : ui::Color::rgb(0x2845B5);
        ui::Color centerWhite = ui::Color::rgb(0xFFFFFF);
        ui::Color centerDot = dark ? ui::Color::rgb(0x1B1E30) : ui::Color::rgb(0x141724);

        for (int i = 0; i < 5; ++i) {
            double th = (i * 2.0 * kPi / 5.0) - (kPi / 2.0);
            double length = r * 0.95;
            double width = r * 0.34;

            painter.save();
            painter.rotate(th * 180.0 / kPi, {0, 0});

            // Petal with heart notch at tip
            ui::Path p;
            p.moveTo(0, r * 0.15);
            p.cubicTo(-width, length * 0.4, -width * 0.9, length * 0.95, -width * 0.3, length);
            p.lineTo(0, length * 0.88);
            p.lineTo(width * 0.3, length);
            p.cubicTo(width * 0.9, length * 0.95, width, length * 0.4, 0, r * 0.15);
            p.close();
            painter.fillPath(p, blueCol);

            // Inner crease
            ui::Path v;
            v.moveTo(0, r * 0.2);
            v.lineTo(0, length * 0.80);
            painter.strokePath(v, darkBlue, 0.9, ui::LineCap::Round, ui::LineJoin::Round);

            painter.restore();
        }

        // Crisp White Central Star / Ring
        double starR = r * 0.24;
        painter.fillEllipse({-starR, -starR, starR * 2.0, starR * 2.0}, centerWhite);

        // Center dark dot
        painter.fillEllipse({-r * 0.08, -r * 0.08, r * 0.16, r * 0.16}, centerDot);
        // Tiny golden pistil fleck
        painter.fillEllipse({-0.8, -0.8, 1.6, 1.6}, ui::Color::rgb(0xFFD000));
        break;
    }

    case 3: {
        // =====================================================================
        // 🌹 CRIMSON GARDEN ROSE (From reference photo)
        // Multi-tiered ruffled crimson petals, golden stamen cluster
        // =====================================================================
        ui::Color outerPetal = dark ? ui::Color::rgb(0xEB3646) : ui::Color::rgb(0xD92534);
        ui::Color midPetal = dark ? ui::Color::rgb(0xF24E5C) : ui::Color::rgb(0xE83648);
        ui::Color innerPetal = dark ? ui::Color::rgb(0xF76B78) : ui::Color::rgb(0xEE4E5E);

        // 6 Outer ruffled petals
        for (int i = 0; i < 6; ++i) {
            double th = i * (2.0 * kPi / 6.0);
            double px = std::cos(th) * (r * 0.48);
            double py = std::sin(th) * (r * 0.48);
            double pr = r * 0.50;
            painter.fillEllipse({px - pr, py - pr, pr * 2.0, pr * 2.0}, outerPetal);
        }

        // 5 Mid petals
        for (int i = 0; i < 5; ++i) {
            double th = i * (2.0 * kPi / 5.0) + 0.3;
            double px = std::cos(th) * (r * 0.32);
            double py = std::sin(th) * (r * 0.32);
            double pr = r * 0.38;
            painter.fillEllipse({px - pr, py - pr, pr * 2.0, pr * 2.0}, midPetal);
        }

        // Inner rosette cup
        painter.fillEllipse({-r * 0.22, -r * 0.22, r * 0.44, r * 0.44}, innerPetal);

        // Golden stamen cluster ring
        for (int i = 0; i < 8; ++i) {
            double th = i * (2.0 * kPi / 8.0);
            double sx = std::cos(th) * (r * 0.14);
            double sy = std::sin(th) * (r * 0.14);
            painter.fillEllipse({sx - 1.2, sy - 1.2, 2.4, 2.4}, ui::Color::rgb(0xFFD633));
        }
        break;
    }

    case 4: {
        // =====================================================================
        // 🌺 SIDE-PROFILE CUPPING SCARLET POPPY (From reference photo)
        // Cupped scarlet petals on green calyx with golden stamens
        // =====================================================================
        ui::Color sepalCol = dark ? ui::Color::rgb(0x3B8E48) : ui::Color::rgb(0x2F7D3A);
        ui::Color scarletCol = dark ? ui::Color::rgb(0xF03A4B) : ui::Color::rgb(0xE02636);
        ui::Color innerScarlet = dark ? ui::Color::rgb(0xFF5A6B) : ui::Color::rgb(0xEB4252);

        // Calyx base
        ui::Path calyx;
        calyx.moveTo(-r * 0.25, r * 0.4);
        calyx.quadTo(0, r * 0.65, r * 0.25, r * 0.4);
        calyx.lineTo(0, r * 0.85);
        calyx.close();
        painter.fillPath(calyx, sepalCol);

        // Center upright petal
        ui::Path centerP;
        centerP.moveTo(0, r * 0.42);
        centerP.cubicTo(-r * 0.4, 0, -r * 0.3, -r * 0.8, 0, -r * 0.95);
        centerP.cubicTo(r * 0.3, -r * 0.8, r * 0.4, 0, 0, r * 0.42);
        centerP.close();
        painter.fillPath(centerP, innerScarlet);

        // Left cupping petal
        ui::Path leftP;
        leftP.moveTo(-r * 0.1, r * 0.42);
        leftP.cubicTo(-r * 0.8, r * 0.1, -r * 0.75, -r * 0.5, -r * 0.35, -r * 0.75);
        leftP.cubicTo(-r * 0.2, -r * 0.1, -r * 0.1, r * 0.2, -r * 0.1, r * 0.42);
        leftP.close();
        painter.fillPath(leftP, scarletCol);

        // Right cupping petal
        ui::Path rightP;
        rightP.moveTo(r * 0.1, r * 0.42);
        rightP.cubicTo(r * 0.8, r * 0.1, r * 0.75, -r * 0.5, r * 0.35, -r * 0.75);
        rightP.cubicTo(r * 0.2, -r * 0.1, r * 0.1, r * 0.2, r * 0.1, r * 0.42);
        rightP.close();
        painter.fillPath(rightP, scarletCol);

        // Stamen peeking from throat
        painter.fillEllipse({-r * 0.1, -r * 0.4, 2.5, 2.5}, ui::Color::rgb(0xFFD000));
        painter.fillEllipse({r * 0.1, -r * 0.42, 2.5, 2.5}, ui::Color::rgb(0xFFD000));
        break;
    }

    case 5: {
        // =====================================================================
        // 🌼 CHAMELI / JASMINE (5 Ivory Star Petals with 3D Crease)
        // =====================================================================
        ui::Color petalColor = dark ? ui::Color::rgb(0xFFFAEE) : ui::Color::rgb(0xFFFDF6);
        ui::Color shadeColor = dark ? ui::Color::rgb(0xF2EAE0) : ui::Color::rgb(0xF5EBE2);
        ui::Color heartColor = dark ? ui::Color::rgb(0xFFD854) : ui::Color::rgb(0xFCC030);

        for (int i = 0; i < 5; ++i) {
            double angle = (i * 2.0 * kPi) / 5.0;
            double length = r * 0.96;
            double width = r * 0.32;

            painter.save();
            painter.rotate(angle * 180.0 / kPi, {0, 0});

            ui::Path litSide;
            litSide.moveTo(0, 0);
            litSide.quadTo(-width, length * 0.42, 0, length);
            litSide.lineTo(0, 0);
            litSide.close();
            painter.fillPath(litSide, petalColor);

            ui::Path shadeSide;
            shadeSide.moveTo(0, 0);
            shadeSide.quadTo(width, length * 0.42, 0, length);
            shadeSide.lineTo(0, 0);
            shadeSide.close();
            painter.fillPath(shadeSide, shadeColor);

            ui::Path crease;
            crease.moveTo(0, length * 0.15);
            crease.lineTo(0, length * 0.88);
            painter.strokePath(crease, dark ? ui::Color::fromArgb(0x30000000) : ui::Color::fromArgb(0x18000000), 0.8, ui::LineCap::Round, ui::LineJoin::Round);

            painter.restore();
        }

        painter.fillEllipse({-r * 0.24, -r * 0.24, r * 0.48, r * 0.48}, dark ? ui::Color::rgb(0x609040) : ui::Color::rgb(0x8AB850));
        painter.fillEllipse({-r * 0.18, -r * 0.18, r * 0.36, r * 0.36}, heartColor);
        painter.fillEllipse({-r * 0.08, -r * 0.08, r * 0.16, r * 0.16}, ui::Color::rgb(0xFF9010));
        break;
    }

    case 6:
    default: {
        // =====================================================================
        // 🌷 TULIP GOBLET (Upright Goblet with Sepals & Petal Striations)
        // =====================================================================
        ui::Color calyxCol = dark ? ui::Color::rgb(0x3E9B4C) : ui::Color::rgb(0x2E7D3A);
        ui::Color tulipOuter = dark ? ui::Color::rgb(0xFFB098) : ui::Color::rgb(0xFFA085);
        ui::Color tulipCenter = dark ? ui::Color::rgb(0xFFC6B4) : ui::Color::rgb(0xFFBAA4);
        ui::Color tulipInner = dark ? ui::Color::rgb(0xFFD8C8) : ui::Color::rgb(0xFFCCBC);

        // Calyx base
        ui::Path calyx;
        calyx.moveTo(-r * 0.18, r * 0.52);
        calyx.quadTo(0, r * 0.68, r * 0.18, r * 0.52);
        calyx.lineTo(0, r * 0.85);
        calyx.close();
        painter.fillPath(calyx, calyxCol);

        // Back Petal
        ui::Path backPetal;
        backPetal.moveTo(0, r * 0.55);
        backPetal.quadTo(-r * 0.35, 0, 0, -r * 0.95);
        backPetal.quadTo(r * 0.35, 0, 0, r * 0.55);
        backPetal.close();
        painter.fillPath(backPetal, tulipInner);

        // Front Center petal
        ui::Path centerPetal;
        centerPetal.moveTo(0, r * 0.55);
        centerPetal.quadTo(-r * 0.38, 0, 0, -r * 0.82);
        centerPetal.quadTo(r * 0.38, 0, 0, r * 0.55);
        centerPetal.close();
        painter.fillPath(centerPetal, tulipCenter);

        // Left cupping petal
        ui::Path leftPetal;
        leftPetal.moveTo(-r * 0.05, r * 0.55);
        leftPetal.cubicTo(-r * 0.78, r * 0.20, -r * 0.72, -r * 0.45, -r * 0.28, -r * 0.72);
        leftPetal.cubicTo(-r * 0.20, -r * 0.08, -r * 0.20, r * 0.28, -r * 0.05, r * 0.55);
        leftPetal.close();
        painter.fillPath(leftPetal, tulipOuter);

        // Right cupping petal
        ui::Path rightPetal;
        rightPetal.moveTo(r * 0.05, r * 0.55);
        rightPetal.cubicTo(r * 0.78, r * 0.20, r * 0.72, -r * 0.45, r * 0.28, -r * 0.72);
        rightPetal.cubicTo(r * 0.20, -r * 0.08, r * 0.20, r * 0.28, r * 0.05, r * 0.55);
        rightPetal.close();
        painter.fillPath(rightPetal, tulipOuter);

        // Vertical striations
        ui::Color stripeCol = dark ? ui::Color::fromArgb(0x40FFFFFF) : ui::Color::fromArgb(0x35E05030);
        ui::Path stripes;
        stripes.moveTo(0, r * 0.48);
        stripes.lineTo(0, -r * 0.68);
        stripes.moveTo(-r * 0.22, r * 0.40);
        stripes.quadTo(-r * 0.48, 0, -r * 0.24, -r * 0.55);
        stripes.moveTo(r * 0.22, r * 0.40);
        stripes.quadTo(r * 0.48, 0, r * 0.24, -r * 0.55);
        painter.strokePath(stripes, stripeCol, 0.8, ui::LineCap::Round, ui::LineJoin::Round);
        break;
    }
    }

    painter.restore();
}

void BotanicalBackgroundView::paint(ui::Painter& painter)
{
    bool dark = ui::Theme::current().darkMode;

    // 1. Flat Canvas Background (Pure flat white in light mode, dark slate in dark mode)
    ui::Color canvasBg = dark ? ui::Color::rgb(0x18181C) : ui::Color::rgb(0xFFFFFF);
    painter.fillRect(localRect(), canvasBg);

    // 2. Defined Foliage Colors (Leafy botanical green matching the reference photo)
    ui::Color vineColor = dark ? ui::Color::fromArgb(0xF53E9B4C)
                               : ui::Color::fromArgb(0xF52E7D3A);

    ui::Color leafColor = dark ? ui::Color::fromArgb(0xF842A652)
                               : ui::Color::fromArgb(0xF82E7D3A);

    // 3. Draw Continuous Vines
    for (const auto& vp : m_vines) {
        drawVine(painter, vp, vineColor);
    }

    // 4. Draw Spiral Tendrils at Branch Junctions/Tips
    for (const auto& tendril : m_tendrils) {
        drawTendril(painter, tendril, vineColor);
    }

    // 5. Draw Dual-Tone Botanical Leaves with Veining
    for (const auto& leaf : m_leaves) {
        drawLeaf(painter, leaf.x, leaf.y, leaf.length, leaf.angle, leafColor, dark);
    }

    // 6. Draw Tender Buds
    for (const auto& bud : m_buds) {
        drawBud(painter, bud, dark);
    }

    // 7. Draw Detailed Flowers
    for (const auto& fl : m_flowers) {
        drawFlower(painter, fl, dark);
    }
}

} // namespace cutefish::notes
