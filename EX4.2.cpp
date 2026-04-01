#include <iostream>
#include <random>
#include <thread>
#include <array>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <cstdlib>

// Classe TicTacToe
class TicTacToe {
private:
    std::array<std::array<char, 3>, 3> board; // Tabuleiro do jogo
    char current_player;                      // Jogador atual ('X' ou 'O')
    bool game_over;                           // Estado do jogo
    char winner;                              // Vencedor do jogo
  
    std::mutex mtx;
    std::condition_variable cv;

public:
    TicTacToe() {
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                board[i][j] = ' ';
            }
        }
        winner = '-';
        game_over = false;
        
        std::mt19937 sorteiaJogador(static_cast<unsigned int>(time(0)));
        std::uniform_int_distribution<int> distr(0, 1);
        current_player = distr(sorteiaJogador) == 0 ? 'X' : 'O';
    }
  
    void display_board() {
        
        std::cout << "\x1B[2J\x1B[H"; 
        
        for(int i = 0; i < 3; i++){
            std::cout << " " << board[i][0] << " | " << board[i][1] << " | " << board[i][2] << std::endl;
            if(i != 2){
                std::cout << "---|---|---" << std::endl;
            }
        }
        std::cout << std::endl;
    }
  
    bool make_move(char player, int row, int col) {
       
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [this, player]() {
            return current_player == player || game_over;
        });

        if (game_over) {
            return true;
        }

        if (board[row][col] == ' ') {
            board[row][col] = player;
            display_board();

            if (check_win(player)) {
                winner = player;
                game_over = true;
            } else if (check_draw()) {
                winner = 'D';
                game_over = true;
            } else {
        
                current_player = (player == 'X') ? 'O' : 'X';
            }

            
            cv.notify_all();
            return true; 
        }

    
        return false;
    }
  
    bool check_win(char player) {
        
        for(int i = 0; i < 3; i++){
            if(board[i][0] == player && board[i][1] == player && board[i][2] == player) return true;
        }
        
        for(int i = 0; i < 3; i++){
            if(board[0][i] == player && board[1][i] == player && board[2][i] == player) return true;
        }
    
        if(board[0][0] == player && board[1][1] == player && board[2][2] == player) return true;
        if(board[0][2] == player && board[1][1] == player && board[2][0] == player) return true;
        
        return false;
    }
  
    bool check_draw() {

        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
                if(board[i][j] == ' '){
                    return false;
                }
            }
        }
        return true;
    }
  
    bool is_game_over() {
        std::lock_guard<std::mutex> lock(mtx);
        return game_over;
    }
  
    char get_winner() {
    
        std::lock_guard<std::mutex> lock(mtx);
        return winner;
    }
};

// Classe Player
class Player {
private:
    TicTacToe& game;
    char symbol;
    std::string strategy;
  
public:
    Player(TicTacToe& g, char s, std::string strat) 
    : game(g), symbol(s), strategy(strat) {}
  
    void play() {
        // Executa enquanto o jogo não acabou
        while(!game.is_game_over()){
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            if(strategy == "sequential"){
                play_sequential();
            } else {
                play_random();
            }
        }
    }
  
private:
    void play_sequential() {
        for(int i = 0; i < 3; i++){
            for(int j = 0; j < 3; j++){
            
                if(game.is_game_over()) return;
                
                
                if(game.make_move(symbol, i, j)){
                    return;
                }
            }
        }
    }
  
    void play_random() {
        int l, c;
        bool move_success = false;
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, 2);
        
        while(!move_success && !game.is_game_over()){
            l = distr(gen);
            c = distr(gen);
            move_success = game.make_move(symbol, l, c);
        } 
    }
};

// Função principal
int main() {
    // Inicializar o jogo e os jogadores
    TicTacToe tabuleiro;
    tabuleiro.display_board();
    
    Player X(tabuleiro, 'X', "sequential");
    Player O(tabuleiro, 'O', "random");
  
    // Criar as threads para os jogadores
    std::thread Jogador1(&Player::play, &X);
    std::thread Jogador2(&Player::play, &O);
  
    // Aguardar o término das threads
    Jogador1.join();
    Jogador2.join();
  
    // Exibir o resultado final do jogo
    char vencedor = tabuleiro.get_winner();
    if(vencedor == 'D'){
        std::cout << "\nO jogo terminou em Empate!\n";
    } else {
        std::cout << "\nVencedor: Jogador " << vencedor << "!\n";
    }
  
    return 0;
}