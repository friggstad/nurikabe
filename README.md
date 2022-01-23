# nurikabe
A nurikabe solver I worked on while bored during the pandemic.

Compile with "make"

Run with
./nurikabe <filename> [--step]

The filename is the .txt file encoding an instance, see the examples included
in the puzzles directory.

The argument --step is optional and will cause the program to wait for
the user to hit "return" after each new character is added to the solution.

The sample puzzles are from the Nurikabe app by Conceptis.

The first line of the puzzle format gives # rows and # columns.
Then a series of lines follow, each describing one numbered square by
giving its row index, column index, and then the value in the square.
Top row is 0, leftmost column is 0.
