
/*

In the NT kernel, a Process can have multiple SIDs assigned to it.
A Process object in kernel memory has a field named "SidAndAttributes", 
which is an array of SID_AND_ATTRIBUTES structs.

Each SID_AND_ATTRIBUTES struct has 2 fields:  

1.  Pointer to a SID struct.
2.  ULONG for an attributes value.

The various NT security-related APIs which accept arrays of 
SID_AND_ATTRIBUTES structs will fail unless the SID structs pointed
to by the Sid pointers in the SID_AND_ATTRIBUTES structs are "local"
memory addresses.  The array items and the data items must be in a
continuous / contiguous block of memory.

Example of an array of three SID_AND_ATTRIBUTES and the SID structs
in memory.  The SID data is located immediately after the array.

 
SID_AND_ATTRIBUTES[3]:
     
Address         Sid Pointer Value      Attributes Value
---------------------------------------------------------
0000:0010       0000:0040               1
0000:0020       0000:0060               1
0000:0030       0000:0080               1

SIDs:
 
Address         Value
-----------------------------------
0000:0040       <SID struct data>
0000:0060       <SID struct data>
0000:0080       <SID struct data>


I made the following two functions to make a copy of a Process
object's array of SID_AND_ATTRIBUTES in the proper memory
layout.

*/



/*  This function makes a copy of a SID.  */

SID* DuplicateSid( SID* SrcSid ) 
{
    SID* DstSid = NULL;
   
    if ( !SrcSid ) return ( NULL );
   
    /*  Allocate buffer for the new SID from the paged pool.
        By default, ExAllocatePool2 returns zero-initialized 
        memory.  */

    DstSid =  ( SID* )  ExAllocatePool2(  POOL_FLAG_PAGED,
                                          SECURITY_MAX_SID_SIZE,
                                          'pool' );
   
    if ( !DstSid ) return ( NULL );
   
    /*  This is the fixed-size part of the SID */

    DstSid->Revision                      = SrcSid->Revision;
    DstSid->SubAuthorityCount             = SrcSid->SubAuthorityCount;
    DstSid->IdentifierAuthority.Value[0]  = SrcSid->IdentifierAuthority.Value[0];
    DstSid->IdentifierAuthority.Value[1]  = SrcSid->IdentifierAuthority.Value[1];
    DstSid->IdentifierAuthority.Value[2]  = SrcSid->IdentifierAuthority.Value[2];
    DstSid->IdentifierAuthority.Value[3]  = SrcSid->IdentifierAuthority.Value[3];
    DstSid->IdentifierAuthority.Value[4]  = SrcSid->IdentifierAuthority.Value[4];
    DstSid->IdentifierAuthority.Value[5]  = SrcSid->IdentifierAuthority.Value[5];
   
    /*  This is the variable-sized part of the SID */
   
    for ( UCHAR  Index = 0;
                 Index < ( SrcSid->SubAuthorityCount );
                 Index += 1 ) {

        DstSid->SubAuthority[Index] = SrcSid->SubAuthority[Index];
    }                
   
    return(DstSid);

}


/*  This function makes a copy of an array of SID_AND_ATTRIBUTES
    structs with the proper memory layout.   */

SID_AND_ATTRIBUTES* DuplicateSidAndAttrsArray(
                    SID_AND_ATTRIBUTES* SrcSidAttrs,
                    ULONG               SrcSidCount )
{
    BOOLEAN Status = FALSE;
 
    if (( !SrcSidAttrs ) || ( !SrcSidCount )) return ( NULL );
   
    /*  Determine how much memory we need for the output buffer.    */
    /*  Size alignment is necessary for the Sids because we         */
    /*  create copies of them using the SECURITY_MAX_SID_SIZE       */
    /*  constant in the DDK which is 4-byte aligned.                */
   
    SIZE_T  AlignedSidSize  =  ALIGN_UP( SECURITY_MAX_SID_SIZE, UINT64 );
    SIZE_T  SidsSize        =  ( SrcSidCount * AlignedSidSize );
    SIZE_T  SidAttrsSize    =  ( SrcSidCount * sizeof ( SID_AND_ATTRIBUTES ));
    UCHAR*  Buffer          =   NULL;
   
    /*  Allocate a buffer from the Paged Pool big enough for 
        both the array of SID_AND_ATTRIBUTES and the SIDs  */
   
    Buffer   =  ( UCHAR* )  ExAllocatePool2( POOL_FLAG_PAGED,
                                           ( SidAttrsSize + SidsSize ),
                                             'pool' );

    if ( !Buffer ) return ( NULL );
 
    /*  Variable for the SID_AND_ATTRIBUTES array part of 
        the output buffer  */

    SID_AND_ATTRIBUTES*  SidAttrs   =  NULL;
   
    /*  Variables to keep track of where things begin */

    UINT64  SidAttrsStart   =  ( UINT64 )  Buffer;
    UINT64  SidsStart       =  ( UINT64 )  Buffer + SidAttrsSize;
   
    /*  Assign array pointer to the beginning of the output buffer */
   
    SidAttrs    =   ( SID_AND_ATTRIBUTES* )  SidAttrsStart;

    /*  Now, iterate through the input SID_AND_ATTRIBUTES array
        and generate a copy of it.  
   
        The output buffer has two regions, one for the
        SID_AND_ATTRIBUTES array elements, and another for
        the SIDs that are pointed to by those array elements  
    */

    for ( ULONG     Index = 0;
                    Index < SrcSidCount;
                    Index += 1 ) {
       
        /*  First, make a copy of each SID from the original input data,
            which does a deep copy.  */
       
        SID* CopyOfSrcSid   = DuplicateSid( SrcSidAttrs[ Index ].Sid );
               
        /*  Determine the correct destination memory offsets in the
            output buffer for the current item.  */
   
        UINT64 SidAttrOffset    = ( UINT64 ) ( SidAttrs + Index );

        UINT64 SidOffset        = ( UINT64 ) ( SidsStart + 
                                             ( Index * AlignedSidSize ));
   
        /*  Set the current item Sid pointer value and Attributes value.  */
       
        ( SidAttrs + Index ) -> Sid         = SidOffset;

        ( SidAttrs + Index ) -> Attributes  = ( SrcSidAttrs + Index ) -> Attributes;

        /*  Copy the contents of the Sid into the correct offset location 
            in the output buffer.    */
       
        RtlMoveMemory(  ( UCHAR* )   SidOffset,
                        ( UCHAR* )   CopyOfSrcSid,
                                     AlignedSidSize);
       
        /*  Free the Sid that DuplicateSid allocated. */
       
        SaferFree( &CopyOfSrcSid, __func__, 0);
       
    }
       

    return ( SidAttrs );

}