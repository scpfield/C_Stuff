#include <stdio.h>

/* 

    This is the implementation of an interview question which
    asked:

    Given a list of numbers incrementing by 1, but with gaps,
    print the sequences of numbers incrementing by 1 and the
    numbers with gaps.

    Sample input:

    1, 2, 3, 4, 5, 10, 20, 22, 23, 24

    Output:

    1-5, 10, 20, 22-24

*/


int main()
{
    /*  Test array of integers with gaps, duplicates,
        and negative numbers.   */

    int Numbers[]   = {  1, 2, 3, 4, 5, 1, 15, 16, 17, 18, 19, 
                        21, 23, 25, 25, 25, -10, -9, -8, 26, 
                        27, 28, 29, 50, 51, 52, 53, 0, 1, 2 };

    int NumbersLen  = (int) sizeof( Numbers ) / sizeof(int);

    /*  Tracking pointers for the Start + End elements 
        of a sequence of numbers increasing by 1.   */

    int* StartSequence  =  &Numbers[0];
    int* EndSequence    =  StartSequence + 1;
    
    /*  Print the test array    */

    printf( "\nTest Numbers Array:\n" );

    for ( int i = 0; i < NumbersLen; i++ ) {

        printf( "%d", Numbers[i] );

        if ( i < ( NumbersLen - 1 )) {
            printf( ", " );
        }
    }
    printf( "\n" );

    /*  Walk the Numbers array using the array pointers 
        until we reach the end */
    
    printf( "\nSequences:\n" );

    while ( EndSequence <= &Numbers[ NumbersLen ]) {

        /*  Look backwards from the EndSequence pointer to see if the 
            numbers in the array are increasing by 1, by dereferencing
            EndSequence and (EndSequence - 1), and subtracting the values. */
            
        if (( *EndSequence - *(EndSequence - 1) == 1 )) {

            /*  If the subtraction result is 1, then increment the
                EndSequence pointer to the address of the next element 
                in the array.  Pointer arithmetic will advance it by
                sizeof(int).  */

            EndSequence += 1;

        }
        else {

            /*  If the subtraction result is not 1, print the 
                start + end values of the sequence. */

            if ( StartSequence == ( EndSequence - 1 )) {

                /*  If the Start and End-1 are the same address, 
                    then just print the number.  */

                printf( "%d", *StartSequence );

            }
            else {
                
                /*  Otherwise, we have a range, so print both values. */
                    
                printf( "%d:%d", *StartSequence, *(EndSequence - 1));

            }

            /* Prepare the tracking pointers for the next sequence. */

            StartSequence   = EndSequence;
            EndSequence     = StartSequence + 1;

            /* Print a comma if we're not at the end */

            if ( EndSequence < &Numbers[NumbersLen] ) {
                printf( ", " );
            }
        }
    }

    printf( "\n" );
    return 0;

}


/*  TEST OUTPUT

Test Numbers Array:
1, 2, 3, 4, 5, 10, 15, 16, 17, 18, 19, 21, 23, 25, 25, 25, -10, -9, -8, 26, 27, 28, 29, 50, 51, 52, 53, 0, 1, 2

Sequences:
1:5, 10, 15:19, 21, 23, 25, 25, 25, -10:-8, 26:29, 50:53, 0:2

*/
