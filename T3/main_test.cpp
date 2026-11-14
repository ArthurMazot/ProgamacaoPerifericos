// main_test.cpp - Test copy of main.cpp
// Both "// Get input" blocks (shipPlace and attack) call the same getInput(),
// which reads {i, j, rotate} from one input vector. attack() ignores rotate.
// Game logic is kept identical to main.cpp; lines marked [TEST] were added.
//
// Build: g++ -std=c++11 -o main_test main_test.cpp
#include <iostream>
#include <vector>    // [TEST]
#include <cstdlib>   // [TEST]
#include <cstring>   // [TEST]

using namespace std;

//////////////////////////////////////////////////
//              Global Variables                //
//////////////////////////////////////////////////

char board1[8][8] = {};
char board2[8][8] = {};
char enemyboard1[8][8] = {};
char enemyboard2[8][8] = {};
char shipCount1;
char shipCount2;

int leds[8][32] = {};

//////////////////////////////////////////////////
//         [TEST] Input vector + getInput       //
//////////////////////////////////////////////////

struct Input { int i, j, rotate; };   // rotate=0 -> vertical (i++) | 1 -> horizontal (j++)

vector<Input> input;      // consumed in order: first the 10 ship placements, then the shots
size_t inputIdx = 0;
bool inputExhausted = false;

// Same function for both places. attack() passes rotate = NULL (ignored).
// Returns false when the vector is empty.
bool getInput(int *i, int *j, char *rotate){
    if(inputIdx >= input.size()){ inputExhausted = true; return false; }
    *i = input[inputIdx].i;
    *j = input[inputIdx].j;
    if(rotate) *rotate = input[inputIdx].rotate;
    inputIdx++;
    return true;
}

// Placement part of the vector (same for every test).
// Order in init(): board1 -> 4,3,3,2,2 ; board2 -> 4,3,3,2,2
const vector<Input> placements = {
    // ---- board1 ----   {i, j, rotate}
    {6, 0, 0},  // ship 4: INVALID (rows 6..9, out of the board)
    {0, 0, 1},  // ship 4: row 0, cols 0-3
    {0, 2, 0},  // ship 3: INVALID (overlaps ship 4 at [0][2])
    {2, 0, 0},  // ship 3: rows 2-4, col 0
    {7, 5, 1},  // ship 3: row 7, cols 5-7
    {3, 7, 1},  // ship 2: INVALID (cols 7..8, out of the board)
    {4, 7, 0},  // ship 2: rows 4-5, col 7
    {5, 3, 1},  // ship 2: row 5, cols 3-4
    // ---- board2 ----
    {1, 1, 0},  // ship 4: rows 1-4, col 1
    {0, 4, 1},  // ship 3: row 0, cols 4-6
    {6, 2, 1},  // ship 3: row 6, cols 2-4
    {3, 6, 0},  // ship 2: rows 3-4, col 6
    {7, 0, 1},  // ship 2: row 7, cols 0-1
};

int invalidCount = 0;
vector<int> logTarget;    // 1 -> board1/enemyboard1, 2 -> board2/enemyboard2
vector<int> logResult;    // value returned by attack(): 0 miss, 1 hit, 2 invalid
int attacksAfterWin = 0;  // attack() called when the target fleet was already sunk

int failures = 0;
void check(bool ok, const char *msg){
    printf("  [%s] %s\n", ok ? "PASS" : "FAIL", msg);
    if(!ok) failures++;
}

//////////////////////////////////////////////////
//                 Functions                    //
//////////////////////////////////////////////////

void printBoard(char (*board)[8]){
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++)
            printf("%d", board[i][j]);
        puts("");}

    puts("========");
}

//=======================================//

void shipPlace(char (*board)[8], int ship){
    char flag = 0;
    char rotate = 0; //Rotate = 0 -> draw vertically | rotate = 1 -> draw horizontaly
    int i;
    int j;

    do {
        // Get input  [TEST]
        flag = 0;
        if(!getInput(&i, &j, &rotate)){
            puts("[TEST] input exhausted during ship placement - aborting");
            exit(1);
        }
        // === //

        int x = i;
        int y = j;
        int *p = (rotate ? &y : &x);

        for(int k = 0; k < ship; k++, (*p)++)
            if(*p > 7 || board[x][y] != 0){
                flag = 1;
                puts("Posição inválida");
                invalidCount++;  // [TEST]
                break;
            }
    } while(flag);

    for(int k = 0; k < ship; k++){
        board[i][j] = ship;
        if(rotate) j++;
        else i++;
    }
}

//=======================================//

char attack(char (*board)[8], char (*enemyboard)[8], char *shipCount){
    // Get input  [TEST]
    int i, j;
    if(*shipCount == 0) attacksAfterWin++;
    if(!getInput(&i, &j, NULL)) return 0;   // [TEST] stop the game when out of input
    // === //

    char r;  // [TEST] used only to log the result
    if(enemyboard[i][j] == 1 || enemyboard[i][j] == 3){ r = 2; goto done; } // Invalid shot
    {
    char hit = (enemyboard[i][j] == 2);

    if(hit != 0){ // Hit
        enemyboard[i][j] = 1;
        board[7-i][7-j] = 1;
        (*shipCount)--;
    }
    else{ // Miss
        enemyboard[i][j] = 3;
        board[7-i][7-j] = 5;
    }
    r = hit;
    }
done:
    logTarget.push_back(board == board1 ? 1 : 2);  // [TEST]
    logResult.push_back(r);                         // [TEST]
    return r;
}

//=======================================//
void init(){
    shipPlace(board1, 4);
    shipPlace(board1, 3);
    shipPlace(board1, 3);
    shipPlace(board1, 2);
    shipPlace(board1, 2);
    shipCount1 = 4 + 3 + 3 + 2 + 2;

    shipPlace(board2, 4);
    shipPlace(board2, 3);
    shipPlace(board2, 3);
    shipPlace(board2, 2);
    shipPlace(board2, 2);
    shipCount2 = 4 + 3 + 3 + 2 + 2;

    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            if(board1[i][j] != 0) enemyboard1[7-i][7-j] = 2;
            else enemyboard1[7-i][7-j] = 0;

            if(board2[i][j] != 0) enemyboard2[7-i][7-j] = 2;
            else enemyboard2[7-i][7-j] = 0;
        }
    }

    printBoard(board1);
    printBoard(enemyboard2);
    printBoard(enemyboard1);
    printBoard(board2);
}

// Same loop as main() in main.cpp; returns the winner it prints.
// [TEST] the break on inputExhausted avoids an infinite loop.
int playGame(){
    char player = 0;
    do{
        if(player == 1) while(attack(board1, enemyboard1, &shipCount1) && shipCount1 != 0);
        else while(attack(board2, enemyboard2, &shipCount2) && shipCount2 != 0);
        player = !player;
        if(inputExhausted && shipCount1 && shipCount2){ puts("[TEST] input exhausted - game stopped"); return -1; }
    }while(shipCount1 && shipCount2);

    if(shipCount2 == 0) { puts("Player 0 venceu"); return 0; }
    else                { puts("Player 1 venceu"); return 1; }
}

//////////////////////////////////////////////////
//                 [TEST] Tests                 //
//////////////////////////////////////////////////

// Reset everything and load: placements + shots (rotate ignored for shots -> 0)
void setup(const vector<Input> &shots){
    memset(board1, 0, sizeof board1);  memset(board2, 0, sizeof board2);
    memset(enemyboard1, 0, sizeof enemyboard1); memset(enemyboard2, 0, sizeof enemyboard2);
    input = placements;
    input.insert(input.end(), shots.begin(), shots.end());
    inputIdx = 0; inputExhausted = false; invalidCount = 0;
    logTarget.clear(); logResult.clear(); attacksAfterWin = 0;
    init();
}

void printLog(){
    printf("  shots (board:result, H=hit m=miss I=invalid): ");
    for(size_t k = 0; k < logTarget.size(); k++)
        printf("%d%c ", logTarget[k], logResult[k] == 1 ? 'H' : logResult[k] == 2 ? 'I' : 'm');
    puts("");
}

bool sameLog(const vector<int> &t, const vector<int> &r){
    return t == logTarget && r == logResult;
}

const char expBoard1[8][8] = {
    {4,4,4,4,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {3,0,0,0,0,0,0,0},
    {3,0,0,0,0,0,0,0},
    {3,0,0,0,0,0,0,2},
    {0,0,0,2,2,0,0,2},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,3,3,3},
};
const char expBoard2[8][8] = {
    {0,0,0,0,3,3,3,0},
    {0,4,0,0,0,0,0,0},
    {0,4,0,0,0,0,0,0},
    {0,4,0,0,0,0,2,0},
    {0,4,0,0,0,0,2,0},
    {0,0,0,0,0,0,0,0},
    {0,0,3,3,3,0,0,0},
    {2,2,0,0,0,0,0,0},
};

// Ship cells in enemy-board coordinates {i, j, rotate(ignored)}
//   board2 ships seen on enemyboard2 (attacked by Player 0)
//   board1 ships seen on enemyboard1 (attacked by Player 1)
const vector<Input> board1Ships = {
    {7,7,0},{7,6,0},{7,5,0},{7,4,0},{5,7,0},{4,7,0},{3,7,0},
    {0,2,0},{0,1,0},{0,0,0},{3,0,0},{2,0,0},{2,4,0},{2,3,0},
};

void testPlacement(){
    puts("\n===== TEST 1: ship placement =====");
    setup({});

    check(invalidCount == 3, "3 invalid positions rejected (out of board x2, overlap x1)");
    check(inputIdx == placements.size(), "all placement inputs consumed");
    check(memcmp(board1, expBoard1, 64) == 0, "board1 matches expected layout");
    check(memcmp(board2, expBoard2, 64) == 0, "board2 matches expected layout");

    bool mirror = true;
    for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++){
        if(enemyboard1[7-i][7-j] != (board1[i][j] ? 2 : 0)) mirror = false;
        if(enemyboard2[7-i][7-j] != (board2[i][j] ? 2 : 0)) mirror = false;
    }
    check(mirror, "enemy boards are the 180-degree mirror of the boards");
    check(shipCount1 == 14 && shipCount2 == 14, "shipCount1 == shipCount2 == 14");
}

void testAttack(){
    puts("\n===== TEST 2: attack() hit / miss / repeated shot =====");
    // enemyboard2[6][6] <-> board2[1][1] (ship 4) ; enemyboard2[0][0] <-> board2[7][7] (water)
    // The rotate value (9) must be ignored by attack()
    setup({ {6,6,9}, {0,0,9}, {6,6,9}, {0,0,9} });

    char r = attack(board2, enemyboard2, &shipCount2);
    check(r == 1, "shot [6][6] is a hit (returns 1)");
    check(enemyboard2[6][6] == 1 && board2[1][1] == 1, "hit marked: enemyboard2[6][6]=1, board2[1][1]=1");
    check(shipCount2 == 13, "shipCount2 decremented to 13");

    r = attack(board2, enemyboard2, &shipCount2);
    check(r == 0, "shot [0][0] is a miss (returns 0)");
    check(enemyboard2[0][0] == 3 && board2[7][7] == 5, "miss marked: enemyboard2[0][0]=3, board2[7][7]=5");

    r = attack(board2, enemyboard2, &shipCount2);
    check(r == 2, "repeated shot on a HIT cell is invalid (returns 2)");
    check(enemyboard2[6][6] == 1 && board2[1][1] == 1 && shipCount2 == 13, "  ...and changes nothing");

    r = attack(board2, enemyboard2, &shipCount2);
    check(r == 2, "repeated shot on a MISS cell is invalid (returns 2)");
    check(enemyboard2[0][0] == 3 && board2[7][7] == 5, "  ...and changes nothing");

    check(memcmp(board1, expBoard1, 64) == 0, "attacking board2 does not modify board1");
}

void testPlayer0Wins(){
    puts("\n===== TEST 3: full game, Player 0 wins =====");
    vector<Input> shots = {
        // Player 0 -> enemyboard2
        {6,6,0}, {6,6,0}, {5,6,0}, {0,0,0},         // hit, invalid (shoot again), hit, miss -> turn passes
        // Player 1 -> enemyboard1
        {7,7,0}, {2,2,0},                           // hit, miss -> turn passes
        // Player 0 -> enemyboard2 : the 12 remaining ship cells
        {4,6,0}, {3,6,0}, {7,3,0}, {7,2,0}, {7,1,0}, {1,5,0},
        {1,4,0}, {1,3,0}, {4,1,0}, {3,1,0}, {0,7,0}, {0,6,0},
    };
    setup(shots);
    int winner = playGame();
    printLog();

    vector<int> t = {2,2,2,2, 1,1, 2,2,2,2,2,2,2,2,2,2,2,2};
    vector<int> r = {1,2,1,0, 1,0, 1,1,1,1,1,1,1,1,1,1,1,1};
    check(sameLog(t, r), "turn order and results as expected (invalid shot repeats the turn)");
    check(shipCount2 == 0 && shipCount1 == 13, "final shipCount2 == 0, shipCount1 == 13");
    check(attacksAfterWin == 0, "no input requested after the last ship is sunk");
    printf("        (attack() called %d time(s) after the fleet was sunk)\n", attacksAfterWin);
    check(winner == 0, "prints 'Player 0 venceu'");
}

void testPlayer1Wins(){
    puts("\n===== TEST 4: full game, Player 1 wins =====");
    vector<Input> shots = { {0,0,0} };                       // Player 0 misses
    shots.insert(shots.end(), board1Ships.begin(), board1Ships.end()); // Player 1 sinks board1
    setup(shots);
    int winner = playGame();
    printLog();

    check(shipCount1 == 0 && shipCount2 == 14, "final shipCount1 == 0, shipCount2 == 14");
    check(attacksAfterWin == 0, "no input requested after the last ship is sunk");
    printf("        (attack() called %d time(s) after the fleet was sunk)\n", attacksAfterWin);
    check(winner == 1, "prints 'Player 1 venceu'");
}

void testPlayer1InvalidShot(){
    puts("\n===== TEST 5: Player 1 repeats a shot (invalid) =====");
    vector<Input> shots = {
        {0,0,0},            // P0: miss               -> turn passes
        {7,7,0}, {1,1,0},   // P1: hit, miss          -> turn passes
        {0,1,0},            // P0: miss               -> turn passes
        {1,1,0},            // P1: INVALID (already shot) -> should shoot again
        {7,6,0},            // P1: hit (if still P1's turn)
    };
    setup(shots);
    playGame();   // stops when input runs out
    printLog();

    // Expected: P1 keeps the turn after the invalid shot, so [7][6] hits enemyboard1
    vector<int> t = {2, 1,1, 2, 1,1};
    vector<int> r = {0, 1,0, 0, 2,1};
    check(sameLog(t, r), "invalid shot by Player 1 repeats Player 1's turn (same as Player 0)");
}

int main(){
    testPlacement();
    testAttack();
    testPlayer0Wins();
    testPlayer1Wins();
    testPlayer1InvalidShot();

    printf("\n===== %s: %d failure(s) =====\n", failures ? "FAILED" : "ALL PASSED", failures);
    return failures ? 1 : 0;
}
