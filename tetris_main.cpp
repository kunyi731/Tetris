#include <chrono>
#include <mutex>
#include <string>
#include <thread>

#include "ncurses.h"
#include "tetris.h"

#define KEY_ESC 27

#define BLOCK_SIZE 2

#define GAME_WIDTH 10
#define GAME_HEIGHT 18

class GameWindow {
 public:
  GameWindow() {
    width = BLOCK_SIZE * GAME_WIDTH * 2;
    height = BLOCK_SIZE * GAME_HEIGHT;

    mvprintw(0, width / 2 - 2, "TETRIS");
    int start_y = 1;
    // Initializes the outer window.
    outer_win = newwin(height + 2, width + 2, start_y, 0);
    wbkgd(outer_win, COLOR_PAIR(1));
    box(outer_win, 0, 0);
    // Draw outer window.
    wrefresh(outer_win);

    // Initializes the inner board window.
    board_win = newwin(height, width, start_y + 1, 1);
    wbkgd(board_win, COLOR_PAIR(1));
    // Draw the initial board window.
    wrefresh(board_win);
  }

  ~GameWindow() {
    delwin(board_win);
    delwin(outer_win);
  }

  void Debug(std::string message, int line) {
    move(height + 3 + line, 0); 
    clrtobot();
    printw(message.c_str());
    refresh();
  }

  void Redraw(const Game& game) {
    werase(board_win);

    if (!game.gameStarted()) {
      mvwprintw(board_win, height / 2 - 1, width / 2 - 12, "Press any key to start.");
      wrefresh(board_win);
      return;
    }
    if (game.gameOver()) {
      mvwprintw(board_win, height / 2 - 1, width / 2 - 5, "Game Over!");
      wrefresh(board_win);
      return;
    }

    for (int y = 0; y < GAME_HEIGHT; y++) {
      for (int x = 0; x < GAME_WIDTH; x++) {
        FillBlock(x, y, game.getBoard().getColor(x, y), game.getBoard().isLineFull(y));
      }
    }
    const auto* piece = game.getPiece();
    if (piece != nullptr) {
      for (const auto& coord : piece->getCoords()) {
        FillBlock(coord.x, coord.y, game.getPiece()->getColor(), false);
      }
    }
    wrefresh(board_win); 
  }

 private:
  void FillBlock(int x, int y, Color c, bool should_blink) {
    if (x < 0 || y < 0 || x >= GAME_WIDTH || y >= GAME_HEIGHT) {
      return;
    }
    int color;
    switch (c) {
      case RED:
        color = 3;
        break;
      case GREEN:
        color = 4;
        break;
      case BLUE:
        color = 5;
        break;
      case MAGENTA:
        color = 6;
        break;
      case CYAN:
        color = 7;
        break;
      case YELLOW:
        color = 8;
        break;
      default:
        return;
    }
    wattron(board_win, COLOR_PAIR(color));
    for (int i = 0; i < BLOCK_SIZE; i++) {
      wmove(board_win, y * BLOCK_SIZE + i, x * BLOCK_SIZE * 2);
      for (int j = 0; j < BLOCK_SIZE * 2; j++) {
        waddch(board_win, ' ' | A_REVERSE | (should_blink ? A_BLINK : A_NORMAL));
      }
    }
    wattroff(board_win, COLOR_PAIR(color));
  }

  int width;
  int height;
  WINDOW* outer_win;
  WINDOW* board_win;
};

std::mutex mu;
void UpdateGame(Game& game, GameWindow& game_win, Action action, std::string& debug) {
  mu.lock();
  game.update(action, debug);
  game_win.Redraw(game);
  mu.unlock();
}

void NewGame() {
  // Initialize game.
  Game game(GAME_HEIGHT, GAME_WIDTH);
  // Initialize game window.
  GameWindow game_win;
  game_win.Redraw(game);

  // Wait for any key press to start.
  getch();
  game.start();

  bool quit = false;
  // Auto drop thread.
  std::thread t([&game, &game_win, &quit]() mutable {
    while (!quit && !game.gameOver()) {
      std::string debug;
      UpdateGame(game, game_win, Action::DOWN, debug);

      static int counter = 0;
      static char chars[]{'|', '/', '-', '\\'};
      std::string message = "Dropping..";
      message += chars[counter++ % 4];
      message += " " + debug;
      game_win.Debug(message, 0);

      std::this_thread::sleep_for(std::chrono::milliseconds(600));
    }
  });
  // Input listener.
  while (true) {
    int ch = getch();
    Action action;
    if (ch == KEY_UP) {
      action = Action::ROTATE;
    } else if (ch == KEY_LEFT) {
      action = Action::LEFT;
    } else if (ch == KEY_RIGHT) {
      action = Action::RIGHT;
    } else if (ch == KEY_DOWN) {
      action = Action::DOWN;
    } else {
      break;
    }
    std::string debug;
    UpdateGame(game, game_win, action, debug);
    game_win.Debug("User action: " + debug, 1);
  }
  quit = true;
  t.join();
}

int main() {
  // Initialize curses.
  initscr();  // Start curses mode.
  curs_set(0);  // Make cursor invisible.
  cbreak();  // Line buffering disabled.
	keypad(stdscr, TRUE);  // We get arrow keys etc...
	noecho();  // Don't echo() while we do getch.
  start_color();  // Use color.
  // General UI.
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  // For blocks.
  init_pair(3, COLOR_RED, COLOR_BLACK);
  init_pair(4, COLOR_GREEN, COLOR_BLACK);
  init_pair(5, COLOR_BLUE, COLOR_BLACK);
  init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(7, COLOR_CYAN, COLOR_BLACK);
  init_pair(8, COLOR_YELLOW, COLOR_BLACK);
  bkgd(COLOR_PAIR(1));
  
  refresh();  // Important!!

  while (true) {
    NewGame();
  }

  endwin();  // Exit curses mode.
	return 0;
}
