

//checks if number with a bitwise AND with -1
//if even sets return_val to 0
//otherwise to 1
//returns to return_location
    (IS_EVEN)//0
    @R0//0
    D=M;//1
    @tmp//2
    M=1;//3
    D=M&D;//4
    @return_val//5
    M=D;//6
