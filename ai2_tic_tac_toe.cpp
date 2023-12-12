// C_Cpp.clang_format_fallbackStyle => BreakBeforeBraces: Attach, all width 2, column 100
/**
 * RULE
 * 1. normal tictactoe but inside each panels are another board of tictactoe.
 * 2. if you play a move on any subBoard, your opponent is forced to play on the subboard indicated
 * by the coordinate you just place. exception: if that subboard is full, player must play the next
 * subboard to the right, if it's full then the next one, and so on. (this will make branching
 * factor <= 9 at all state)
 */
#include "board_tools.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits.h>
#include <vector>

using namespace std;
using namespace ai2;

size_t max_depth = 40;

struct MoveRecord {
  Coordinate subBoard;
  Coordinate move;
  char playerSymbol;
  MoveRecord(const MoveRecord &other)
      : move(other.move), subBoard(other.subBoard), playerSymbol(other.playerSymbol){};
  MoveRecord(Coordinate subBoard, Coordinate move, char playerSymbol)
      : move(move), subBoard(subBoard), playerSymbol(playerSymbol){};
};

struct Node {
public:
  GameBoard gameState;
  // vector<Move> moveHistory; // can just store last move for memory and performance reason
  size_t depth;
  MoveRecord lastMove = {COORDINATE_DEFAULT, COORDINATE_DEFAULT, EMPTY_SYMBOL};
  // Node(const GameBoard gameState) : gameState(gameState) { moveHistory = vector<Move>(); }
  Node(const Node &state)
      : gameState(state.gameState), depth(state.depth), lastMove(state.lastMove) {}
  Node(const GameBoard gameState, size_t depth) : gameState(gameState), depth(depth) {
    // this->lastMove = Move({COORDINATE_DEFAULT,COORDINATE_DEFAULT,EMPTY_SYMBOL});
  }
  // DON'T CHANGE GAME STATE HERE. BUG PRONED.
  void updateNodeRecord(char playerSymbol, Coordinate move,
                        Coordinate subBoard = COORDINATE_DEFAULT) {
    depth += 1;
    lastMove.move = move;
    lastMove.playerSymbol = playerSymbol;
    if (subBoard == COORDINATE_DEFAULT) {
      lastMove.subBoard = gameState.subBoard;
    } else {
      lastMove.subBoard = subBoard;
    }
  }
};

int leafNodeCount = 0;
int pruneCount = 0;
int depth = 1;
bool prune40 = false;
bool prune30 = false;

/**
 * Minimax with alpha-beta-pruning work like this:
 * - The alpha (set to min) and beta (set to max) will pass from parent to child
 * - The MAX node will try to find the max eval value that exceed the beta. This is because, the
 * MAX node will definitely play the path with high value, but the MAX's parent, which is MIN, has
 * already find another MAX child with lower value (which is passed down to MAX as beta). That MAX
 * will not be chosen as the true successor by MIN and we can prune this MAX node, backtracking to
 * the MIN parent and expand other MIN's childnodes instead.
 */
int minimax(GameBoard &gameboard, int alpha, int beta, bool isMaxPlayer) {
  char playerSymbol = getPlayerSymbol(isMaxPlayer);
  if (gameboard.winner == TIE_SYMBOL) {
    leafNodeCount++;
    cout << "TIE";
    return 0;
  } else if (gameboard.winner != EMPTY_SYMBOL) {
    leafNodeCount++;
    return gameboard.winner == getPlayerSymbol(true) ? MAX_EVAL : MIN_EVAL;
  }
  // else if (depth >= 48) {
  //   return gameboard.evaluateWinnerBoard();
  // }
  Coordinate subBoardTmp = gameboard.subBoard;
  const char winnerTmp = gameboard.winner;
  const char winnerBoardTmp = gameboard.winnerBoard[subBoardTmp.row][subBoardTmp.col];
  const auto playableMoves = gameboard.getLegalMovesOnTheCurrentSubBoard(playerSymbol, 5);
  if (isMaxPlayer) {
    auto maxVal = INT_MIN;
    for (auto const &move : playableMoves) {
      const char fullBoardTmp =
          gameboard.fullBoard[subBoardTmp.row][subBoardTmp.col][move.second.row][move.second.col];
      depth++;
      // move and recur
      gameboard.playMoveOnBoard(move.second, playerSymbol);
      maxVal = max(maxVal, minimax(gameboard, alpha, beta, false));
      // return to previous state
      depth--;
      gameboard.fullBoard[subBoardTmp.row][subBoardTmp.col][move.second.row][move.second.col] =
          fullBoardTmp;
      gameboard.winner = winnerTmp;
      gameboard.winnerBoard[subBoardTmp.row][subBoardTmp.col] = winnerBoardTmp;
      gameboard.subBoard = subBoardTmp;
      alpha = max(alpha, maxVal);
      if ((maxVal == MAX_EVAL) || (beta <= alpha)) {
        // pruneCount++;
        cout << depth << "|" << leafNodeCount << endl;
        // cout << depth << "|" << leafNodeCount << "|" << pruneCount << "|" << alpha << "|" << beta
        //      << "|" << maxVal << endl;
        break;
      }
    }
    return maxVal;
  } else {
    auto minVal = INT_MAX;
    for (auto const &move : playableMoves) {
      const char fullBoardTmp =
          gameboard.fullBoard[subBoardTmp.row][subBoardTmp.col][move.second.row][move.second.col];
      depth++;
      // move and recur
      gameboard.playMoveOnBoard(move.second, playerSymbol);
      minVal = min(minVal, minimax(gameboard, alpha, beta, true));
      // return to previous state
      depth--;
      gameboard.fullBoard[subBoardTmp.row][subBoardTmp.col][move.second.row][move.second.col] =
          fullBoardTmp;
      gameboard.winner = winnerTmp;
      gameboard.winnerBoard[subBoardTmp.row][subBoardTmp.col] = winnerBoardTmp;
      gameboard.subBoard = subBoardTmp;
      beta = min(beta, minVal);
      if ((minVal == MIN_EVAL) || (beta <= alpha)) {
        // pruneCount++;
        cout << depth << "," << leafNodeCount << endl;
        if (depth == 40 && !prune40) {
          cout << RED;
        } else if (depth == 30 && !prune30){
          cout << GREEN;
        }
        break;
      }
    }
    return minVal;
  }
}

int testPlayFirstMove(Node node, bool isMaxPlayer) {
  char playerSymbol = getPlayerSymbol(isMaxPlayer);
  node.depth++;
  if (node.depth >= focusDepthStart && node.depth <= focusDepthEnd) {
    cout << node.depth << endl;
    node.gameState.print();
    node.gameState.printWinnerBoard();
  }
  if (node.gameState.winner != EMPTY_SYMBOL) {
    return 0;
  }
  const auto playableMoves = node.gameState.getLegalMovesOnTheCurrentSubBoard(
      isMaxPlayer, isMaxPlayer ? depthForMaxPlayer : depthForMinPlayer);
  if (node.depth >= focusDepthStart && node.depth <= focusDepthEnd) {
    for (auto &&move : playableMoves) {
      cout << move.first << " " << move.second << "\n";
    }
  }
  testDepth++;
  // cout << "-----endturn-----\n";
  Node childNode(node);
  childNode.gameState.playMoveOnBoard(playableMoves[0].second, playerSymbol);
  return testPlayFirstMove(childNode, !isMaxPlayer);
}

void playGame(Node node) {
  bool isGameEnd = false;
  bool isHumanTurn = true;
  size_t row, col, subrow, subcol;
  while (!isGameEnd) {
    if (isHumanTurn) {
      node.gameState.print(node.lastMove.subBoard, node.lastMove.move);
      node.gameState.printWinnerBoard();
      while (true) {
        // Human turn
        cout << "Enter your move: ";
        if (node.gameState.subBoard == COORDINATE_DEFAULT) {
          cout << "As the first player, you can place it anywhere (e.g. 1 1 1 1)\n: ";
          cin >> row >> col >> subrow >> subcol;
          if (node.gameState.isLegalMove(row, col, subrow, subcol)) {
            node.updateNodeRecord(getPlayerSymbol(false), {subrow, subcol}, {row, col}); //
            node.gameState.subBoard = {row, col};
            node.gameState.playMoveOnBoard({subrow, subcol}, getPlayerSymbol(false));
            break;
          }
        } else {
          cin >> subrow >> subcol;
          if (node.gameState.isLegalMove(node.gameState.subBoard.row, node.gameState.subBoard.col,
                                         subrow, subcol)) {
            node.updateNodeRecord(getPlayerSymbol(false), {subrow, subcol}); //
            node.gameState.playMoveOnBoard({subrow, subcol}, getPlayerSymbol(false));
            break;
          }
        }
        cout << "That's an illegal move...\n";
      }
    } else {
      const auto allMoves = node.gameState.getLegalMovesOnTheCurrentSubBoard(true, 4);
      auto idx = 0;
      for (auto moves : allMoves) {
        if (idx == 3) {
          break;
        }
        cout << moves.first << " " << moves.second << "\n";
        idx++;
      }
      const auto randomMove = allMoves[0].second;
      node.updateNodeRecord(getPlayerSymbol(true), randomMove); //
      node.gameState.playMoveOnBoard(randomMove, getPlayerSymbol(true));
      cout << "\n--------------------------------------\nAI: My turn! I place "
           << getPlayerSymbol(true) << " at {" << randomMove.row << ", " << randomMove.col
           << "}!\n--------------------------------------\n";
    }
    if (node.gameState.winner != EMPTY_SYMBOL) {
      cout << "The winner is " << node.gameState.winner << "!\n";
      node.gameState.print(node.lastMove.subBoard, node.lastMove.move);
      node.gameState.printWinnerBoard();
      isGameEnd = true;
    }
    isHumanTurn = !isHumanTurn;
  }
}

int main() {
  cout << RESET;
  const int boardSize = 3;
  GameBoard initialGameBoard = GameBoard(boardSize);
  initialGameBoard.subBoard = {1, 1};
  initialGameBoard.playMoveOnBoard({1, 0}, getPlayerSymbol(false));
  initialGameBoard.print();
  initialGameBoard.printWinnerBoard();
  // TESTING EVAL ALGO: AUTOMATICALLY PLAY BEST MOVE
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
  // cout << "[CONFIG] max_ply " << depthForMaxPlayer << " | min_ply " << depthForMinPlayer << endl;
  // Node startNode(initialGameBoard, 0);
  // cout << testPlayFirstMove(startNode, true) << endl;
  cout << minimax(initialGameBoard, INT_MIN, INT_MAX, true);
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "\nTime difference = "
            << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << " us"
            << std::endl;
  return 0;
}