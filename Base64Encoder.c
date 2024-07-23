#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <io.h>

/*

    This is my implementation a base64 encoder, which is used in MIME 
    email formats for binary attachments, and in other protocols.

    The base64 RFC-4648 specification calls for taking 3 bytes of 
    8-bit data, and converting it into 4 bytes of 6-bit data (the 
    two highest bits of the 4 output bytes are unused).

    The values of the 6-bit bytes are then used as indexes into a 
    table of printable ASCII characters, defined in the RFC-4648,
    for which there are 64 possible values, because 64 is the largest 
    unsigned 6-bit number.  (2^6 = 64).  That is the reason it is 
    called "base64" encoding.

    Basically, we want to convert these bits:

    11111111-11111111-11111111

    Into these bits:

    00111111-00111111-00111111-00111111

    The program accepts data input via stdin.

    In a Windows CMD prompt:

        echo Hello World | Base64Encoder.exe

        "echo" adds a newline, to remove it:

        echo | set /p="Hello World" | Base64Encoder.exe

    Currently this likely only compiles on Windows because of 
    the way the stdin calls are.

*/


unsigned char GetBase64Char( unsigned char InputValue );


int main()
{
    unsigned char InputBytes[3] = { 0 };
   
    _setmode( _fileno( stdin ), _O_BINARY );
    
    while( 1 ) { 
                            
        /*  Initialize variables for this iteration of the loop */

        unsigned char InputByte1    = 0;
        unsigned char InputByte2    = 0;
        unsigned char InputByte3    = 0;
        unsigned char NewByte1      = 0;
        unsigned char NewByte2      = 0;
        unsigned char NewByte3      = 0;
        unsigned char NewByte4      = 0;


        /*  Read the next 0-3 bytes from stdin.  
            0 bytes means EOF.  */

        int InputByteCnt            = _read(
                                        _fileno( stdin ), 
                                        InputBytes, 
                                        3 );

        /*  When we reach near the end of the input bytes, 
            we might not have an even multiple of 3 bytes to 
            work with.
            
            If we have 1 input byte remaining, RFC says to use
            "0" as the 2nd input byte value to generate the first
            two Base64 output chars, then use the "=" symbol 
            as padding for the last two Base64 output chars.

            If we have 2 input bytes remaining, RFC says to use 
            "0" as the third input byte value to generate the first
            three Base64 output chars, then use the "=" symbol 
            as padding for the last Base64 output char.
        */

        if (InputByteCnt == 0) {
            break;
        }

        if (InputByteCnt > 0) {
            InputByte1 = InputBytes[0];
        }

        if (InputByteCnt > 1) {
            InputByte2 = InputBytes[1];
        }

        if (InputByteCnt > 2) {
            InputByte3 = InputBytes[2];
        }


        /*  Start by taking the first 6 bits from the the 1st input byte,
            and shifting them to the right by 2 to get them into
            position to make a new 6-bit value.

            Bits we want from 1st byte:     1111 1100  (0xFC)
            Bits after shifting >> 2:       0011 1111

        */

        NewByte1 = (InputByte1 & 0xFC) >> 2;

        /*  Next, take the last two bits from the 1st byte, and
            the first 4 bits from the 2nd byte, shift both into 
            proper positions, then combine to make a new 6-bit value.

            Bits we want from 1st byte:     0000 0011  (0x03)
            Bits after shifting << 4:       0011 0000

            Bits we want from 2nd byte:     1111 0000  (0xF0)
            Bits after shifting >> 4:       0000 1111

            Combined into new 6-bit value:  0011 1111
        */

        NewByte2 =  ((InputByte1 & 0x03) << 4) |
                    ((InputByte2 & 0xF0) >> 4);


        if (InputByteCnt > 1) {

            /*  Next, take the last 4 bits from the 2nd byte, and
                the first 2 bits from the 3rd byte, shift both
                into position, then combine to make a new 6-bit value.

                Bits we want from 2nd byte:         0000 1111  (0x0F)
                Bits after shifting << 2:           0011 1100

                Bits we want from 3rd byte:         1100 0000  (0xC0)
                Bits after shifting >> 6:           0000 0011

                Combining into new 6-bit value:     0011 1111
            */

            NewByte3 = ((InputByte2 & 0x0F) << 2) |
                       ((InputByte3 & 0xC0) >> 6);
        }
        else
            /* This is to handle the special case as described in RFC 4648 
               when we do not have an even multiple of 3 bytes, generate 
               the character '=' */
            NewByte3 = 255;


        if (InputByteCnt > 2) {

            /*  Finally, take the last 6 bits from the 3rd byte
                to make a new 6-bit value.  No need to shift the bits
                since they are already in the correct position.

                Bits we want from the 3rd byte:     0011 1111  (0x3F)
                New 6-bit value:                    0011 1111
            */

            NewByte4 = (InputByte3 & 0x3F);
        }
        else
            /* This is to handle the special case as described in RFC 4648
               when we do not have an even multiple of 3 bytes, generate
               the character '=' */
            NewByte4 = 255;


        /*  Find + print the printable ASCII characters.
            If the NewByte == 255, GetBase64Char returns
            the padding symbol '='.  
        */
        
        printf("%c", GetBase64Char(NewByte1));
        printf("%c", GetBase64Char(NewByte2));
        printf("%c", GetBase64Char(NewByte3));
        printf("%c", GetBase64Char(NewByte4));

        /* Loop around to the next batch of input data */
    }

    /* All done */
    return 0;
}


unsigned char GetBase64Char( unsigned char InputChar ) {

    /*  From RFC 4648, table of 6-bit input values 
        and which ASCII values to map to.    */

    /*  0-25 -> A-Z  */
    if ((InputChar >= 0) && (InputChar <= 25))
        return(InputChar + 65);

    /*  26-51 -> a-z  */
    if ((InputChar > 25) && (InputChar <= 51))
        return(InputChar + 71);

    /*  52-61 -> 0-9  */
    if ((InputChar > 51) && (InputChar <= 61))
        return(InputChar - 4);

    /*  62 -> "+"   */
    if (InputChar == 62)
        return(43);

    /*  63 = "/"  */
    if (InputChar == 63)
        return(47);

    /*  Special: End Padding = '='  */
    if (InputChar == 255)
        return(61);

}

