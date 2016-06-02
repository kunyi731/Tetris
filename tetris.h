#include <array>
#include <memory>
#include <string>
#include <vector>

using std::array;
using std::vector;

struct Coord {
  int x;
  int y;
  Coord(int x, int y) : x(x), y(y) {};
  Coord operator+ (const Coord& other) {
    return Coord(x + other.x, y + other.y);
  }
  void operator+= (const Coord& other) {
    x += other.x;
    y += other.y;
  }  
  bool operator== (const Coord& other) {
    return x == other.x && y == other.y;
  }
};

using arr_4 = array<Coord, 4>; 

enum Action {
  DOWN,
  LEFT,
  RIGHT,
  ROTATE
};

enum PieceType {
  SHAPE_I,
  SHAPE_L,
  SHAPE_L_M,
  SHAPE_O,
  SHAPE_S,
  SHAPE_S_M,
  SHAPE_T,
  NUM_SHAPES
};

using Orientation = int;

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3
#define NUM_ORIENTATIONS 4

enum Color {
  RED,
  GREEN,
  BLUE,
  MAGENTA,
  CYAN,
  YELLOW,
  NUM_COLORS,
  EMPTY = NUM_COLORS
};

// Define a 4x4 grid in which all pieces (all type/orientation) will fit.
// Piece bottom center is the coordinate of
// [ ][ ][ ][ ]
// [ ][ ][ ][ ]
// [ ][ ][ ][ ]
// [ ][x][ ][ ]

class Piece {
 public:
  Piece(Coord bottom_center, PieceType type, Orientation orientation,
        Color color);
  void update(Action action);
  arr_4 getCoords() const;
  arr_4 getNextCoords(Action action) const;

	Color getColor() const;
  
 private:
  Coord bottom_center;
  PieceType type;
  Orientation orientation;
  Color color;
};

// One row of board is represented by an array of colors.
// Empty grid is marked by "EMPTY" color.
class BoardRow {
 public:
  BoardRow(int width);
  BoardRow(BoardRow&& row);
  BoardRow& operator=(BoardRow&& row);

  Color getColor(int offset) const;
  void setColor(int offset, Color c);
  bool isFull() const;
 private:
  // Disallow copy and assign.
  BoardRow(const BoardRow& row) = delete;
  BoardRow& operator=(const BoardRow& row) = delete;

  int width;
  std::unique_ptr<vector<Color>> colors;
  int num_filled;
};

class Board {
 public:
  Board(int height, int width);
  int getHeight() const;
  int getWidth() const;
  int getNumRows() const;
  Color getColor(int x, int y) const;
  Color getColor(Coord coord) const;
  bool isLineFull(int y) const;

  void addPiece(const Piece& p);
  void removeFullRows();
 private:
  int height;
  int width;
  vector<BoardRow> rows;
};

class Game {
 public:
  Game(int height, int width);

  const Board& getBoard() const;
  const Piece* getPiece() const;

  bool gameStarted() const; 
  bool gameOver() const; 

  void start();
  bool update(Action action, std::string& debug);
 private:
  void newPiece();
  bool detectConflict(const arr_4& coords) const;

  Board board;
  std::unique_ptr<Piece> piece;
  bool game_started = false;
  bool game_over = false;
};
