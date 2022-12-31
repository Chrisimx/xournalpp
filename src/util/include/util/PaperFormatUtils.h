/*
 * Xournal++
 *
 * Paper format utilities
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#include <memory>
#include <variant>
#include <vector>

#include <gtk/gtk.h>

#include "util/raii/GObjectSPtr.h"

#pragma once

namespace PaperFormatUtils {
/**
 * @brief A unique smart pointer to a GtkPaperSize
 */
using GtkPaperSizeUniquePtr_t = std::unique_ptr<GtkPaperSize, decltype(&gtk_paper_size_free)>;

/**
 * @brief A vector of menu options, either being a GtkPaperSize or the displayed name of the option.
 * @details The variant facilitates adding options which can't be expressed as a GtkPaperSize (e.g. a custom option
 *      or a copy current page format option)
 */
using PaperFormatMenuOptionVector_t = std::vector<std::variant<std::string, GtkPaperSizeUniquePtr_t>>;

/**
 * @brief Loads the default PaperSizes into the passed vector
 * @param menuOptions The vector containg the menu options
 */
void loadDefaultPaperSizes(PaperFormatMenuOptionVector_t& menuOptions);

/**
 * @brief Creates a GtkComboBox filled with the passed vector of GtkPaperSize entries and/or special entries
 * @param menuOptions The vector of menu options
 * @returns A smart pointer pointing to the GtkComboBox
 */
auto createPaperFormatDropDown(PaperFormatMenuOptionVector_t& menuOptions) -> xoj::util::WidgetSPtr;
}  // namespace PaperFormatUtils