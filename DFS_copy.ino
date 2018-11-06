enum _direction{E, S, W, N};
byte dir = E;
byte pos[2] = {0,0};

typedef struct {
    byte node_dir; // dir robot was facing when it entered that node 
    bool visited;
} node;
node curr_node;
node maze[9][9]; // maze[x][y]

void setup() {
    curr_node = {dir, 1};
    for(int i = 0;  i < 9;  i++) {
        for(int j = 0; j < 9; i++) {
            maze[i][j] = {dir, 0};
        }
    }
}

void loop() {
    dfs();
}

int get_x(int x, int d){
    if (d == W) { // going west
        return x - 1; 
    } else if (d == E) { // going east                   
        return x + 1;    
    } else {
        return x;    
    }
}

int get_y(int y, int d){
    if (d == N) { // going north 
        return y - 1; 
    } else if (d == S){ // going south                 
        return y + 1;    
    } else {
        return y;
    }
}

void dfs() {  
    byte right = (dir + 1) % 4;
    byte left = (dir + 3) % 4;
    
    byte x = pos[0]; // current node 
    byte y = pos[1];
    curr_node = maze[x][y];
    if (!maze[x][y].visited) { // if curr_node not visited 
        curr_node.node_dir = dir;
        curr_node.visited = 1;
    }

    int f_x = get_x(x, dir); // pos of node in front of the current node
    int f_y = get_y(y, dir);
    
    int r_x = get_x(x, right); // pos of node to the right of the current node  
    int r_y = get_y(y, right);
    
    int l_x = get_x(x, left); // pos of node to the left of the current node
    int l_y = get_y(y, left);
    
    // blocked means wall or node visited
    bool r_block = maze[r_x][r_y].visited; // || right_wall;
    bool l_block = maze[l_x][l_y].visited; // || left_wall;
    bool f_block = maze[f_x][f_y].visited; // || forward_wall;
     
    if (r_block && l_block && f_block) { // 3 sides blocked 
        dir = (curr_node.node_dir + 2) % 4; 
    } else if (f_block && !r_block) { // front blocked and right not blocked
        dir = right;
    } else if (f_block && !l_block) { //front blocked and left not blocked
        dir = left;   
    } // else dir = dir
}
