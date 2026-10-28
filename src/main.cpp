// SPDX-FileCopyrightText: 2026 Cutefish Desktop
// SPDX-License-Identifier: BSD-3-Clause

#include "ui/application.h"
#include "ui/main_window.h"

int main(int argc, char* argv[])
{
    cutefish::ui::Application app(argc, argv);
    app.setApplicationId("org.cutefish.notes");

    cutefish::notes::MainWindow window;
    if (argc > 1 && std::string(argv[1]) == "--test-editor") {
        window.openNewNoteEditor();
    }
    window.show();
    window.activate();

    return app.run();
}
