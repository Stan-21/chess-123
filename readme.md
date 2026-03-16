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

# Chess Movement 2
Bishop / Rook / Queen Movement
- To implement the moves for these pieces I follow a similar pattern to the Knight and King.  I looped through every piece of that type and checked what moves were valid.  I used the MagicBitboards.h file that was provided by the professor to generate the possible moves for Bishops, Rooks, and Queens and used those moves to determine which ones were valid.  The other change that I added to the movement for these pieces was a self_occupancy variable to prevent them from capturing pieces of the same color.
- To use the MagicBitBoard I called initMagicBitboards() when the game is created and also called cleanupMagicBitBoards() to free up the memory when stopping the game.

- NOTE: There is a second screenshot of the first 20 moves just like the assignment asked for, but bishop / rook / queen moves don't show up cause they cannot move on turn one.

# Chess AI (Submission 1)
- My chess AI uses the negamax algorithm with alpha beta pruning to determine what the 'optimal' move is for the AI.  As of now, the AI can only play as black and can run at a depth of four with a reasonable amount of delay between moves.  The main challenge for me came from filtering out the moves.  I tried to follow the lecture video on this, but with the audio gone it was a bit difficult to correctly follow so I had to do my own research.  
- The evaluation function for my AI consists of two parts: assigning points to each piece and weights on optimal positions for each piece.  For each piece I just assigned a value based on what was given online and created a table with weights on each board position.
- Leaving this at submission 1 cause I would like to go back (hopefully before the quarter ends) and optimize it.