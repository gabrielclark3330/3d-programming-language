# Programming Language Specification:

## Description:
The language described here is a 3D Esolang, similar to ARGH. Its execution space is a 3D grid where instructions are executed based on the current direction of the instruction pointer and its location within the grid. This language interacts with a stack, and the grid can be thought of as both the program and memory.

## Basic Concepts:

1. **State (3D grid)**: The execution space.
2. **Instruction Pointer (IP)**: Points to the current cell in the state that is being executed. The IP moves in a specific direction.
3. **Instruction Direction**: Defines the direction in which the IP should move after executing an instruction.
4. **Stack**: A Last-In-First-Out (LIFO) data structure used to store and manipulate values.

## Commands:

### Data Manipulation:

1. **`A`**: Add the value of the cell above the current cell to the value on top of the stack.
2. **`a`**: Add the value of the cell below the current cell to the value on top of the stack.
3. **`D`**: Delete (pop) the top value off the stack.
4. **`d`**: Duplicate the top value on the stack.
5. **`F`**: Fetch (pop) the value from the top of the stack and store it in the cell above.
6. **`f`**: Fetch (pop) the value from the top of the stack and store it in the cell below.
7. **`G`**: Get one byte from stdin and store in the cell above the IP.
8. **`g`**: Get one byte from stdin and store in the cell below the IP.
9. **`R`**: Reduce the value on top of the stack by the value of the cell above.
10. **`r`**: Reduce the value on top of the stack by the value of the cell below.
11. **`S`**: Store (push) the value of the cell above the current cell onto the stack.
12. **`s`**: Store (push) the value of the cell below the current cell onto the stack.

### Output:

1. **`P`**: Send the value in the cell above the current cell to stdout.
2. **`p`**: Send the value in the cell below the current cell to stdout.

### Control Flow:

1. **`H`**: Jump (move IP) left to the next cell whose value matches the value on top of the stack and set the execution direction to left.
2. **`h`**: Set the execution direction to left.
3. **`J`**: Jump down to the next cell whose value matches the value on top of the stack and set the execution direction to down.
4. **`j`**: Set the execution direction to down.
5. **`K`**: Jump up to the next cell whose value matches the value on top of the stack and set the execution direction to up.
6. **`k`**: Set the execution direction to up.
7. **`L`**: Jump right to the next cell whose value matches the value on top of the stack and set the execution direction to right.
8. **`l`**: Set the execution direction to right.
9. **`W`**: Jump in the positive z-direction to the next cell whose value matches the value on top of the stack and set the execution direction to positive z.
10. **`w`**: Set the execution direction to positive z.
11. **`E`**: Jump in the negative z-direction to the next cell whose value matches the value on top of the stack and set the execution direction to negative z.
12. **`e`**: Set the execution direction to negative z.
13. **`X`**: If the value on top of the stack is negative, turn the execution direction 90 degrees to the left.
14. **`x`**: If the value on top of the stack is positive, turn the execution direction 90 degrees to the right.
15. **`q`**: Quit/end program execution.

### Special:

1. **`#`**: Behaves like `j` but only if its position in the code/data array is (0,0) and only if there's a `!` in the cell on its right side.

## Notes:

- This language is Turing complete given infinite state size.
- The state's size is defined by stateSize.x, stateSize.y, and stateSize.z respectively for dimensions.
- The IP's position is defined by instructionPointer.x, instructionPointer.y, and instructionPointer.z respectively.
- The values in the grid cells and stack are expected to be char values.
- There are checks in place to ensure IP doesn't go out of bounds in the state grid.

Remember, this language functions in a 3D grid. As such, it can execute in all three dimensions allowing for very intricate designs and interactions. Programmers should carefully design their program grid to ensure the desired behavior is achieved.

# To run
1. Install raylib
2. Open `3d-programming-language/d3argh/rayLibArgh
/rayLibArgh` in Visual Studio and run `rayLibArgh.cpp`

# .3D file format
Certainly! Based on the example you've given, I'll attempt to provide a general guideline or format for writing programs in this "language". 

### Program Structure for the 3D Language:

1. **Initialization:**
```
Size()
<dimensions>
```
This specifies the size of the grid. In the provided example, it's `4,4,4`.

2. **Program Body:** 
The program body consists of a series of layers (in this example, four layers). Each layer consists of rows and each row consists of cells, similar to a 3D matrix or tensor.

- Each cell contains either a command, a number, a space (interpreted as a null operation), or a combination of these.
- Commands are executed based on the current cell's position and contents. The semantics of commands are implied but not fully defined in your examples. However, based on some observations:
  - `s`, `d`, `r`, `e`, `j`, `l`, `w`, `h`, `k`, `p`, `f`, `A`, and `F` seem to be commands.
  - Some commands (e.g., `j`) appear multiple times, possibly implying looping or jumping mechanisms.

3. **Delimiter:**
```
-----
```
This separates each layer. It’s essential for distinguishing between the layers of the 3D matrix.

### Guidelines:

1. **Dimension Declaration:** Start the program with the `Size()` function, followed by the grid's dimensions.
   
2. **Layered Structure:** Always ensure the program is structured in layers, separated by the delimiter `-----`.

3. **Consistent Dimensions:** Ensure that the number of rows and cells in each row is consistent with the declared dimensions.

4. **Commands and Numbers:** Use commands and numbers within cells to carry out operations. The exact semantics of each command would need further clarification.

5. **Whitespace:** Utilize spaces for null operations or placeholders. This helps maintain the grid's shape when not all cells in a row have operations.

6. **End of File:** The program concludes when all layers are defined, and no additional delimiters are required after the last layer.

For a thorough understanding and a complete guide, you'd need to provide a detailed list of commands and their exact functions. The example given hints at complex operations such as loops, but without a clear command list, it's speculative to assume their exact behavior.


# Example Program
## A simple accumulator looks as follows
```
Size()
4,4,4
sdre,
1 0j,
  j,
llle,
-----
e,
  f,
   ,
w  A,
-----
j,
  w,
   ,
w  d,
-----
jhhh,
jlpk,
jk,
wkFh,
-----
```
