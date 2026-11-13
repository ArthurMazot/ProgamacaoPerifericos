#include <iostream>
#include <time.h>

using namespace std;

//////////////////////////////////////////////////
//                  Commits                     //
//////////////////////////////////////////////////

// Arthur, 13/11/2026, (First commit)
// Created a function to print the board
// Created a function to check if the position is valid and place the ship on the board

//////////////////////////////////////////////////
//              Global Variables                //
//////////////////////////////////////////////////

// Water        -> 0
// Hit          -> 1
// Small Ship   -> 2
// Medium Ship  -> 3
// Large Ship   -> 4

char board1[8][8] = {};
char board2[8][8] = {};

//////////////////////////////////////////////////
//                 Functions                    //
//////////////////////////////////////////////////


void printBoard(char (*board)[8]){
    for(int i = 0; i < 8; i++){
        for(int j = 0; j < 8; j++)
            printf("%c", board[i][j] + '0');
        puts("");}

    puts("========");
}

//=======================================//

void shipPos(char (*board)[8], int ship){
    char flag = 0;
    char rotate = 0; //Rotate = 0 -> draw horizontally | rotate = 1 -> draw vertically
    int i;
    int j;

    do {
        // Do a function that returns the coordinates [i, j] and the rotate of the ship
        // Get input
        flag = 0;
        rotate = rand()%2;
        i = rand()%8;
        j = rand()%8;
        // === //

        if(i < 0 || i > 7 || j < 0 || j > 7){
            puts("Posição fora da grade");
            continue;
        }

        int x = i;
        int y = j;
        int *p = (rotate ? &y : &x);
        
        // Check if the ship will go out the board and if it will pass trought outer ship
        for(int k = 0; k < ship; k++){
            if(*p > 7 || board[x][y] != 0){
                flag = 1;
                puts("Posição inválida");
                break;
            }

            if(rotate) y++;
            else x++;
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

//////////////////////////////////////////////////
//                    Main                      //
//////////////////////////////////////////////////

int main(){
    srand(time(NULL));

    shipPos(board1, 4);
    shipPos(board1, 3);
    shipPos(board1, 3);
    shipPos(board1, 2);
    shipPos(board1, 2);

    shipPos(board2, 4);
    shipPos(board2, 3);
    shipPos(board2, 3);
    shipPos(board2, 2);
    shipPos(board2, 2);
    printBoard(board1);
    printBoard(board2);
}
