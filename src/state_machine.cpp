#include "state_machine.hpp"
#include "chess.hpp"
#include <stdio.h>
#include <string.h>

/*********************************************************************************\
;---------------------------------------------------------------------------------;
;                                Helper Functions                                 ;
;---------------------------------------------------------------------------------;
\*********************************************************************************/

void print_bitboard(unsigned long long bitboard){
    for(int i = 0 ; i < 64 ; i++){
        if(i%8 == 0) {printf("\n");}
        if(bitboard & 1ull<<(63-i)){
            printf("X ");
        }
        else{
            printf(". ");
        }
    }
}

unsigned long long get_bitboard_from_square(int square){
    int mod = square%8;
    int div = square/8;
    return 1ull<<(63 - ((div/2*8) + mod));
}

void get_moves_from_square(Move_List *move_list, Move_List *current_square_moves, int square){
    current_square_moves->length = 0;
    for(int i = 0; i < move_list->length; i++){
        printf("SEARCHING %d\n", i);
        if(move_list->moves[i].source_square == square){
            printf("FOUND\n");
            printf("Target square: %d\n", move_list->moves[i].target_square);
            printf("%d\n", current_square_moves->length);
            current_square_moves->moves[current_square_moves->length] = move_list->moves[i];
            current_square_moves->length ++;
        }
    }
    printf("Searching complete, found %d moves\n", current_square_moves->length);
}

void get_attacks_on_square(Move_List *move_list, Move_List *current_square_moves, int square){
    current_square_moves->length = 0;
    for(int i = 0; i < move_list->length; i++){
        printf("SEARCHING %d\n", i);
        if(move_list->moves[i].target_square == square){
            printf("FOUND\n");
            printf("Source square: %d\n", move_list->moves[i].source_square);
            current_square_moves->moves[current_square_moves->length] = move_list->moves[i];
            current_square_moves->length ++;
        }
    }
    printf("Searching complete, found %d moves\n", current_square_moves->length);
}

bool is_friendly(Chess *chess, int side, int square){
    int piece = chess->get_piece_on_square(square);
    if(side == 8){
        if(piece > 0 && piece < 18 || piece == 43 || piece == 46) {
            return true;
        }
        else return false;
    }
    else{
        if(piece >= 18 && piece < 24 || piece == 51 || piece == 54) {
            return true;
        }
        else return false;
    }
    

}

/*********************************************************************************\
;---------------------------------------------------------------------------------;
;                                Actual Functions                                 ;
;---------------------------------------------------------------------------------;
\*********************************************************************************/

State update_state(Chess *chess, int instruction, int side, State state, int state_memory[], 
    Move_List *move_list, Move_List *current_square_moves){
    //chess.print_board();
    unsigned long long square = get_bitboard_from_square(instruction);
    unsigned long long occupied = chess->get_occupied();
    unsigned long long friendlies;
    unsigned long long enemies;

    if(side == 16){  //black turn
        friendlies = chess->get_black();
        enemies = chess->get_white();
    }
    else{ //white turn
        friendlies = chess->get_white();
        enemies = chess->get_black();
    }

    switch(state){
        case Idle:
            if((friendlies & square) != 0ULL){
                get_moves_from_square(move_list, current_square_moves, instruction);
                state_memory[0] = instruction;
                return State::FriendlyPU;
            }
            else if((enemies & square) != 0ULL){
                get_attacks_on_square(move_list, current_square_moves, instruction);
                state_memory[0] = instruction;
                if(current_square_moves->length > 0){
                    return State::EnemyPU;
                }
                else{
                    return State::InvalidPiecePU;
                }
            }
            else{
                return State::Error;
            }
            break;
        case FriendlyPU:
            if(instruction == state_memory[0]){
                return State::Idle;
            }
            else if ((friendlies & square) != 0ULL){
                state_memory[1] = instruction;
                return State::InvalidPiecePU;
            }
            else{
                for(int i = 0; i < current_square_moves->length; i++){
                    if(current_square_moves->moves[i].target_square == instruction){
                        if(current_square_moves->moves[i].capture != 0){
                            state_memory[1] = instruction;
                            return State::FriendlyAndEnemyPU;
                        }
                        chess->make_move(side, current_square_moves->moves[i]);
                        printf("MOVE COMMITTED \n");
                        return State::MoveComplete;
                    }
                }
            }
            break;
        case EnemyPU:
            if(instruction == state_memory[0]){
                return State::Idle;
            }
            else if((enemies & square) != 0ULL){
                state_memory[1] = instruction;
                return State::InvalidPiecePU;
            }
            else{
                for(int i = 0; i < current_square_moves->length; i++){
                    if(current_square_moves->moves[i].source_square == instruction){
                        state_memory[1] = instruction;
                        return State::FriendlyAndEnemyPU;
                    }
                }
            }
            break;
        case FriendlyAndEnemyPU:
            int prev_friendly_square;
            int prev_enemy_square;
            if(is_friendly(chess, side, state_memory[0])){
                prev_friendly_square = state_memory[0];
                prev_enemy_square = state_memory[1];
            }
            else{
                prev_friendly_square = state_memory[1];
                prev_enemy_square = state_memory[0];
            }
            printf(">>>>>%d\n", prev_friendly_square);
            printf(">>>>>%d\n", prev_enemy_square);

            if(instruction == prev_friendly_square){
                state_memory[0] = prev_enemy_square;
                return State::EnemyPU;
            }
            else if(instruction == prev_enemy_square){
                printf("CAPTURED!\n");
                for(int i = 0; i < current_square_moves->length; i++){
                    if(current_square_moves->moves[i].target_square == prev_enemy_square && 
                       current_square_moves->moves[i].source_square == prev_friendly_square){
                        chess->make_move(side, current_square_moves->moves[i]);
                        printf("MOVE COMMITTED \n");
                        return State::MoveComplete;
                    }
                }
            }
            else{
                return State::Error;
            }
    }

    return state;
}

/*********************************************************************************\
;---------------------------------------------------------------------------------;
;                                      Main                                       ;
;---------------------------------------------------------------------------------;
\*********************************************************************************/

int main() {
    //variable setup
    Chess chess;
    enum State state = Idle;
    int state_memory[] = {0};
    Move_List move_list[1];
    Move_List current_square_moves[1];

    printf("hello world\n");
    printf("%d \n", state);

    print_bitboard(chess.get_occupied());
    printf("\n");
    int side = 8; //8 = white, 16 = black
    while(true){chess.generate_moves(side,128/*change later*/, move_list, 0);
        chess.print_board();
        while(true){
            int square;
            printf("Type number of square: \n");
            scanf("%d", &square);
            printf("the square is %d \n", square);
            state = update_state(&chess, square, side, state, state_memory, move_list, current_square_moves);
            printf("current state is: %d \n", state);
            printf("State memory 0: %d\n", state_memory[0]);
            printf("State memory 1: %d\n", state_memory[1]);
            if(state == State::MoveComplete) break;
        }
        state = State::Idle;
        printf("move complete\n");
        side = 24 - side;
    }
}

