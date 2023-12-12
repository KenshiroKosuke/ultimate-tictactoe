#include "colors.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits.h>
#include <vector>
using namespace std;

#pragma once
namespace ai2 {

  int testDepth = 1;
  int focusDepth = 36;
  int depthForMaxPlayer = 8;
  int depthForMinPlayer = 2;
  int focusDepthStart = 38;
  int focusDepthEnd = 80;
  const char TIE_SYMBOL = 't';
  const char EMPTY_SYMBOL = '.';
  const int MAX_EVAL = 1023;
  const int MIN_EVAL = -MAX_EVAL;

  class Player {
  public:
    char symbol;
    bool isMaxPlayer;
    Player(char symbol, bool isMaxPlayer) {
      this->symbol = symbol;
      this->isMaxPlayer = isMaxPlayer;
    }
  };

  vector<ai2::Player> players = vector<Player>{Player('x', false), Player('o', true)};
  ;
  // AI is maxPlayer and also the second player: [1]
  char getPlayerSymbol(bool isMaxPlayer) { return players[isMaxPlayer ? 1 : 0].symbol; }

  struct Coordinate {
    size_t row;
    size_t col;
    void update(size_t n) { row = n; } // test
    friend bool operator==(Coordinate const &lhs, Coordinate const &rhs) {
      return lhs.row == rhs.row && lhs.col == rhs.col;
    };
    friend bool operator!=(Coordinate const &lhs, Coordinate const &rhs) {
      return lhs.row != rhs.row || lhs.col != rhs.col;
    };
  };
  const Coordinate COORDINATE_DEFAULT = {(size_t)1000000000, (size_t)1000000000};

  std::ostream &operator<<(std::ostream &outs, const Coordinate &vs) {
    return outs << "{" << vs.row << ", " << vs.col << "} ";
  }

  class GameBoard {
  public:
    // access via fullBoard [board's row] [board's col] [subboard's row] [subboard's col]
    // ?: empty
    vector<vector<vector<vector<char>>>> fullBoard;
    // ?: empty, t: tie
    vector<vector<char>> winnerBoard;
    char winner = EMPTY_SYMBOL;
    Coordinate subBoard = COORDINATE_DEFAULT;
    // @param size min: 3
    GameBoard(size_t size) {
      this->fullBoard = vector<vector<vector<vector<char>>>>(
          size, vector<vector<vector<char>>>(
                    size, vector<vector<char>>(size, vector<char>(size, EMPTY_SYMBOL))));
      this->winnerBoard = vector<vector<char>>(size, vector<char>(size, EMPTY_SYMBOL));
    }
    GameBoard(const GameBoard &gameBoard) {
      fullBoard = gameBoard.fullBoard;
      winnerBoard = gameBoard.winnerBoard;
      winner = gameBoard.winner;
      subBoard = gameBoard.subBoard;
      // moves = gameBoard.moves;
    }

    void print(Coordinate lastSubBoard = COORDINATE_DEFAULT,
               Coordinate lastMove = COORDINATE_DEFAULT) {
      // cout << lastMove.row << lastMove.col << endl;
      for (size_t bigRow = 0; bigRow < fullBoard.size(); bigRow++) {
        for (size_t smallRow = 0; smallRow < fullBoard.size(); smallRow++) {
          for (size_t col = 0; col < pow(fullBoard.size(), 2); col++) {
            if (col != 0 && col % fullBoard.size() == 0) {
              bool correctCol = false;
              if (subBoard.col == 0 && col == winnerBoard.size()) {
                correctCol = true;
              } else if (subBoard.col == winnerBoard.size() - 1 &&
                         col == winnerBoard.size() * (winnerBoard.size() - 1)) {
                correctCol = true;
              } else if (col == subBoard.col * winnerBoard.size() ||
                         col == subBoard.col * winnerBoard.size() * 2) {
                correctCol = true;
              }
              if (correctCol && subBoard.row == bigRow) {
                cout << RED << "| " << RESET;
              } else {
                cout << "| ";
              }
            }
            if (lastMove != COORDINATE_DEFAULT && bigRow == lastSubBoard.row &&
                (col / fullBoard.size()) == lastSubBoard.col && smallRow == lastMove.row &&
                col % fullBoard.size() == lastMove.col) {
              cout << RED
                   << fullBoard[bigRow][col / fullBoard.size()][smallRow][col % fullBoard.size()]
                   << RESET << " ";
            } else {
              cout << fullBoard[bigRow][col / fullBoard.size()][smallRow][col % fullBoard.size()]
                   << " ";
            }
          }
          cout << "\n";
        }
        if (bigRow != winnerBoard.size() - 1) {
          bool isCorrectRow = false;
          if (subBoard.row == 0) {
            if (bigRow == 0) {
              isCorrectRow = true;
              // cout << string(fullBoard.size() * 2, '=') +
              //           string((pow(fullBoard.size(), 2) - 1) * 2 - 1, '-') + "\n";
            }
          } else if (subBoard.row == winnerBoard.size() - 1) {
            if (bigRow == winnerBoard.size() - 2) {
              isCorrectRow = true;
              // cout << string((pow(fullBoard.size(), 2) - 1) * 2 - 1, '-') +
              //           string(fullBoard.size() * 2, '=') + "\n";
            }
          } else if ((bigRow == subBoard.row - 1) || (bigRow == subBoard.row)) {
            isCorrectRow = true;
          }
          if (isCorrectRow) {
            if (subBoard.col == 0) {
              cout << RED + string(fullBoard.size() * 2 + 1, '+') + RESET +
                          string(2 * pow(fullBoard.size(), 2) - 4, '-') + "\n";
            } else if (subBoard.col == winnerBoard.size() - 1) {
              cout << string(2 * pow(fullBoard.size(), 2) - 4, '-') + RED +
                          string(fullBoard.size() * 2 + 1, '+') + RESET + "\n";
            } else {
              cout << string((fullBoard.size() * 2 + 1) * subBoard.col + subBoard.col - 1, '-') +
                          RED + string(fullBoard.size() * 2 + 1, '+') + RESET +
                          string(fullBoard.size() * 2 * (fullBoard.size() - subBoard.col) - 3 -
                                     2 * subBoard.col,
                                 '-') +
                          "\n";
            }
          } else {
            cout << string((pow(fullBoard.size(), 2) + fullBoard.size() - 1) * 2 - 1, '-') + "\n";
            ;
          }
        }
      }
      cout << "\n";
    };

    void printSubBoard(size_t row, size_t col) {
      for (size_t i = 0; i < fullBoard.size(); i++) {
        for (size_t j = 0; j < fullBoard.size(); j++) {
          cout << fullBoard[row][col][i][j] << " ";
        }
        cout << "\n";
      }
      cout << "\n";
    }

    void printWinnerBoard() {
      for (size_t i = 0; i < winnerBoard.size(); i++) {
        for (size_t j = 0; j < winnerBoard.size(); j++) {
          cout << winnerBoard[i][j] << " ";
        }
        cout << "\n";
      }
      cout << "\n";
    }

    // ai don't need to call this to check, will call getLegalMoves to get child nodes instead
    bool isLegalMove(size_t row, size_t col, size_t subRow, size_t subCol) {
      // cout << "size: " << winnerBoard.size() << endl;
      // cout << "symbol now: " << fullBoard[row][col][subRow][subCol] << endl;
      // cout << "symbol empty: " << EMPTY_SYMBOL << endl;
      // const auto k = fullBoard[row][col][subRow][subCol] == EMPTY_SYMBOL;
      // cout << "comparision: " << k << endl;
      if (row >= winnerBoard.size() || col >= winnerBoard.size() || subRow >= winnerBoard.size() ||
          subCol >= winnerBoard.size()) {
        return false;
      }
      if ((subBoard.row != COORDINATE_DEFAULT.row) &&
          ((subBoard.row != row) || (subBoard.col != col))) {
        return false;
      }
      return fullBoard[row][col][subRow][subCol] == EMPTY_SYMBOL;
    }

    /**
     * Check if board is in a win state "after" a move "has been updated in the board".
     * It's important to note that this function only check if the "move" will cause the game to
     * end, thus it'll only calculate the results caused by that "move" being played.
     * "subBoard" is before getting updated by this move.
     * @param move: a move that has been already been made
     * @param playerSymbol: player that played the last move
     * @return bool: return true if win, else return false
     */
    bool isWinSubBoard(Coordinate move, char playerSymbol) {
      // horizontal
      for (size_t index = 0; index < winnerBoard.size(); index++) {
        if (fullBoard[subBoard.row][subBoard.col][move.row][index] != playerSymbol) {
          break;
        }
        if (index == winnerBoard.size() - 1) {
          return true;
        }
      }
      // vertical
      for (size_t index = 0; index < winnerBoard.size(); index++) {
        if (fullBoard[subBoard.row][subBoard.col][index][move.col] != playerSymbol) {
          break;
        }
        if (index == winnerBoard.size() - 1) {
          return true;
        }
      }
      // diagonal
      for (size_t index = 0; index < winnerBoard.size(); index++) {
        if (fullBoard[subBoard.row][subBoard.col][index][index] != playerSymbol) {
          break;
        }
        if (index == winnerBoard.size() - 1) {
          return true;
        }
      }
      for (size_t index = 0; index < winnerBoard.size(); index++) {
        if (fullBoard[subBoard.row][subBoard.col][index][winnerBoard.size() - 1 - index] !=
            playerSymbol) {
          break;
        }
        if (index == winnerBoard.size() - 1) {
          return true;
        }
      }
      return false;
    }

    // check after move being played to see if winnerBoard (or even winner) have to be updated
    bool isSubBoardFull() {
      for (size_t row = 0; row < winnerBoard.size(); row++) {
        for (size_t col = 0; col < winnerBoard.size(); col++) {
          if (fullBoard[subBoard.row][subBoard.col][row][col] == EMPTY_SYMBOL) {
            return false;
          }
        }
      }
      return true;
    }

    // check if there is no subboard left to play
    bool isBoardFull() {
      for (size_t row = 0; row < winnerBoard.size(); row++) {
        for (size_t col = 0; col < winnerBoard.size(); col++) {
          if (winnerBoard[row][col] == EMPTY_SYMBOL) {
            return false;
          }
        }
      }
      return true;
    }

    // Call this function when the player win subBoard to see if the game end.
    // Don't forget to update "winnerBoard" before calling this.
    // "subBoard" is where the move was made (before it's getting updated after this)
    char calculateWinner(char playerSymbol) {
      // horizontal
      for (size_t index = 0; index < winnerBoard.size(); index++) {
        if (winnerBoard[subBoard.row][index] != playerSymbol) {
          break;
        }
        if (index == winnerBoard.size() - 1) {
          return playerSymbol;
        }
      }
      // vertical
      for (size_t index = 0; index < winnerBoard.size(); index++) {
        if (winnerBoard[index][subBoard.col] != playerSymbol) {
          break;
        }
        if (index == winnerBoard.size() - 1) {
          return playerSymbol;
        }
      }
      // diagonal
      if (subBoard.row == subBoard.col) {
        for (size_t index = 0; index < winnerBoard.size(); index++) {
          if (winnerBoard[index][index] != playerSymbol) {
            break;
          }
          if (index == winnerBoard.size() - 1) {
            return playerSymbol;
          }
        }
      }
      if (subBoard.row == winnerBoard.size() - 1 - subBoard.col) {
        for (size_t index = 0; index < winnerBoard.size(); index++) {
          if (winnerBoard[index][winnerBoard.size() - 1 - index] != playerSymbol) {
            break;
          }
          if (index == winnerBoard.size() - 1) {
            return playerSymbol;
          }
        }
      }
      // check if the game end with a draw
      if (isBoardFull()) {
        return TIE_SYMBOL;
      }
      return EMPTY_SYMBOL;
    }

    // call this "after" the "subBoard" get update to the lastest subBoard by playMoveOnBoard
    auto getNextSubBoard(Coordinate lastMove) {
      while (true) {
        if (winnerBoard[lastMove.row][lastMove.col] == EMPTY_SYMBOL) {
          return lastMove;
        }
        lastMove.row = (lastMove.row + ((lastMove.col + 1) / fullBoard.size())) % fullBoard.size();
        lastMove.col = (lastMove.col + 1) % fullBoard.size();
      }
    }

    int getSubBoardValue(Coordinate targetSubBoard, bool isMaxPlayer) {
      int subBoardValue = 0;
      const char opponentSymbol = getPlayerSymbol(!isMaxPlayer);
      const char mySymbol = getPlayerSymbol(isMaxPlayer);
      int piece = 1;
      for (size_t col = 0; col < winnerBoard.size(); col++) {
        if (winnerBoard[targetSubBoard.row][col] == opponentSymbol) {
          piece = 0;
          break;
        } else if (winnerBoard[targetSubBoard.row][col] == mySymbol) {
          piece++;
        }
      }
      subBoardValue += piece;
      piece = 1;
      for (size_t row = 0; row < winnerBoard.size(); row++) {
        if (winnerBoard[row][targetSubBoard.col] == opponentSymbol) {
          piece = 0;
          break;
        } else if (winnerBoard[row][targetSubBoard.col] == mySymbol) {
          piece++;
        }
      }
      subBoardValue += piece;
      piece = 1;
      if (targetSubBoard.row == targetSubBoard.col) {
        for (size_t index = 0; index < winnerBoard.size(); index++) {
          if (winnerBoard[index][index] == opponentSymbol) {
            piece = 0;
            break;
          } else if (winnerBoard[index][index] == mySymbol) {
            piece++;
          }
        }
        subBoardValue += piece;
        piece = 1;
      }
      if (targetSubBoard.row == winnerBoard.size() - 1 - targetSubBoard.col) {
        for (size_t index = 0; index < winnerBoard.size(); index++) {
          if (winnerBoard[index][winnerBoard.size() - 1 - index] == opponentSymbol) {
            piece = 0;
            break;
          } else if (winnerBoard[index][winnerBoard.size() - 1 - index] == mySymbol) {
            piece++;
          }
        }
        subBoardValue += piece;
      }
      return subBoardValue;
    }

    int getSubBoardValue2(Coordinate targetSubBoard, bool isMaxPlayer) {
      int subBoardValue = 0;
      const char opponentSymbol = getPlayerSymbol(!isMaxPlayer);
      const char mySymbol = getPlayerSymbol(isMaxPlayer);
      int piece = 1;
      for (size_t col = 0; col < winnerBoard.size(); col++) {
        if (winnerBoard[targetSubBoard.row][col] == opponentSymbol) {
          piece = 0;
          break;
        } else if (winnerBoard[targetSubBoard.row][col] == mySymbol) {
          piece++;
        }
      }
      subBoardValue += piece;
      piece = 1;
      for (size_t row = 0; row < winnerBoard.size(); row++) {
        if (winnerBoard[row][targetSubBoard.col] == opponentSymbol) {
          piece = 0;
          break;
        } else if (winnerBoard[row][targetSubBoard.col] == mySymbol) {
          piece++;
        }
      }
      subBoardValue += piece;
      piece = 1;
      if (targetSubBoard.row == targetSubBoard.col) {
        for (size_t index = 0; index < winnerBoard.size(); index++) {
          if (winnerBoard[index][index] == opponentSymbol) {
            piece = 0;
            break;
          } else if (winnerBoard[index][index] == mySymbol) {
            piece++;
          }
        }
        subBoardValue += piece;
        piece = 1;
      }
      if (targetSubBoard.row == winnerBoard.size() - 1 - targetSubBoard.col) {
        for (size_t index = 0; index < winnerBoard.size(); index++) {
          if (winnerBoard[index][winnerBoard.size() - 1 - index] == opponentSymbol) {
            piece = 0;
            break;
          } else if (winnerBoard[index][winnerBoard.size() - 1 - index] == mySymbol) {
            piece++;
          }
        }
        subBoardValue += piece;
      }
      return subBoardValue;
    }
    /**
     * @brief - If play and win the game => top priority
     * @brief - If play and block the opponent from winning that subBoard +1 (for each opp's pieces
     * that we block)
     * @brief - If play and create the possible line to win that subBoard +1 (for our pieces in each
     * line)
     * @param move move to play
     * @param isMaxPlayer player that play
     * @param ply number of turn to look ahead
     * @param subBoardValue factor that will multiply lineScore
     */
    int evaluateAiMove(Coordinate move, bool isMaxPlayer, size_t ply, int subBoardValue) {
      const char playerSymbol = getPlayerSymbol(isMaxPlayer);
      const char opponentSymbol = getPlayerSymbol(!isMaxPlayer);
      int score = 0;
      const auto originalMoveSymbol = fullBoard[subBoard.row][subBoard.col][move.row][move.col];
      const auto originalWinnerSymbol = winnerBoard[subBoard.row][subBoard.col];
      fullBoard[subBoard.row][subBoard.col][move.row][move.col] = playerSymbol;
      // stop if 1) win game 2) tie immediately (last empty cell that subboard)
      bool isNotEnd = false;
      if (isWinSubBoard(move, playerSymbol)) {
        winnerBoard[subBoard.row][subBoard.col] = playerSymbol;
        char newWinner = calculateWinner(playerSymbol);
        if (newWinner == playerSymbol) {
          score = isMaxPlayer ? MAX_EVAL : MIN_EVAL;
        } else if (newWinner == TIE_SYMBOL) {
          score = 0;
        } else {
          score = isMaxPlayer ? subBoardValue : -subBoardValue; // extra score
          isNotEnd = true;
        }
      } else if (!isSubBoardFull()) {
        isNotEnd = true;
      }
      if (isNotEnd) {
        int lineScore = 0;
        // horizontal
        int me = 0;
        int them = 0;
        for (size_t col = 0; col < winnerBoard.size(); col++) {
          if (fullBoard[subBoard.row][subBoard.col][move.row][col] == playerSymbol) {
            me++;
          } else if (fullBoard[subBoard.row][subBoard.col][move.row][col] == opponentSymbol) {
            them++;
          }
        }
        if (me < 2) {
          lineScore += them; // prevent opponent from winning
        }
        if (them == 0) {
          lineScore += me; // help us winning
        }
        // vertical
        me = 0;
        them = 0;
        for (size_t row = 0; row < winnerBoard.size(); row++) {
          if (fullBoard[subBoard.row][subBoard.col][row][move.col] == playerSymbol) {
            me++;
          } else if (fullBoard[subBoard.row][subBoard.col][row][move.col] == opponentSymbol) {
            them++;
          }
        }
        if (me < 2) {
          lineScore += them;
        }
        if (them == 0) {
          lineScore += me;
        }
        // diag
        me = 0;
        them = 0;
        if (move.row == move.col) {
          for (size_t index = 0; index < winnerBoard.size(); index++) {
            if (fullBoard[subBoard.row][subBoard.col][index][index] == playerSymbol) {
              me++;
            } else if (fullBoard[subBoard.row][subBoard.col][index][index] == opponentSymbol) {
              them++;
            }
          }
          if (me < 2) {
            lineScore += them;
          }
          if (them == 0) {
            lineScore += me;
          }
          me = 0;
          them = 0;
        }
        if (move.row == winnerBoard.size() - 1 - move.col) {
          for (size_t index = 0; index < winnerBoard.size(); index++) {
            if (fullBoard[subBoard.row][subBoard.col][index][winnerBoard.size() - 1 - index] ==
                playerSymbol) {
              me++;
            } else if (fullBoard[subBoard.row][subBoard.col][index]
                                [winnerBoard.size() - 1 - index] == opponentSymbol) {
              them++;
            }
          }
          if (me < 2) {
            lineScore += them;
          }
          if (them == 0) {
            lineScore += me;
          }
        }

        // max gain = +- 4 * size * subBoardValue
        score += (isMaxPlayer ? lineScore : -lineScore) * subBoardValue;

        if (ply != 0) {
          const auto tmp = subBoard;
          subBoard = getNextSubBoard(move);
          const int nextSubBoardValue = getSubBoardValue(subBoard, !isMaxPlayer);
          int scoreFromNextPlayer = isMaxPlayer ? INT_MAX : INT_MIN;
          for (size_t subrow = 0; subrow < winnerBoard.size(); subrow++) {
            for (size_t subcol = 0; subcol < winnerBoard.size(); subcol++) {
              if (fullBoard[subBoard.row][subBoard.col][subrow][subcol] == EMPTY_SYMBOL) {
                // get best score of next move for opponent: if current are max => want min
                if (isMaxPlayer) {
                  scoreFromNextPlayer = min(
                      evaluateAiMove({subrow, subcol}, !isMaxPlayer, ply - 1, nextSubBoardValue),
                      scoreFromNextPlayer);
                } else {
                  scoreFromNextPlayer = max(
                      evaluateAiMove({subrow, subcol}, !isMaxPlayer, ply - 1, nextSubBoardValue),
                      scoreFromNextPlayer);
                }
              }
            }
          }
        // foundBestChild:
          score += scoreFromNextPlayer;
          subBoard = tmp;
        }
      }
      // Reset board state
      fullBoard[subBoard.row][subBoard.col][move.row][move.col] = originalMoveSymbol;
      winnerBoard[subBoard.row][subBoard.col] = originalWinnerSymbol;
      return score;
    }
    /**
     * AI player call this "after" the "subBoard" get update to the lastest subBoard by
     * playMoveOnBoard playMoveOnBoard will also ensure that there's no case where all subboards are
     * full (thus, not stuck in an inifinite loop)
     * @param ply if ply = 1, it will look ahead for 1 turn (current turn, don't check opponent)
     * if ply=2, it will check the next opponent's move.
     */
    vector<pair<int, Coordinate>> getLegalMovesOnTheCurrentSubBoard(bool isMaxPlayer, size_t ply) {
      vector<pair<int, Coordinate>> scores;
      int index = 0;
      const auto factor = getSubBoardValue(subBoard, isMaxPlayer);
      for (size_t subrow = 0; subrow < winnerBoard.size(); subrow++) {
        for (size_t subcol = 0; subcol < winnerBoard.size(); subcol++) {
          if (fullBoard[subBoard.row][subBoard.col][subrow][subcol] == EMPTY_SYMBOL) {
            // if (testDepth == focusDepth) {
            //   cout << "enter" << subrow << subcol << " ply: " << ply - 1 << "\n";
            // }
            scores.push_back(
                make_pair(evaluateAiMove({subrow, subcol}, isMaxPlayer, ply - 1, factor),
                          Coordinate{subrow, subcol}));
          }
        }
      }
      if (getPlayerSymbol(isMaxPlayer) == players[1].symbol) {
        sort(scores.begin(), scores.end(),
             [](pair<int, Coordinate> &a, pair<int, Coordinate> &b) { return a.first > b.first; });
      } else {
        sort(scores.begin(), scores.end(),
             [](pair<int, Coordinate> &a, pair<int, Coordinate> &b) { return a.first < b.first; });
      }
      return scores;
    }

    /**
     * Update fullboard (and winnerboard status and winner if needed).
     */
    void playMoveOnBoard(Coordinate move, char playerSymbol) {
      fullBoard[subBoard.row][subBoard.col][move.row][move.col] = playerSymbol;
      // check win and update status (only check for current player's symbol)
      if (isWinSubBoard(move, playerSymbol)) {
        winnerBoard[subBoard.row][subBoard.col] = playerSymbol;
        winner = calculateWinner(playerSymbol); // also internally call isBoardFull
      } else {
        // check to see if tie in that subboard and update in winnerBoard
        if (isSubBoardFull()) {
          winnerBoard[subBoard.row][subBoard.col] = TIE_SYMBOL;
          // also check if the game end because that's the only subBoard left
          if (isBoardFull()) {
            winner = TIE_SYMBOL;
          }
          // **** ADD NEW RULE: if place the last panel of that subboard, the game end in a tie.
          winner = TIE_SYMBOL;
        }
      }
      if (winner == EMPTY_SYMBOL) {
        subBoard = getNextSubBoard(move);
      }
      return;
    }

    auto evaluateWinnerBoard() {
      char symbol = EMPTY_SYMBOL;
      const char maxSymbol = getPlayerSymbol(true);
      int score = 0;
      int piece = 0; // the piece in each row (the greater, the more valuable)
      // horizontally
      for (size_t row = 0; row < winnerBoard.size(); row++) {
        piece = 0;
        for (size_t col = 0; col < winnerBoard.size(); col++) {
          if (winnerBoard[row][col] != EMPTY_SYMBOL) {
            if (symbol != EMPTY_SYMBOL && winnerBoard[row][col] != symbol) {
              symbol = EMPTY_SYMBOL;
              break;
            }
            symbol = winnerBoard[row][col];
            piece += 1;
          }
        }
        if (symbol != EMPTY_SYMBOL) {
          score += (symbol == maxSymbol)? piece: -piece;
          symbol = EMPTY_SYMBOL;
        }
      }
      // vertically
      for (size_t col = 0; col < winnerBoard.size(); col++) {
        piece = 0;
        for (size_t row = 0; row < winnerBoard.size(); row++) {
          if (winnerBoard[row][col] != EMPTY_SYMBOL) {
            if (symbol != EMPTY_SYMBOL && winnerBoard[row][col] != symbol) {
              symbol = EMPTY_SYMBOL;
              break;
            }
            symbol = winnerBoard[row][col];
            piece += 1;
          }
        }
        if (symbol != EMPTY_SYMBOL) {
          score += (symbol == maxSymbol)? piece: -piece;
          symbol = EMPTY_SYMBOL;
        }
      }
      // diagonally
      for (size_t index = 0; index < winnerBoard.size(); index++) {
        piece = 0;
        if (winnerBoard[index][index] != EMPTY_SYMBOL) {
          if (symbol != EMPTY_SYMBOL && winnerBoard[index][index] != symbol) {
            symbol = EMPTY_SYMBOL;
            break;
          }
          symbol = winnerBoard[index][index];
          piece += 1;
        }
      }
      if (symbol != EMPTY_SYMBOL) {
        score += (symbol == maxSymbol)? piece: -piece;
        symbol = EMPTY_SYMBOL;
      }
      for (size_t index = 0; index < winnerBoard.size(); index++) {
        piece = 0;
        if (winnerBoard[index][winnerBoard.size() - 1 - index] != EMPTY_SYMBOL) {
          if (symbol != EMPTY_SYMBOL &&
              winnerBoard[index][winnerBoard.size() - 1 - index] != symbol) {
            symbol = EMPTY_SYMBOL;
            break;
          }
          symbol = winnerBoard[index][winnerBoard.size() - 1 - index];
          piece += 1;
        }
      }
      if (symbol != EMPTY_SYMBOL) {
        score += (symbol == maxSymbol)? piece: -piece;
      }
      return score;
    }
  };
}