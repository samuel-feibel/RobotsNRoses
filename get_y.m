function [y_val] = get_y(y, d)

if (d == 3) % going north 
    y_val =  y - 1; 
elseif (d == 1) % going south                 
    y_val = y + 1;    
else 
    y_val = y;
end

end 