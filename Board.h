
#ifndef  _BOARD
#define  _BOARD
typedef unsigned long long U64;
const int win = 7;              // 连五
const int flex4 = 6;            // 活四
const int block4 = 5;           // 冲四
const int flex3 = 4;            // 活三
const int block3 = 3;           // 眠三
const int flex2 = 2;            // 活二
const int block2 = 1;           // 眠二
const int Ntype = 8;            // 棋型个数
const int MaxSize = 20;         // 棋盘最大尺寸
const int hashSize = 1 << 20;   // 哈希表尺寸

// hash表相关
const int hash_exact = 0;
const int hash_alpha = 1;
const int hash_beta = 2;
const int unknown = -20000;

// 点的状态
const int Outside = 3;
const int Empty = 2;
const int Black = 1;
const int White = 0;
// 方向向量
const int dx[4] = { 1, 0, 1, 1 };
const int dy[4] = { 0, 1, 1, -1 };

// 坐标
struct Pos {
  int x;
  int y;
};

// 带评价的点
struct Point {
  Pos p;
  int val;
};

// 棋盘单点结构
struct Cell {
  int piece;
  int IsCand;
  int pattern[2][4];
};

// 哈希表结构
struct Hashe {
  U64 key;
  int depth;
  int hashf;
  int val;
};

class Board {
public:
  int step = 0;                                 // 棋盘落子数
  int size = 15;                                // 棋盘当前尺寸
  int b_start, b_end;                           // 棋盘坐标索引
  U64 zobristKey = 0;                           // 表示当前局面的zobristKey
  U64 zobrist[2][MaxSize + 4][MaxSize + 4];     // zobrist键值表
  Hashe hashTable[hashSize];                    // 哈希表
  int typeTable[10][6][6][3];                   // 初级棋型表
  int patternTable[65536][2];                   // 完整棋型表
  int pval[8][8][8][8];                         // 棋形分值表
  Cell cell[MaxSize + 8][MaxSize + 8];          // 棋盘
  Pos remMove[MaxSize * MaxSize];               // 记录落子
  Point cand[256];                              // 临时存储合理着法(两格内有子)
  bool IsLose[MaxSize + 4][MaxSize + 4];        // 记录根节点的必败点

    Board();
   ~Board();
  void InitType();
  void InitPval();
  void InitPattern();
  void InitZobrist();
  void SetSize(int _size);
  void MakeMove(Pos next);
  void DelMove();
  void Undo();
  void ReStart();
  void UpdateType(int x, int y);
  U64 Rand64();
  int GetKey(int x, int y, int i);
  int GetPval(int a, int b, int c, int d);
  int LineType(int role, int key);
  int ShortLine(int *line);
  int CheckFlex3(int *line);
  int CheckFlex4(int *line);
  int GetType(int len, int len2, int count, int block);

  /* 可内联成员函数 */
  int color(int step) {
    return step & 1;
  }

  bool CheckXy(int x, int y) {
    return cell[x][y].piece != Outside;
  }

  Cell * LastMove(){
    return &cell[remMove[step].x][remMove[step].y];
  }

  void TypeCount(Cell *c, int role, int *type) {
    ++type[c->pattern[role][0]];
    ++type[c->pattern[role][1]];
    ++type[c->pattern[role][2]];
    ++type[c->pattern[role][3]];
  }

  bool IsType(Cell *c, int role, int type) {
    return c->pattern[role][0] == type
      || c->pattern[role][1] == type
      || c->pattern[role][2] == type
      || c->pattern[role][3] == type;
  }

  bool CheckWin() {
    int role = color(step);
    Cell *c = LastMove();

    return c->pattern[role][0] == win
      || c->pattern[role][1] == win
      || c->pattern[role][2] == win
      || c->pattern[role][3] == win;
  }

};
#endif
