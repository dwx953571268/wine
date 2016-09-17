
#include "AI.h"
#include<ctime>
#include<cmath>
#include<cstring>
#include<cstdlib>
#include<iostream>
  using namespace std;

int AI::GetTime() {
  return (double)(clock() - start) / CLOCKS_PER_SEC * 1000;
}

int AI::StopTime() {
  return (timeout_turn < time_left / 7) ? timeout_turn : time_left / 7;
}

// 界面下子
void AI::TurnMove(Pos next) {
  next.x += 4, next.y += 4;
  MakeMove(next);
}

// 返回最佳点
Pos AI::TurnBest() {
  Pos best = gobang();
  best.x -= 4, best.y -= 4;
  return best;
}

// 搜索最佳点
Pos AI::gobang() {
  start = clock();
  total = 0;
  stopThink = false;

  // 第一步下中心点
  if (step == 0) {
    BestMove.x = size / 2 + 4;
    BestMove.y = size / 2 + 4;
    return BestMove;
  }
  // 第二，三步随机
  if (step == 1 || step == 2) {
    int rx, ry;
    int d = step * 2 + 1;
    srand(time(NULL));
    do {
      rx = rand() % d + remMove[1].x - step;
      ry = rand() % d + remMove[1].y - step;
    } while (!CheckXy(rx, ry) || cell[rx][ry].piece != Empty);
    BestMove.x = rx;
    BestMove.y = ry;
    return BestMove;
  }
  // 迭代加深搜索
  memset(IsLose, false, sizeof(IsLose));
  for (int i = 2; i <= SearchDepth; i += 2) {
    if (i > 4 && GetTime() * 12 >= StopTime())
      break;
    MaxDepth = i;
    BestVal = minimax(i, -10001, 10000);
    if (BestVal == 10000)
      break;
  }

  ThinkTime = GetTime();

  return BestMove;
}

inline bool AI::Same(Pos a, Pos b) {
  return a.x == b.x && a.y == b.y;
}

  // 根节点搜索
int AI::minimax(int depth, int alpha, int beta) {
  UpdateRound(2);
  Pos move[28];
  int val;
  int count = GetMove(move, 27);

  if (count == 1) {
    BestMove = move[1];
    return 0;
  }

  move[0] = (depth > 2) ? BestMove : move[1];

  // 遍历所有走法
  for (int i = 0; i <= count; i++) {
    if (i > 0 && Same(move[0], move[i]))
      continue;

    if (IsLose[i])
      continue;

    MakeMove(move[i]);
    do {
      if (i > 0 && alpha + 1 < beta) {
        val = -AlphaBeta(depth - 1, -alpha - 1, -alpha);
        if (val <= alpha || val >= beta)
          break;
      }
      val = -AlphaBeta(depth - 1, -beta, -alpha);
    } while (0);
    DelMove();

    if (stopThink)
      break;

    if (val == -10000)
      IsLose[i] = true;

    if (val >= beta) {
      BestMove = move[i];
      return val;
    }
    if (val > alpha) {
      alpha = val;
      BestMove = move[i];
    }
  }
  return alpha == -10001 ? BestVal : alpha;
}

// 带pvs的搜索
int AI::AlphaBeta(int depth, int alpha, int beta) {
  total++;

  static int cnt = 1000;
  if (--cnt <= 0) {
    cnt = 1000;
    if (MaxDepth > 4 && GetTime() + 200 >= StopTime())
      stopThink = true;
  }
  // 对方最后一子连五
  if (CheckWin())
    return -10000;

  // 叶节点
  if (depth == 0)
    return evaluate();

  Pos move[28];
  int count = GetMove(move, 27);


  // 遍历所有move
  int val;
  for (int i = 1; i <= count; i++) {

    MakeMove(move[i]);
    do {
      if (i > 1 && alpha + 1 < beta) {
        val = -AlphaBeta(depth - 1, -alpha - 1, -alpha);
        if (val <= alpha || val >= beta)
          break;
      }
      val = -AlphaBeta(depth - 1, -beta, -alpha);
    } while (0);
    DelMove();

    if (stopThink)
      break;

    if (val >= beta) {
      return val;
    }
    if (val > alpha) {
      alpha = val;
    }
  }
  return alpha;
}

// 剪枝
// 下子方有成五点或活四点
// 对方有成五点(冲四或活四),此时只有一个着法
// 对方有活四点(活三),则有多个防点
int AI::CutCand(Pos * move, Point * cand, int Csize) {
  int me = color(step + 1);
  int you = color(step);
  int moveLen = 0, candLen = 0;
  
  if (cand[1].val >= 2400) {
    move[++moveLen] = cand[++candLen].p;
  } 
  else if (cand[1].val == 1200) {
    move[++moveLen] = cand[++candLen].p;
    if (cand[2].val == 1200) {
      move[++moveLen] = cand[++candLen].p;
    }

    int i, j, k;
    for (k = step; k > 0; k -= 2) {
      if (IsType(remMove[k], you, flex3))
        break;
    }
    for (i = 0; i < 4; ++i) {
      Pos m = remMove[k];
      if (cell[m.x][m.y].pattern[you][i] == flex3) {
        m.x -= (dx[i] * 4), m.y -= (dy[i] * 4);
        for (j = 0; j < 9; ++j) {
          if (cell[m.x][m.y].piece == Empty && IsType(m, you, block4)) {
            move[++moveLen] = m;
          }
          m.x += dx[i], m.y += dy[i];
        }
      }
    }
    for (i = candLen + 1; i <= Csize; ++i) {
      if (IsType(cand[i].p, me, block4))
        move[++moveLen] = cand[i].p;
    }
  }
  return moveLen;
}

// 生成所有着法，并返回个数
int AI::GetMove(Pos * move, int branch) {
  Point cand[200];
  int Csize = 0, Msize = 0;
  int val;
  for (int i = b_start; i < b_end; i++) {
    for (int j = b_start; j < b_end; j++) {
      if (IsCand[i][j] && cell[i][j].piece == Empty) {
        val = ScoreMove(i, j);
        if (val > 0) {
          ++Csize;
          cand[Csize].p.x = i;
          cand[Csize].p.y = j;
          cand[Csize].val = val;
        }
      }
    }
  }
  // 着法排序
  sort(cand, Csize);
  Csize = (Csize < branch) ? Csize : branch;
  // 棋型剪枝
  Msize = CutCand(move, cand, Csize);

  // 如果没有剪枝
  if (Msize == 0) {
    Msize = Csize;
    for (int k = 1; k <= Msize; ++k) {
      move[k] = cand[k].p;
    }
  }

  return Msize;
}

// 排序
void AI::sort(Point * a, int n) {
  int i, j;
  Point key;
  for (i = 2; i <= n; i++) {
    key = a[i];
    for (j = i; j > 1 && a[j - 1].val < key.val; j--) {
      a[j] = a[j - 1];
    }
    a[j] = key;
  }
}


// 局势评价函数
int AI::evaluate() {
  int Ctype[Ntype] = { 0 };     // 先手方棋型个数
  int Htype[Ntype] = { 0 };     // 后手方棋型个数
  int Cscore = 0, Hscore = 0;   // 双方分值
  int me = color(step + 1);     // 先手方
  int you = color(step);        // 后手方
  Cell *c;

  // 统计棋型
  for (int i = b_start; i < b_end; ++i) {
    for (int j = b_start; j < b_end; ++j) {
      if (IsCand[i][j] && cell[i][j].piece == Empty) {
        // 加上该点棋型
        c = &cell[i][j];
        TypeCount(c, me, Ctype);
        TypeCount(c, you, Htype);
      }
    }
  }

  if (Ctype[win] > 0)
    return 10000;
  if (Htype[win] > 1)
    return -10000;
  if (Ctype[flex4] > 0 && Htype[win] == 0)
    return 10000;

  // 计算分值
  for (int i = 1; i < Ntype; ++i) {
    Cscore += Ctype[i] * Tval[i];
    Hscore += Htype[i] * Tval[i];
  }

  return Cscore - Hscore + Cscore >> 2;
}

// 着法打分
int AI::ScoreMove(int x, int y) {
  int score = 0;
  int MeType[Ntype] = { 0 };
  int YouType[Ntype] = { 0 };
  int me = color(step + 1);
  int you = color(step);
  Cell *c = &cell[x][y];

  TypeCount(c, me, MeType);
  TypeCount(c, you, YouType);

  if (MeType[win] > 0)
    return 10000;
  if (YouType[win] > 0)
    return 5000;
  if (MeType[flex4] > 0 || MeType[block4] > 1)
    return 2400;
  if (MeType[block4] > 0 && MeType[flex3] > 0)
    return 2000;
  if (YouType[flex4] > 0 || YouType[block4] > 1)
    return 1200;
  if (YouType[block4] > 0 && YouType[flex3] > 0)
    return 1000;
  if (MeType[flex3] > 1)
    return 400;
  if (YouType[flex3] > 1)
    return 200;

  for (int i = 1; i <= block4; i++) {
    score += MeVal[i] * MeType[i];
    score += YouVal[i] * YouType[i];
  }

  return score;
}
