#include <iostream>

using namespace std;

//////////////////////////////////////////////////
//              Global Variables                //
//////////////////////////////////////////////////

// Boards
// Water        -> 0
// Hit          -> 1
// Small Ship   -> 2
// Medium Ship  -> 3
// Large Ship   -> 4
// Miss         -> 5

// Enemy Boards
// Water -> 0
// Hit   -> 1
// Ship  -> 2
// Miss  -> 3

//=============//
//    board1   //---\ 
//=============//   |
// enemyboard2 //-\ |
//=============// | |
// enemyboard1 //-|-/
//=============// |
//    board2   //-/
//=============//

// leds: Combine the 4 boards in one vector that containing colors
char board1[8][8] = {};
char board2[8][8] = {};
char enemyboard1[8][8] = {};
char enemyboard2[8][8] = {};
char shipCount1;
char shipCount2;


int leds[8][32] = {};

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
        // Do a function that returns the coordinates [i, j] and the rotate of the ship
        // Get input
        flag = 0;
        i = 0;
        j = 0;
        rotate = 0;
        // === //

        int x = i;
        int y = j;
        int *p = (rotate ? &y : &x);
        
        // Check if the ship will go out the board and if it will pass trought outer ship
        for(int k = 0; k < ship; k++, (*p)++)
            if(*p > 7 || board[x][y] != 0){
                flag = 1;
                puts("Posição inválida");
                break;
            }
        // === //
    } while(flag);

    // Place the ship on the board
    for(int k = 0; k < ship; k++){
        board[i][j] = ship;
        if(rotate) j++;
        else i++;
    }
    // === //
}

//=======================================//

char attack(char (*board)[8], char (*enemyboard)[8], char *shipCount){
    // Get input
    int i = 0;
    int j = 0;
    // === //

    if(enemyboard[i][j] == 1 || enemyboard[i][j] == 3) return 2; // Invalid shot
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

    return hit;
}

//=======================================//
void init(){
    shipPlace(board1, 4);
    shipPlace(board1, 3);
    shipPlace(board1, 3);
    shipPlace(board1, 2);
    shipPlace(board1, 2);
    shipCount1 = 4 + 3 + 3 + 2 + 2; // Sum of all ships

    shipPlace(board2, 4);
    shipPlace(board2, 3);
    shipPlace(board2, 3);
    shipPlace(board2, 2);
    shipPlace(board2, 2);
    shipCount2 = 4 + 3 + 3 + 2 + 2;

    // Inicialize enemy boards
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++){
            if(board1[i][j] != 0) enemyboard1[7-i][7-j] = 2;
            else enemyboard1[7-i][7-j] = 0;

            if(board2[i][j] != 0) enemyboard2[7-i][7-j] = 2;
            else enemyboard2[7-i][7-j] = 0;
        }
    }
    // === //

    printBoard(board1);
    printBoard(enemyboard2);
    printBoard(enemyboard1);
    printBoard(board2);

}

//////////////////////////////////////////////////
//                    Main                      //
//////////////////////////////////////////////////

int main(){
    init();

    // Player = 0 -> attack board2 and enemyboard2, if hits decrement shipCount2
    // Player = 1 -> attack board1 and enemyboard1, if hits decrement shipCount1
    // The game ends when one player hit all the ships oh your enemy (shipCount == 0)
    char player = 0;
    do{ // Game is in this loop
        if(player == 1) while(attack(board1, enemyboard1, &shipCount1) && shipCount1 != 0);
        else while(attack(board2, enemyboard2, &shipCount2) && shipCount2 != 0);
        player = !player;
    }while(shipCount1 && shipCount2);

    if(shipCount2 == 0) puts("Player 0 venceu");
    else puts("Player 1 venceu");
}