// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <vector>
#include "ui/view.h"

namespace cutefish::notes {

struct FlowerNode {
    double x = 0;
    double y = 0;
    double size = 15.0;
    int type = 0; // 0: Scarlet Poppy, 1: Golden Sunflower, 2: Cobalt Floret, 3: Crimson Rose, 4: Profile Poppy, 5: Chameli, 6: Tulip
    double angle = 0;
};

struct BudNode {
    double x = 0;
    double y = 0;
    double size = 9.0;
    int type = 0; // 0: Pointed scarlet bud, 1: Nodding green calyx bud
    double angle = 0;
};

struct TendrilNode {
    double x = 0;
    double y = 0;
    double size = 11.0;
    double angle = 0;
    bool flip = false;
};

struct VinePoint {
    double x1, y1;
    double cx1, cy1;
    double cx2, cy2;
    double x2, y2;
    double width = 2.4;
};

struct LeafNode {
    double x = 0;
    double y = 0;
    double length = 11.0;
    double angle = 0;
};

class BotanicalBackgroundView : public ui::View {
public:
    BotanicalBackgroundView();
    ~BotanicalBackgroundView() override = default;

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;

private:
    void generateBotanicalGarden(double w, double h);
    void drawVine(ui::Painter& painter, const VinePoint& vp, ui::Color vineColor);
    void drawLeaf(ui::Painter& painter, double x, double y, double length, double angle, ui::Color leafColor, bool dark);
    void drawTendril(ui::Painter& painter, const TendrilNode& tendril, ui::Color vineColor);
    void drawBud(ui::Painter& painter, const BudNode& bud, bool dark);
    void drawFlower(ui::Painter& painter, const FlowerNode& flower, bool dark);

    double m_lastW = 0;
    double m_lastH = 0;
    std::vector<VinePoint> m_vines;
    std::vector<LeafNode> m_leaves;
    std::vector<TendrilNode> m_tendrils;
    std::vector<BudNode> m_buds;
    std::vector<FlowerNode> m_flowers;
};

} // namespace cutefish::notes
