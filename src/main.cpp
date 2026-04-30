#include <raylib.h>
#include <iostream>
#include <cstdio>
#include <sstream>

bool side;
std::string ip;
bool between(int value, int min, int max) {
  return value > min && value < max;
}
std::string exec(std::string cmd) {
  FILE* pipe = popen(cmd.c_str(), "r");
  if(!pipe) {
    perror("popen failed");
    return "";
  }
  char buffer[1024];
  while(fgets(buffer, sizeof(buffer), pipe)) {
     break;
  }
  int exit_status = pclose(pipe);
  if(exit_status == -1) {
    perror("pclose failed");
  }
  return buffer;
}
int GetWinner(int matrix[3][3]) {
  const int height = GetScreenHeight();
  const int width = GetScreenWidth();
  const int line = height*0.8;
  const float y = (height-line)/2+height*0.05;
  const float x = width/2-line/2;
  const int len = line/3;

  for(int i = 0; i<3; i++) {
    if(matrix[i][0] == matrix[i][1] && matrix[i][1] == matrix[i][2] && matrix[i][0] != 0) {
      DrawLineEx((Vector2){x+len*(i+0.5f), y}, (Vector2){x+len*(i+0.5f), y+line}, 3, GREEN);
      return matrix[i][0];
    } else if(matrix[0][i] == matrix[1][i] && matrix[1][i] == matrix[2][i] && matrix[0][i] != 0) {
      DrawLineEx((Vector2){x, y+len*(i+0.5f)}, (Vector2){x+line, y+len*(i+0.5f)}, 3, GREEN);
      return matrix[0][i];
    }
  }
  if(matrix[0][0] == matrix[1][1] && matrix[1][1] == matrix[2][2] && matrix[1][1] != 0) {
    DrawLineEx((Vector2){x, y}, (Vector2){x+line, y+line}, 3, GREEN);
    return matrix[1][1];
  }
  if(matrix[0][2] == matrix[1][1] && matrix[1][1] == matrix[2][0] && matrix[1][1] != 0) {
    DrawLineEx((Vector2){x+line, y}, (Vector2){x, y+line}, 3, GREEN);
    return matrix[1][1];
  }
  return 0;
}
int setup() {
  int opt;
  std::cout << "1. Host a game\n2. Join a game\n> ";
  std::cin >> opt;
  if(opt == 1) {
    side = false;
    std::cout << "Waiting for client\n";
    std::string out = exec("netcat -l -p 2137");
    out.pop_back();
    std::cout << "Client joined! IP: " << out << "\n";
    ip = out;
    return 0;
  } else {
    side = true;
    std::string tmp;
    std::cout << "Your IP: ";
    std::cin >> tmp;
    std::cout << "Server IP: ";
    std::cin >> ip;
    std::stringstream ss;
    ss << "echo " << tmp << " | netcat -q0 " << ip << " 2137";
    
    std::cout << ss.str();
    std::string out = exec(ss.str());
  }
  return 0;
}

int main() {
  if(setup() != 0) {
    std::cout << "Unknwon exception!\n";
    return 1;
  }
  InitWindow(800, 700, "Kolko i Krzyzyk");
  SetTargetFPS(15);
  
  int matrix[3][3] = {{0,0,0},{0,0,0},{0,0,0}};
  bool turn = false;
  bool play = true;
  int win = 0;
  while(!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    // Rysowanie linii
    const int height = GetScreenHeight();
    const int width = GetScreenWidth();
    const int line = height*0.8;
    const int y = (height-line)/2+height*0.05;
    const int x = width/2-line/2;
    const int len = line/3;
    DrawLine(x, y+len, x+line, y+len, WHITE);
    DrawLine(x, y+len*2, x+line, y+len*2, WHITE);
    DrawLine(x+len, y, x+len, y+line, WHITE);
    DrawLine(x+len*2, y, x+len*2, y+line, WHITE);

    if(win != 0) {
      std::string str = win == 1 ? "o won" : "x won";
      DrawText(str.c_str(), width/2-MeasureText(str.c_str(), height*0.05)/2, height*0.05, height*0.05, GREEN);
    } else if(!play) {
      DrawText("Tied", width/2-MeasureText("Tied", height*0.05)/2, height*0.05, height*0.05, YELLOW);
    } else if(turn) {
      DrawText("Turn: x", width/2-MeasureText("Turn: x", height*0.05)/2, height*0.05, height*0.05, WHITE);
    } else {
      DrawText("Turn: o", width/2-MeasureText("Turn: o", height*0.05)/2, height*0.05, height*0.05, WHITE);
    }

    int won = GetWinner(matrix);
    if(won != 0) {
      win = won;
      if(play) SetTargetFPS(15);
      play = false;
    }
    
    bool stop = true;
    for(int i = 0; i<3; i++) {
      for(int j = 0; j<3; j++) {
        const int num = matrix[i][j];
        const int size = len*0.8;
        if(num == 1) {
          const Vector2 w = MeasureTextEx(GetFontDefault(), "o", size, 0);
          DrawText("o", x+len*i+(len/2-w.x/2), y+len*j+(len/2-w.y/2), size, BLUE);
        } else if(num == 2) {
          const Vector2 w = MeasureTextEx(GetFontDefault(), "x", size, 0);
          DrawText("x", x+len*i+(len/2-w.x/2), y+len*j+(len/2-w.y/2), size, RED);
        } else stop = false; 
      }
    }
    if(play && stop) {
      SetTargetFPS(15);
      play = false;
    }

    EndDrawing();
    if(!play) {
      if(IsKeyPressed(KEY_R)) {
        play = true;
        turn = false;
        win = 0;
        for(int i = 0; i<3; i++) {
          for(int j = 0; j<3; j++) {
            matrix[i][j] = 0;
          }
        }
      }
      continue;
    };

    if(side == turn && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      Vector2 pos = GetMousePosition();
      if(between(pos.x, x, x+line) && between(pos.y, y, y+line)) {
        int clickX = 2, clickY = 2;
        if(between(pos.x, x, x+len)) {
          clickX = 0;
        } else if(between(pos.x, x+len, x+len*2)) {
          clickX = 1;
        }
        if(between(pos.y, y, y+len)) {
          clickY = 0;
        } else if(between(pos.y, y+len, y+len*2)) {
          clickY = 1;
        }

        if(matrix[clickX][clickY] == 0) {
          std::stringstream ss;
          ss << "echo " << clickX << clickY << " | netcat -q0 " << ip << " 2137";
          std::cout << "you: " << ss.str() << "\n";
          std::string out = exec(ss.str());
          matrix[clickX][clickY] = turn+1;
          turn = !turn;
        }  
      }
    } else if(side != turn) {
      std::string out = exec("netcat -l -p 2137");
      int clickX = out[0] - '0';
      int clickY = out[1] - '0';

      matrix[clickX][clickY] = turn+1;
      turn = !turn;
    }
  }
  CloseWindow();
}
