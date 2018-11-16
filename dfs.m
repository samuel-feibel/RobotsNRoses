function dfs = dfs()

global maze;
global pos; 
global dir;
global curr_node;
global wall_loc;

right = mod((dir + 1), 4);
left  = mod((dir + 3), 4);

x = pos(1);
y = pos(2);
curr_node = maze(y, x, :);
if (~curr_node(2)) % (!maze[x][y].visited)
    maze(y, x, :) = [dir 1];
end 
maze(1, 1, 1) = 2; % corner case
curr_node = maze(y, x, :); % called twice bc not a ptr to maze 

%disp(maze(:, :, 2));

f_x = get_x(x, dir); % pos of node in front of the current node
f_y = get_y(y, dir);

r_x = get_x(x, right); % pos of node to the right of the current node  
r_y = get_y(y, right);

l_x = get_x(x, left); % pos of node to the left of the current node
l_y = get_y(y, left);

wall_bin = de2bi(wall_loc(y,x), 4, 'right-msb'); % wall stuff 
if dir == 0
    r_wall = wall_bin(2); % S
    l_wall = wall_bin(1); % N
    f_wall = wall_bin(3); % E 
elseif dir == 1
    r_wall = wall_bin(4); % W
    l_wall = wall_bin(3); % E
    f_wall = wall_bin(2); % S
elseif dir == 2 % going west
    r_wall = wall_bin(1); % N
    l_wall = wall_bin(2); % S
    f_wall = wall_bin(4); % W
elseif dir == 3 
    r_wall = wall_bin(3); % E
    l_wall = wall_bin(4); % W
    f_wall = wall_bin(1); % N
end

if (r_wall)
    r_block = 1;
else 
    r_block = maze(r_y, r_x, 2);
end

if (l_wall)
    l_block = 1;
else 
    l_block = maze(l_y, l_x, 2);
end

if (f_wall)
    f_block = 1;
else
    f_block = maze(f_y, f_x, 2);
end
disp(f_block);
disp(r_block);
disp(l_block);
    
if (r_block && l_block && f_block)  % 3 sides blocked 
    dir = mod(curr_node(1) + 2, 4); % (curr_node.node_dir + 2) % 4; 
elseif (f_block && ~r_block)  % front blocked and right not blocked
    dir = right;
elseif (f_block && ~l_block)  % front blocked and left not blocked
    dir = left; 
end % else dir = dir

% update pos
curr_dir = mod(dir, 2) + 1;
if (dir == 3 || dir == 2)  % going west or north
    pos(curr_dir) = pos(curr_dir) - 1; 
else                       % going east or south
    pos(curr_dir) = pos(curr_dir) + 1;  
end
%disp(pos);

end 
        
