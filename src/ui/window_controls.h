// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ui/header_button.h"
#include "ui/view.h"

namespace cutefish::notes {

class WindowControls : public ui::View {
public:
    WindowControls();
    ~WindowControls() override = default;

    ui::SizeF sizeHint() const override { return {66, 20}; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;

private:
    ui::HeaderButton* m_close = nullptr;
    ui::HeaderButton* m_minimize = nullptr;
    ui::HeaderButton* m_maximize = nullptr;
};

} // namespace cutefish::notes
