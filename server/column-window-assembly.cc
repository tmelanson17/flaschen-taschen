// -*- mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation version 2.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://gnu.org/licenses/gpl-2.0.txt>

#include "led-flaschen-taschen.h"

#include <unistd.h>
#include "multi-spi.h"

#define START_X_MINI_WINDOW 3 * 5 // 0 index
#define WIDTH_MINI_WINDOW 3 * 5
#define START_Y_MINI_WINDOW 0 * 5 // 0 index
#define HEIGHT_MINI_WINDOW 3 * 5

namespace {

// Returns the extended height value of the crate given the original x,y coordinates.
static int GetHeightMiniWindow(const int x, const int y, const int height) {
     if (x < START_X_MINI_WINDOW || x >= WIDTH_MINI_WINDOW + START_X_MINI_WINDOW ||
         y < START_Y_MINI_WINDOW || y >= HEIGHT_MINI_WINDOW + START_Y_MINI_WINDOW) {
         // Return the default value.
         return height - y - 1;
     }
     int snake_x_column = ((x - START_X_MINI_WINDOW)) / 5;
     return height + snake_x_column*HEIGHT_MINI_WINDOW +
         (HEIGHT_MINI_WINDOW - (y - START_Y_MINI_WINDOW) - 1);
}

static int GetColumnMiniWindow(const int x, const int y) {
     if (x < START_X_MINI_WINDOW || x >= WIDTH_MINI_WINDOW + START_X_MINI_WINDOW ||
         y < START_Y_MINI_WINDOW || y >= HEIGHT_MINI_WINDOW + START_Y_MINI_WINDOW) {
         // Return the default value.
         return x / 5;
     }   
     return 2; // Column with the extended wiring.

}
}  // namespace

// Screw it, harcoding the height
ColumnWindowAssembly::ColumnWindowAssembly(spixels::MultiSPI *spi)
    : spi_(spi), width_(0), height_(40)  {}

ColumnWindowAssembly::~ColumnWindowAssembly() {
    for (size_t i = 0; i < columns_.size(); ++i)
        delete columns_[i];
}

void ColumnWindowAssembly::AddColumn(FlaschenTaschen *taschen) {
    columns_.push_back(taschen);
    width_ += 5;
    // height_ = std::max(height_, taschen->height());
}

void ColumnWindowAssembly::SetPixel(int x, int y, const Color &col) {
    if (x < 0 || x >= width() || y < 0 || y >= height())
        return;
    const int crate_from_left = GetColumnMiniWindow(x, y);
    FlaschenTaschen *column = columns_[columns_.size() - crate_from_left - 1];

    int true_height = GetHeightMiniWindow(x, y, height());
    // Our physical display has the (0,0) at the bottom right corner.
    // Flip it around.
    column->SetPixel(4 - x % 5, true_height, col);
}

void ColumnWindowAssembly::Send() {
    spi_->SendBuffers();
    usleep(50);  // WS2801 triggers on 50usec no data.
}
