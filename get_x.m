function [x_val] = get_x(x, d)

if (d == 2)  % going west
    x_val = x - 1; 
elseif (d == 0)  % going east                   
    x_val = x + 1;    
else 
    x_val = x;    
end

end 