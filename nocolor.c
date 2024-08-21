/*  
    This is a small tool to strip ANSI control code sequences
    from text that is piped to it via stdin, and outputs the 
    cleaned text to stout.

    Reason:  

    By default I usually have other tools such as: 
    ls, grep, ip, dmesg, diff, less, jq, etc., configured 
    to output colors, which is nice.

    The side-effect is when redirecting their output to a text
    file, such as:  ls -l > out

    Viewing the file with 'vi', all the raw ANSI escape
    code sequences from the color output are preserved, 
    it looks like this:

    ^[[0m^[[0;38;2;225;225;225mdrwxr-x--- 23 root
    ^[[0;38;2;225;225;225mdrwxr-xr-x 17 root root
    ^[[0;38;2;225;225;225m-rw-------  1 root root
    ^[[0;38;2;225;225;225m-rw-------  1 root root

    Of course you can just re-run 'ls' with the --color=none option.
    
    It becomes more of a pain when stringing together multiple
    tools that output color in a pipeline: 
    
    $ ls -l | grep foo | grep bar > out  

    Then you have to add a --color=none to each component of 
    the pipeline command.

    Instead, adding this tool at the end of the pipeline will 
    strip out all the the ANSI color sequences from everything.

    $ ls -l | grep foo | nocolor > out

    Other things that work, echoing escape sequences using echo.

    $ echo -e "\033[38;2;255;0;0mTHIS_IS_RED" | nocolor

    Normally prints in red, but 'nocolor' detects and removes it.

    To build:

    No need for a Makefile, simply run:

    $ gcc -O3 nocolor.c -o nocolor

    Some day, perhaps will make into a kernel module that does 
    it automatically for all text mode output to files.

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


void PrintUsage() {

    printf( "\n" );
    printf( "Program is intended to receive data via stdin pipe.\n" );
    printf( "\n" );
    printf( "Examples:\n" );
    printf( "cat somefile | ./nocolor\n" );
    printf( "echo 'somedata' | ./nocolor\n" );
    printf( "\n");
    return;
}


int main( int argc, char* argv[] ) {

    char Escape      = 0x1B;
    char Bracket     = '[';
    char c           = 0;
    int  StartSeq    = 0;
    int  FoundSeq    = 0;

    if ( isatty( STDIN_FILENO )) {
        PrintUsage();
        return(1);
    }


    /*  
        Loop while reading chars from stdin, one at a time.

        Set flags to track when the Control Sequence Introducer
        (CSI) sequence is found ( "ESC[" ), and when it ends.

        I tried to make it more 'clever' but just screwed things 
        up, so this is the simple way.

    */
        

    while ( read( STDIN_FILENO, &c, 1 ) > 0 ) {

        /*  Find a CSI  */

        if (( c == Escape ) && ( ! StartSeq ) && ( ! FoundSeq )) {

            StartSeq = 1;
            continue;

        }

        if (( c == Bracket ) && ( StartSeq ) && ( ! FoundSeq )) {

            FoundSeq = 1;
            StartSeq = 0;
            continue;

        }
        else
            StartSeq = 0;


        /*  If we haven't found a CSI, write the char to stdout */

        if ( ! FoundSeq )

            write( STDOUT_FILENO, &c, 1 );


        /*  If we found a CSI, don't write anything to stdout
            until we've reached the end of the sequence */
        
        else

            if (( c >= 0x40 ) && ( c <= 0x7E )) {

                /*  Per the ANSI control sequence documentation,
                    the final character of a sequence will be
                    in the range:  0x40 -> 0x7E, so reset the 
                    flags and wait for the next CSI. */

                FoundSeq = 0;
                StartSeq = 0;

            }


        /*  Loop around and read the next char from stdin */

    }

    return(0);

}
