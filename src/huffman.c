#include <stdio.h>

#define PROB                unsigned short
#define COUNT_CHARS         257
#define INTBITS             (sizeof (int) * 8)
#define USED_FREE           0
#define USED_AVAIL          1
#define USED_LOCKED         2

struct node {
    PROB prob ;
    struct node *left,*right ;
    char * code ;
    int size ;
    int used ;
} ;

static int count_prob (FILE * f,struct node * p,int m) {
    struct node *t ;
    int c ;
    
    while ((c = fgetc (f)) >= 0) {
        if (c >= m)
            return -1 ;
        if (++(p + c)->prob != (PROB)-1)
            continue ;
        for (t = p;t < p+m;++t) 
            t->prob >>= 1 ;                         
    }
    return 0 ;
}

static struct node * build_tree (struct node * p,int m) {
    unsigned i ;
    struct node *a,*b ;
    
    for (i = 0;i < m;++i) 
        p [i].used = USED_AVAIL ;
    for (i = m;i < m*2;++i)
        p [i].used = USED_FREE ;         
                 
    for (;;) {        
        
        a = NULL ;
        b = NULL ;
        for (i = 0;i < m*2;++i) {
            if (p [i].used != USED_AVAIL) 
               continue ;            
            if (!a || p [i].prob < a->prob)
                a = &p [i] ;
        }
        if (!a) 
            return NULL ;
        
        a->used = USED_LOCKED ;
            
        for (i = 0;i < m*2;++i) {
            if (p [i].used != USED_AVAIL) 
               continue ;
            if (!b || p [i].prob < b->prob)
                b = &p [i] ;
        }
        
        if (!b) 
            return a ;
                                 
        b->used = USED_LOCKED ;                                     
        
        for (i = m;p [i].used != USED_FREE && i < m*2;++i) ;                
        p [i].prob = 
            a->prob + 
            b->prob ;                         
        p [i].left = a ; 
        p [i].right = b ;
        p [i].used = USED_AVAIL ;                    
    }                
    return NULL ;    
}

static char * memdup (char * from,size_t size) {
    return (char *)memcpy (malloc (size),from,size) ;    
}


static void build_codes (struct node * curr,int bits,char * code) {
    unsigned i ;
    if (curr->left || curr->right) {                    
        if (curr->left) {
            code [bits] = 1 ;
            build_codes (curr->left,bits+1,code) ;        
        }
        if (curr->right) {
            code [bits] = 0 ;
            build_codes (curr->right,bits+1,code) ;
        }
        return ;
    }    
    curr->code = memdup (code,bits) ;
    curr->size = bits ;        
}

static void kill_tree (struct node * curr) {
    if (curr) {
        kill_tree (curr->left) ;
        kill_tree (curr->right) ;
        free (curr->code) ;        
    }
}

static void encode (FILE * o,FILE * f,struct node * codes,int m) {
    int c,i,j ;
    unsigned bits ;
    char * code ;
    int size ;
        
    j = 0 ;
    bits = 0 ; 
    
    for (i = 0;i < m-1;++i)
        fwrite (&codes [i].prob,1,sizeof (codes [i].prob),o); 
    
    do {
        c = fgetc (f) ;
        if (c < 0 || c > m-1)
            c = m-1 ;     
        size = codes [c].size ;
        code = codes [c].code ;        
        for (i = 0;i < size;++i) {            
            bits |= code [i] << j ;
            ++j ;            
            if (j >= INTBITS) {
                fwrite (&bits,1,sizeof (bits),o) ;
                bits = 0 ;
                j = 0 ;
            }                                        
        }         
    }
    while (c != m-1) ;
    fwrite (&bits,1,sizeof (bits),o) ;
}

static void decode (FILE * o,FILE * f,struct node * root,struct node * base,int m) {
    unsigned bits ;
    int i,j = INTBITS;        
    struct node * temp ;
    unsigned flag = 0 ;
        
    for (;;) {
        temp = root ;
        while (temp->left || temp->right) {
            if (j >= INTBITS) {                                
                if (!fread (&bits,1,sizeof (bits),f))
                    //return 
                ;
                j = 0 ;
            }    
            if (bits & (1 << j)) 
                temp = temp->left ;
            else 
                temp = temp->right ;
            ++j ;            

        } 
        i = temp - base ;
        if (i < m-1)
            fwrite (&i,1,1,o) ;
        else
            break ;              
    }    
}

static int read_prob (FILE * f,struct node * p,int m) {
    int i ;
    for (i = 0;i < m-1;++i) 
        fread (&p [i].prob,1,sizeof (p [i].prob),f) ;
    return 0 ;    
}

int huff_encode (FILE * o,FILE * f) {
    struct node tree [COUNT_CHARS*2] ;
    struct node * root ;
    char code_buff [COUNT_CHARS*2] ;           
    size_t pos = ftell (f) ;
        
    if (count_prob (f,tree,COUNT_CHARS) < 0)
        return -1 ;
    root = build_tree (tree,COUNT_CHARS) ;
    if (!root)
        return -1 ;
    build_codes (root,0,code_buff) ;
    fseek (f,pos,SEEK_SET) ;
    encode (o,f,tree,COUNT_CHARS) ;
    kill_tree (tree) ;                   
    return 0 ;
}

int huff_decode (FILE * o,FILE * f) {
    struct node tree [COUNT_CHARS*2] ;
    struct node * root ;
        
    if (read_prob (f,tree,COUNT_CHARS) < 0)
        return -1 ;
    root = build_tree (tree,COUNT_CHARS) ;
    if (!root)
        return -1 ;
    decode (o,f,root,tree,COUNT_CHARS) ;                    
    kill_tree (tree) ;                   
    return 0 ;
}
