# FEN Board Setup
Mainly copied the lecture code, but the general gist is that it checks each letter.  Depending on whether it is capitalized or not, it sets it to the corresponding piece of the matching color. 

# Chess Movement 1
Knight Movement
- To implemented Knight movement, I first generated all of the possible ways that a knight could move given an x position as a BitboardElement.  To easily figure out the directions and movements, I used vectors with the corresponding <2, 1> pairs need to move in an L shape.
- Then when generating the moves, I would use the KnightMovesBitBoard to see where the Knight can move to.  If it a valid move in the KnightMovesBitBoard and a friendly piece isn't on that square, then the Knight can move there.
King Movement
- Similarly to the Knight movement, I also created a KingMoveBitBoard, but instead of an L shape I used adjacent squares.
- Just like the Knight, I would use that bit board to see where the King can move to.  If it is valid without a friendly piece, then the King could be moved.
Pawn Movement
- To do the pawn movement, I first split it up into three steps.  Moving one step forward, moving two steps forward if it is on the starting square, and capturing diagonal enemy pieces.
- To do the forward movement, I first checked the color of the pieces and set the direction variable accordingly.  Then to give the option to move twice, I made it so pawns of a certain color have the option to move twice on a given row.  While it is supposed to be the first movement that allows double movement, NOT a certain row, pawns cannot move backwards so this implementation works.
- Finally, to do the diagonal capturing, I checked the forward diagonal spaces for each pawn.  If there is an enemy piece, then the pawn could move there.