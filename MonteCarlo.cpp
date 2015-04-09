#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <limits.h>
#include <string>
#include <string.h>
#include <sstream>
#include <set>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stack>
#include <queue>

using namespace std;
typedef long long ll;

const int BOARD_SIZE  = 3;              // 盤面の大きさ
const int BOARD_MAX   = BOARD_SIZE * BOARD_SIZE; // ボードの大きさ

// ボードの状態(1文字)
const int W = -1; // 壁
const int E = 0; // 空白
const int O = 1; // O(先手)
const int X = 2; // X(後手)

// ボードの状態(フルネーム)
const int WALL  = -1; // 壁
const int EMPTY = 0;  // 空白

// ゲームの状態を表す
const int NONE  = 0;  // 何もなし
const int O_WIN = 1;  // Oの勝利
const int X_WIN = 2;  // Xの勝利
const int DRAW  = 3;  // 引き分け

// 三目並べの盤面
int board[BOARD_MAX] = {
  E, E, E,
  E, E, E,
  E, E, E
};

// 保存用の盤面
int boardCopy[BOARD_MAX];
int boardCopy2[BOARD_MAX];

// プレイアウトの総数
int playoutCount = 0;

// xorshiftによる乱数生成
unsigned long long xor128(){
  static unsigned long long rx=123456789, ry=362436069, rz=521288629, rw=88675123;
  unsigned long long rt = (rx ^ (rx<<11));
  rx=ry; ry=rz; rz=rw;
  return (rw=(rw^(rw>>19))^(rt^(rt>>8)));
}

// 探索済みのボードを調べる用
int checkBoard[BOARD_MAX];

/*
 * 二次元座標を一次元に変換する
 *   y: y座標
 *   x: x座標
 */
int getZ(int y, int x){
  return y * BOARD_SIZE + x;
}

/*
 *      V V V
 *  D1  1 2 3 
 *     +-+-+-+
 *  H1 |x|x|x|
 *     +-+-+-+
 *  H2 |4|5|6|
 *     +-+-+-+
 *  H3 |7|8|9|
 *     +-+-+-+
 *  D2
 */
int getH1(){
  int z1 = getZ(0, 0);
  int z2 = getZ(0, 1);
  int z3 = getZ(0, 2);

  return (board[z1] & board[z2] & board[z3]);
}

/*
 *      V V V
 *  D1  1 2 3 
 *     +-+-+-+
 *  H1 |1|2|3|
 *     +-+-+-+
 *  H2 |x|x|x|
 *     +-+-+-+
 *  H3 |7|8|9|
 *     +-+-+-+
 *  D2
 */
int getH2(){
  int z4 = getZ(1, 0);
  int z5 = getZ(1, 1);
  int z6 = getZ(1, 2);

  return (board[z4] & board[z5] & board[z6]);
}

/*
 *      V V V
 *  D1  1 2 3 
 *     +-+-+-+
 *  H1 |1|2|3|
 *     +-+-+-+
 *  H2 |4|5|6|
 *     +-+-+-+
 *  H3 |x|x|x|
 *     +-+-+-+
 *  D2
 */
int getH3(){
  int z7 = getZ(2, 0);
  int z8 = getZ(2, 1);
  int z9 = getZ(2, 2);

  return (board[z7] & board[z8] & board[z9]);
}

/*
 *      V V V
 *  D1  1 2 3 
 *     +-+-+-+
 *  H1 |x|2|3|
 *     +-+-+-+
 *  H2 |x|5|6|
 *     +-+-+-+
 *  H3 |x|8|9|
 *     +-+-+-+
 *  D2
 */
int getV1(){
  int z1 = getZ(0, 0);
  int z4 = getZ(1, 0);
  int z7 = getZ(2, 0);

  return (board[z1] & board[z4] & board[z7]);
}

/*
 *      V V V
 *  D1  1 2 3 
 *     +-+-+-+
 *  H1 |1|x|3|
 *     +-+-+-+
 *  H2 |4|x|6|
 *     +-+-+-+
 *  H3 |7|x|9|
 *     +-+-+-+
 *  D2
 */
int getV2(){
  int z2 = getZ(0, 1);
  int z5 = getZ(1, 1);
  int z8 = getZ(2, 1);

  return (board[z2] & board[z5] & board[z8]);
}

/*
 *      V V V
 *  D1  1 2 3 
 *     +-+-+-+
 *  H1 |1|2|x|
 *     +-+-+-+
 *  H2 |4|5|x|
 *     +-+-+-+
 *  H3 |7|8|x|
 *     +-+-+-+
 *  D2
 */
int getV3(){
  int z3 = getZ(0, 2);
  int z6 = getZ(1, 2);
  int z9 = getZ(2, 2);

  return (board[z3] & board[z6] & board[z9]);
}

/*
 *      V V V
 *  D1  1 2 3 
 *     +-+-+-+
 *  H1 |x|2|3|
 *     +-+-+-+
 *  H2 |4|x|6|
 *     +-+-+-+
 *  H3 |7|8|x|
 *     +-+-+-+
 *  D2
 */
int getD1(){
  int z1 = getZ(0, 0);
  int z5 = getZ(1, 1);
  int z9 = getZ(2, 2);

  return (board[z1] & board[z5] & board[z9]);
}

/*
 *      V V V
 *  D1  1 2 3 
 *     +-+-+-+
 *  H1 |x|2|3|
 *     +-+-+-+
 *  H2 |4|x|6|
 *     +-+-+-+
 *  H3 |7|8|x|
 *     +-+-+-+
 *  D2
 */
int getD2(){
  int z3 = getZ(0, 2);
  int z5 = getZ(1, 1);
  int z7 = getZ(2, 0);

  return (board[z3] & board[z5] & board[z7]);
}

/*
 * マークを反転させる
 *   mark: マーク
 */
int flipMark(int mark){
  return (3 ^ mark);
}

/*
 * ボードに印を付ける。無効な手にはエラーを返す
 *     y: y座標
 *     x: x座標
 *  mark: 印
 */
int putMark(int z, int mark){
  if(board[z] != EMPTY) return -1;

  board[z] = mark;

  return 0;
}

/*
 * ゲームの状態を調べます
 *   0: まだ決着していない
 *   1: Oの勝ち
 *   2: Xの勝ち
 */
int checkGameState(){
  int h1 = getH1();
  int h2 = getH2();
  int h3 = getH3();

  if(h1 != NONE) return h1;
  if(h2 != NONE) return h2;
  if(h3 != NONE) return h3;

  int v1 = getV1();
  int v2 = getV2();
  int v3 = getV3();

  if(v1 != NONE) return v1;
  if(v2 != NONE) return v2;
  if(v3 != NONE) return v3;

  int d1 = getD1();
  int d2 = getD2();

  if(d1 != NONE) return d1;
  if(d2 != NONE) return d2;

  for(int y = 0; y < BOARD_SIZE; y++){
    for(int x = 0; x < BOARD_SIZE; x++){
      int z = getZ(y,x);
      if(board[z] == EMPTY) return NONE;
    }
  }

  return DRAW;
}

/*
 * プレイアウトを行う
 */
int playout(int mark){
  playoutCount += 1;
  int loopMax = 9;    // 9手で終わる
  int gameState;
  int m = mark;

  for(int i = 0; i < loopMax; i++){
    int putList[BOARD_MAX];
    int listSize = 0;

    // 候補手を全て列挙
    for(int y = 0; y < BOARD_SIZE; y++){
      for(int x = 0; x < BOARD_SIZE; x++){
        int z = getZ(y,x);

        // 既に印がある場合は候補から外す
        if(board[z] != EMPTY) continue;

        putList[listSize] = z;
        listSize += 1;
      }
    }

    int z, r = 0;

    while(true){
      // 置ける場所がない場合
      if(listSize == 0){
        break;
      }else{
        r = xor128() % listSize;
        z = putList[r];
      }

      int err = putMark(z, m);
      if(err == 0) break;

      putList[r] = putList[listSize-1];
      listSize -= 1;
    }

    gameState = checkGameState();

    // 決着がついた場合はプレイアウトを終わる
    if(gameState != NONE) break;

    //mark = flipMark(mark);
    m = flipMark(m);
  }

  int win = (gameState == O_WIN)? 1 : 0;
  if(mark == X) win = -win;

  return win;
}

// 最善手を探索する
int selectBestPut(int mark){
  // playoutの回数
  int tryCount      = 10000;
  int bestZ         = 0;
  double bestValue  = -100.0;

  memcpy(boardCopy, board, sizeof(board));

  for(int y = 0; y < BOARD_SIZE; y++){
    for(int x = 0; x < BOARD_SIZE; x++){
      int z = getZ(y,x);

      if(board[z] != EMPTY) continue;

      int err = putMark(z, mark);
      if(err != 0) continue;

      int winCount = 0;

      for(int i = 0; i < tryCount; i++){
        memcpy(boardCopy2, board, sizeof(board));

        int win = -playout(flipMark(mark));
        winCount += win;

        memcpy(board, boardCopy2, sizeof(boardCopy2));
      }

      double winRate = (double)winCount / tryCount;
      printf("z = %d, WinCount = %d/%d, winRate = %5.3f\n", z, winCount, tryCount, winRate);

      if(winRate > bestValue){
        bestValue = winRate;
        bestZ = z;
      }

      memcpy(board, boardCopy, sizeof(boardCopy));
    }
  }

  return bestZ;
}

void printBoard(){
  cout << "+-+-+-+" << endl;

  for(int y = 0; y < BOARD_SIZE; y++){
    for(int x = 0; x < BOARD_SIZE; x++){
      int z = getZ(y,x);

      if(board[z] == O){
        cout << "|O";
      }else if(board[z] == X){
        cout << "|X";
      }else{
        cout << "| ";
      }
    }
    cout << "|" << endl;
    cout << "+-+-+-+" << endl;
  }
  cout << endl;
}


int main(){
  int mark = O;

  while(true){
    playoutCount = 0;
    int z = selectBestPut(mark);

    int err = putMark(z, mark);
    if(err != 0){
      printf("Err!\n");
      exit(0);
    }

    printBoard();

    int gameState = checkGameState();
    printf("GameState = %d\n", checkGameState());

    if(gameState != NONE) break;
    mark = flipMark(mark);
  }

  return 0;

  return 0;
}
