#include <iostream>

#include "tetris.h"

// Piece coordinate maps.
// Each piece is array of 4 coords.
// 7 piece types x 4 orientations x 4 coords.
const arr_4 coord_map[NUM_SHAPES][NUM_ORIENTATIONS] = {
  {
    {Coord{-1, -1}, Coord{0, -1}, Coord{1, -1}, Coord{2, -1}},
    {Coord{0, -3}, Coord{0, -2}, Coord{0, -1}, Coord{0, 0}},
    {Coord{-1, -1}, Coord{0, -1}, Coord{1, -1}, Coord{2, -1}},
    {Coord{0, -3}, Coord{0, -2}, Coord{0, -1}, Coord{0, 0}}
  },
  {
    {Coord{0, -2}, Coord{0, -1}, Coord{0, 0}, Coord{1, 0}},
    {Coord{-1, -1}, Coord{0, -1}, Coord{1, -1}, Coord{-1, 0}},
    {Coord{0, -2}, Coord{1, -2}, Coord{1, -1}, Coord{1, 0}},
    {Coord{1, -1}, Coord{-1, 0}, Coord{0, 0}, Coord{1, 0}}
  },
  {
    {Coord{1, -2}, Coord{1, -1}, Coord{0, 0}, Coord{1, 0}},
    {Coord{-1, -1}, Coord{-1, 0}, Coord{0, 0}, Coord{1, 0}},
    {Coord{0, -2}, Coord{1, -2}, Coord{0, -1}, Coord{0, 0}},
    {Coord{-1, -1}, Coord{0, -1}, Coord{1, -1}, Coord{1, 0}}
  },
  {
    {Coord{0, -1}, Coord{1, -1}, Coord{0, 0}, Coord{1, 0}},
    {Coord{0, -1}, Coord{1, -1}, Coord{0, 0}, Coord{1, 0}},
    {Coord{0, -1}, Coord{1, -1}, Coord{0, 0}, Coord{1, 0}},
    {Coord{0, -1}, Coord{1, -1}, Coord{0, 0}, Coord{1, 0}}
  },
  {
    {Coord{0, -1}, Coord{1, -1}, Coord{-1, 0}, Coord{0, 0}},
    {Coord{0, -2}, Coord{0, -1}, Coord{1, -1}, Coord{1, 0}},
    {Coord{0, -1}, Coord{1, -1}, Coord{-1, 0}, Coord{0, 0}},
    {Coord{0, -2}, Coord{0, -1}, Coord{1, -1}, Coord{1, 0}}
  },
  {
    {Coord{-1, -1}, Coord{0, -1}, Coord{0, 0}, Coord{1, 0}},
    {Coord{1, -2}, Coord{0, -1}, Coord{1, -1}, Coord{0, 0}},
    {Coord{-1, -1}, Coord{0, -1}, Coord{0, 0}, Coord{1, 0}},
    {Coord{1, -2}, Coord{0, -1}, Coord{1, -1}, Coord{0, 0}}
  },
  {
    {Coord{0, -1}, Coord{-1, 0}, Coord{0, 0}, Coord{1, 0}},
    {Coord{-1, -2}, Coord{-1, -1}, Coord{0, -1}, Coord{-1, 0}},
    {Coord{-1, -1}, Coord{0, -1}, Coord{1, -1}, Coord{0, 0}},
    {Coord{1, -2}, Coord{0, -1}, Coord{1, -1}, Coord{1, 0}}
  }
};

// Helper: get the next DOWN/LEFT/RIGHT of certain coord.
// Get the next orientation if action is ROTATE.
void update(Coord& c, Orientation& o, Action action) {
  if (action == DOWN) {
    c += Coord{0, 1};
  } else if (action == LEFT) {
    c += Coord{-1, 0};
  } else if (action == RIGHT) {
    c += Coord{1, 0};
  } else if (action == ROTATE) {
    o = (o + 1) % NUM_ORIENTATIONS;
  } 
}

// Helper: get coords mapped to certain piece shape + orientation.
arr_4 getCoords(PieceType type, Orientation orientation, Coord offset) {
  auto coords = coord_map[type][orientation];
  for (auto& c : coords) {
    c += offset;
  }
  return coords;
}

// Constructor for Piece class.
Piece::Piece(Coord bottom_center, PieceType type, Orientation orientation, Color color) :
    bottom_center(bottom_center), type(type), orientation(orientation), color(color) {};

void Piece::update(Action action) {
  ::update(bottom_center, orientation, action);
}

arr_4 Piece::getCoords() const {
  return ::getCoords(type, orientation, bottom_center);
}

arr_4 Piece::getNextCoords(Action action) const {
  auto next_bottom_center = bottom_center;
  auto next_orientation = orientation;
  ::update(next_bottom_center, next_orientation, action);

  return ::getCoords(type, next_orientation, next_bottom_center);
}

Color Piece::getColor() const {
  return color;
}

/*========== BoardRow ==========*/

// Constructor.
BoardRow::BoardRow(int width) : width(width), num_filled(0) {
  colors.reset(new vector<Color>(width, EMPTY)); 
};

BoardRow::BoardRow(BoardRow&& other) : width(other.width),
                                       num_filled(other.num_filled) {
  colors.reset(other.colors.release());
}

BoardRow& BoardRow::operator=(BoardRow&& other) {
  colors.reset(other.colors.release());
  num_filled = other.num_filled;
  return *this;
}

// Returns color for this row for certain offset.
Color BoardRow::getColor(int offset) const {
  return (*colors)[offset];
}

// Sets color given offset.
void BoardRow::setColor(int offset, Color c) {
  num_filled += (c == EMPTY) ? 0 : 1;
  (*colors)[offset] = c;
}

bool BoardRow::isFull() const {
  return num_filled == width;
}

/*========== Board ==========*/
Board::Board(int height, int width) : height(height), width(width), rows() {};

int Board::getHeight() const {
  return height;
}

int Board::getWidth() const {
  return width;
}

int Board::getNumRows() const {
  return rows.size();
}

Color Board::getColor(int x, int y) const {
  int row_idx = height - y - 1;
  if (row_idx >= getNumRows()) {
    return EMPTY;
  }
  return rows[row_idx].getColor(x);
}

Color Board::getColor(Coord coord) const {
  return getColor(coord.x, coord.y);
}

bool Board::isLineFull(int y) const {
  int row_idx = height - y - 1;
  if (row_idx >= getNumRows()) {
    return false;
  }
  return rows[height - y - 1].isFull();
}

void Board::addPiece(const Piece& p) {
  for (const auto& coord : p.getCoords()) {
    int row_idx = height - coord.y - 1;
    while (row_idx >= getNumRows()) {
      rows.emplace_back(width);
    }
    rows[row_idx].setColor(coord.x, p.getColor());
  }
}

void Board::removeFullRows() {
  rows.erase(std::remove_if(rows.begin(), rows.end(), [](BoardRow& row) {
    return row.isFull();
  }), rows.end());
}

/*========== Game ==========*/
Game::Game(int height, int width) : board(height, width) {}

const Board& Game::getBoard() const {
  return board;
}

const Piece* Game::getPiece() const {
  return piece.get();
}

bool Game::gameStarted() const {
  return game_started;
}

bool Game::gameOver() const {
  return game_over;
}

void Game::start() {
  game_started = true;
  newPiece();
}

// Main function of game update logic.
bool Game::update(Action action, std::string& debug) {
  if (game_over) {
    return false;
  }
  // If any row is full, remove all colors.
  board.removeFullRows();

  // If piece is consumed, generate a new random piece.
  if (!piece.get()) {
    newPiece();
    debug += " new piece";
    if (detectConflict(piece->getCoords())) {
      debug += " game over";
      game_over = true;
      return false;
    }
  }
  auto next_coords = piece->getNextCoords(action);
  if (!detectConflict(next_coords)) {
    debug += " no conflict";
    piece->update(action);
  } else if (action == Action::DOWN) {
    debug += " piece landed";
    board.addPiece(*piece);
    piece.reset(nullptr);
  }
  return true;
}

bool Game::detectConflict(const arr_4& coords) const {
  for (const auto& coord : coords) {
    if (coord.x < 0 || coord.x >= board.getWidth() || coord.y >= board.getHeight()) {
      return true;
    }
    if (board.getColor(coord) != Color::EMPTY) {
      return true;
    }
  }
  return false;
}

void Game::newPiece() {
  Coord p(board.getWidth() / 2 - 1, -1);
  PieceType rand_type = static_cast<PieceType>(rand() % NUM_SHAPES);
  Orientation rand_orientation = rand() % NUM_ORIENTATIONS;
  Color rand_color = static_cast<Color>(rand() % NUM_COLORS);
  piece.reset(new Piece(p, rand_type, rand_orientation, rand_color));
}
