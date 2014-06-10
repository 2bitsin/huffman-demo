#include <stdio.h>
#include "huffman.h"


int main (int c,char ** v) {
    FILE *i,*o ;
    
    if (c < 4) {
        printf ("%s e|d <ieinantis> <iseinantis>",v [0]) ;
        return 0 ;        
    }
    
    i = fopen (v [2],"rb") ;   
    o = fopen (v [3],"wb") ;   
    if (!i || !o) {
        printf ("negaliu atidaryti failo %s arba %s\n",v [2],v [3]) ;
        return -1 ;
    }    
    if (v [1][0] == 'e') {
        huff_encode (o,i) ;   
    }
    else if (v [1][0] == 'd') {        
        huff_decode (o,i) ;   
    }
    else {
        printf ("Nezinoma komanda '%s'",v [1]) ;
    }        
    return 0 ;
}

