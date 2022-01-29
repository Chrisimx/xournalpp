#pragma once

#include "control/tools/PdfElemSelection.h"
#include "gui/PageView.h"
#include "gui/XournalView.h"

#include "GladeGui.h"

class MainWindow;

enum class PdfMarkerStyle : uint8_t {
    POS_TEXT_BOTTOM = 0,
    POS_TEXT_MIDDLE,
    POS_TEXT_TOP,

    WIDTH_TEXT_LINE,
    WIDTH_TEXT_HEIGHT
};


class PdfFloatingToolbox {
public:
    PdfFloatingToolbox(MainWindow* theMainWindow, GtkOverlay* overlay);
    PdfFloatingToolbox& operator=(const PdfFloatingToolbox&) = delete;
    PdfFloatingToolbox(const PdfFloatingToolbox&) = delete;
    PdfFloatingToolbox& operator=(PdfFloatingToolbox&&) = delete;
    PdfFloatingToolbox(PdfFloatingToolbox&&) = delete;
    ~PdfFloatingToolbox() = default;

public:
    /// Show the toolbox at the provided coordinates. Must have an active
    /// selection.
    void show(int x, int y);

    /// Hide the floating toolbox widget (keeping the current selection).
    void hide();

    /// Returns true if the toolbox is currently hidden.
    bool isHidden() const;

    /// Returns the selection, or nullptr if no PDF element is selected.
    PdfElemSelection* getSelection() const;

    /// Clears the current selection (without any visual effects).
    void clearSelection();

    /// Create a new selection (without any visual effects)
    void newSelection(double x, double y, XojPageView* pageView);

    /// Returns true iff a PDF element is selected;
    bool hasSelection() const;

    /// Cancel the selection, rerender the page, and hide the toolbox.
    void userCancelSelection();

private:
    void show();

    static gboolean getOverlayPosition(GtkOverlay* overlay, GtkWidget* widget, GdkRectangle* allocation,
                                       PdfFloatingToolbox* self);

    static void switchSelectTypeCb(GtkButton* button, PdfFloatingToolbox* pft);
    static void strikethroughCb(GtkButton* button, PdfFloatingToolbox* pft);
    static void underlineCb(GtkButton* button, PdfFloatingToolbox* pft);
    static void copyTextCb(GtkButton* button, PdfFloatingToolbox* pft);
    static void highlightCb(GtkButton* button, PdfFloatingToolbox* pft);

    void copyTextToClipboard();
    void createStrokes(PdfMarkerStyle position, PdfMarkerStyle width, int markerOpacity);

private:
    // bool hidden;

    GtkWidget* floatingToolbox;
    MainWindow* theMainWindow;

    std::unique_ptr<PdfElemSelection> pdfElemSelection = nullptr;

    struct {
        int x;
        int y;
    } position;
};