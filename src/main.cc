#include "minesweeper.h"

#include <array>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <queue>
#include <random>
#include <thread>
#include <utility>

#include <raylib.h>

enum class TileState : uint8_t {
  STATE_INVISIBLE = 0,
  STATE_CLICKED = 1,
  STATE_FLAGGED = 2,
  STATE_HOVER = 4
};

enum class GameState : uint8_t {
  GAME_UNINIT = 0,
  GAME_PROGRESS = 1,
  GAME_WIN = 2,
  GAME_LOSS = 3,
};

struct Tile {
  uint16_t x;
  uint16_t y;
  TileState state;
  uint8_t val;
};

struct Config {
  uint8_t rows;
  uint8_t cols;
  uint8_t validEmptyCells;
  uint8_t n_mines;
};

static inline Config config;

using Board = std::vector<std::vector<Tile>>;

static constexpr std::array<std::array<int8_t, 2>, 8> dirs = {{
  {0, 1}, // mid - down
  {0, -1}, // mid - up
  {1, 0}, // right - mid
  {1, 1}, // right - down
  {1, -1}, // right - up
  {-1, 0}, // left - mid
  {-1, -1}, // left - up
  {-1, 1}, // left - down
}};

inline uint32_t getRand(const uint32_t s, const uint32_t e) {
  std::uniform_int_distribution<int>  Distribution(s, e);
  std::random_device rd;
  std::mt19937 mt(rd());
  return Distribution(mt);
}

inline std::pair<uint16_t, uint16_t> getRandCoord(const uint32_t rows, const uint32_t cols) {
  return std::pair(getRand(0, rows - 1), getRand(0, cols - 1));
}

bool validate_board(const auto board, Tile start) {

  auto x = start.x;
  auto y = start.y;

  auto board_checked = board;
  // Zero out board except starting coord
  for(auto &i : board_checked) {
    for(auto j : i) {
      if(j.val < 253) j.val = 0;
    }
  }

  std::vector<uint8_t> adjescent;
  while(true) {
    for(int i = 0; i < 8; ++i) {
      auto cx = x + dirs[i][0];
      auto cy = y + dirs[i][1];
    }

    break;
  }
  return true;
}

bool within_board(const uint16_t x, const uint16_t y) {
  return (x >= 0 && x >= 0 && x < config.rows  && y < config.cols );
}

bool validate_mine(const auto& board, const uint16_t x, const uint16_t y) {
  for(int i = 0; i < 8; ++i) {
    auto cx = x + dirs[i][0];
    auto cy = y + dirs[i][1];
    if (within_board(cx, cy)) {
      if (board[cx][cy].val + 1 > config.validEmptyCells && (board[cx][cy].val != 254 || board[cx][cy].val == 253)) return false;
    }
  }
  return true;
}

void generate_board(auto& board, Tile start) {
  uint8_t mines_placed = 0;
  while ((mines_placed) <= config.n_mines) {
    auto [x, y] = getRandCoord(config.rows , config.cols );
    if(board[x][y].val == 254 || board[x][y].val == 253) continue;
    if(validate_mine(board, x, y)) {
      board[x][y].val = 254;
      board[x][y].state = TileState::STATE_INVISIBLE;
      mines_placed++;
      for(int i = 0; i < 8; ++i) {
        auto cx = x + dirs[i][0];
        auto cy = y + dirs[i][1];
        if (within_board(cx, cy) && board[cx][cy].val != 254 && board[cx][cy].val != 253) {
          board[cx][cy].val++;
        }
      }
    }
  }

  board[start.x][start.y].val = 0;
  // Get value for start tile
  for(int i = 0; i < 8; ++i) {
    auto cx = start.x + dirs[i][0];
    auto cy = start.y + dirs[i][1];
    if (within_board(cx, cy) && board[cx][cy].val == 254) {
      board[start.x][start.y].val += 1;
    }
  }
}

void draw_board(auto &board, double timer, GameState state, uint16_t count) {
  ClearBackground(RAYWHITE);
  BeginDrawing();
  for(int i = 0; i < board.size(); ++i) {
    for(int j = 0; j < board[0].size(); ++j) {
      switch(board[i][j].state) {
        case TileState::STATE_INVISIBLE:
          DrawRectangleV(
              {.x = 40.f * (i ), .y = 40.f * (j ) },
              {.x = 30, .y = 30},
              GRAY
              );
          break;
        case TileState::STATE_CLICKED:
          DrawRectangleV(
              {.x = (i ) * 40.f, .y = (j ) * 40.f  },
              {.x = 30, .y = 30},
              LIGHTGRAY
              );

          switch(board[i][j].val) {
            case 1:
              DrawText(std::to_string(board[i][j].val).c_str(), 40 * i, 40 * j, 22, BLUE);
              break;
            case 2:
              DrawText(std::to_string(board[i][j].val).c_str(), 40 * i, 40 * j, 22, GREEN);
              break;
            case 3:
              DrawText(std::to_string(board[i][j].val).c_str(), 40 * i, 40 * j, 22, RED);
              break;
            case 4:
              DrawText(std::to_string(board[i][j].val).c_str(), 40 * i, 40 * j, 22, PURPLE);
              break;
            case 5:
              DrawText(std::to_string(board[i][j].val).c_str(), 40 * i, 40 * j, 22, PINK);
              break;
            case 6:
              DrawText(std::to_string(board[i][j].val).c_str(), 40 * i, 40 * j, 22, ORANGE);
              break;

          }
          break;
        case TileState::STATE_FLAGGED:
          DrawRectangleV(
              {.x = (i ) * 40.f, .y = (j ) * 40.f  },
              {.x = 30, .y = 30},
              RED
              );
          break;
        case TileState::STATE_HOVER:
          DrawRectangleV(
              {.x = (i ) * 40.f, .y = (j ) * 40.f  },
              {.x = 30, .y = 30},
              BLUE
              );
          break;
      }
    }
  }
  DrawRectangleV({0, config.cols * 40.f}, {config.rows * 40.f, 80}, DARKGRAY);
  DrawText(std::to_string(timer).c_str(), (config.rows * 40.f) / 2, config.cols * 80 / 2, 20, RED);
  if(state == GameState::GAME_WIN) {
    DrawText("YOU WON!!!", (config.rows * 40.f) / 4, config.cols * 80 / 2, 20, YELLOW);
  } else if(state == GameState::GAME_LOSS) {
    DrawText("YOU LOSE.", (config.rows * 40.f) / 4, config.cols * 80 / 2, 20, YELLOW);
  }
  DrawText((std::to_string(count) + "/" + std::to_string(config.cols * config.rows - config.n_mines - 1)).c_str(), 
      (config.rows * 40.f / 4) * 3, config.cols * 80 / 2, 20, WHITE);
  EndDrawing();
}

uint16_t bfs_click_zeroes(auto &board, uint16_t start_x, uint16_t start_y) {
  std::queue<std::pair<uint16_t, uint16_t>> q;
  uint16_t total = 0;
  q.push({start_x, start_y});
  while(!q.empty()) {
    auto [x, y] = q.front();
    q.pop();
    for(int i = 0; i < dirs.size(); ++i) {
      auto cx = x + dirs[i][0];
      auto cy = y + dirs[i][1];

      if(within_board(cx, cy)) {
        if(board[cx][cy].state != TileState::STATE_CLICKED && board[cx][cy].val == 0) {
          board[cx][cy].state = TileState::STATE_CLICKED;
          total++;
          q.push({cx, cy});
        } else if(board[cx][cy].val <= config.validEmptyCells && board[cx][cy].state != TileState::STATE_CLICKED) {
          board[cx][cy].state = TileState::STATE_CLICKED;
          total++;
        }
      }
    }
  }
  return total;
}

void game_loop() {
  Board board;
  board.resize(config.rows);
  double start_time = 0;
  double cur_time = 0;
  GameState state = GameState::GAME_UNINIT;

  uint16_t clicked = 0;
  for(auto &i : board) {
    i.resize(config.cols);
  }
  bool started = false;
  while(!WindowShouldClose()) {
    if(GetKeyPressed() == KEY_R) {
      // Restart the game (clear the board)
      for(auto &i : board) {
        std::fill(i.begin(), i.end(), std::move(Tile {
            .x = 0,
            .y = 0,
            .val = 0
            }));
      }
      state = GameState::GAME_UNINIT;
      start_time = 0;
      cur_time = 0;
      clicked = 0;
    }

    auto mouse_coords = GetMousePosition();
    const uint16_t x = mouse_coords.x / 40;
    const uint16_t y = mouse_coords.y / 40;

    if(state != GameState::GAME_LOSS && state != GameState::GAME_WIN) {
      if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (state == GameState::GAME_PROGRESS) {
          // Continue in progress game
          if(board[x][y].val == 254) {
            state = GameState::GAME_LOSS;
            start_time = 0;
          } else if (board[x][y].state != TileState::STATE_CLICKED) {

            // A click on a tile occured. We will then BFS all 0 TileState
            // and set them to Clicked as well.
            if(board[x][y].state != TileState::STATE_FLAGGED){
              clicked++;
            }
            board[x][y].state = TileState::STATE_CLICKED;
            if(board[x][y].val == 0) {
              clicked += bfs_click_zeroes(board, x, y);
            }
          }
        } else {
          // Generate a new board based on the player's first click
          board[x][y].val = 253;
          generate_board(board, Tile {
              .x = x,
              .y = y,
              .state = TileState::STATE_CLICKED,
              .val = 254,
              });
          board[x][y].state = TileState::STATE_CLICKED;
          validate_board(board, Tile{ .x= x, .y = y});
          state = GameState::GAME_PROGRESS;
          // Start the game timer
          start_time = GetTime();
          cur_time = 0;
          clicked = 1;
          if(board[x][y].val == 0) {
            clicked += bfs_click_zeroes(board, x, y);
          }
        }
      } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && state == GameState::GAME_PROGRESS) {
        if (board[x][y].state != TileState::STATE_FLAGGED) {
          board[x][y].state = TileState::STATE_FLAGGED;
        }
      }
    }
    if(clicked == config.cols * config.rows - config.n_mines - 1) {
      state = GameState::GAME_WIN;
    } else if (state == GameState::GAME_PROGRESS){
      cur_time = GetTime();
    }
    draw_board(board, cur_time - start_time, state, clicked);
    }
}

int main() {
  config.rows = 16;
  config.cols = 10;
  config.validEmptyCells = 5;
  config.n_mines = 30;
  InitWindow(config.rows * 40, config.cols * 40 + 80, "minesweeper");
  std::cout << "sizeof: " << sizeof(Tile) << "\n";
  game_loop();
}
