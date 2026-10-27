// Name : Emma-leila
// Last name : Fazilleau
// student ID : 260ADB193
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: <every AI tool, website and book you used; details in comments where you used them> 

// ressource 1 : youtube video on how to open et read files => https://www.youtube.com/watch?v=eIlcl-mt3tg 
// ressource 2 : youtube video who explains buffers => https://www.youtube.com/watch?v=hwx3C18eQwc 
// ressource 3 : youtube video on how to write a file => https://www.youtube.com/watch?v=UqB4EgUxapM&t=6s
// ressource 4 : chat gpt => https://chatgpt.com/share/6adf760f-df40-83ed-a00b-745b03e63ef9
// I asked chat gpt to give me a list of steps because I didn't know where to start to calculate mathematical expressions
// ressource 5 : youtube video on how to handle errors => https://www.youtube.com/watch?v=OOuZLI5ingc
// ressource 6 : chat gpt => https://chatgpt.com/share/6ae07b62-8aa8-83ed-a2f4-e3b1cc1ee7ef
// I asked chat gpt to tell me what to include and define so the folder that I create works on all PC types
// ressource 7 : youtube video about pointers => https://www.youtube.com/watch?v=2ybLD6_2gKM
// ressource 8 : youtube video on recursive descent parsers => https://www.youtube.com/watch?v=jCT6T7HxZJk&t=377s
// ressource 9 : https://www.w3schools.com/c/ => I used it to understand malloc and pointers

// I tested each step but didn't commit the changes everytime




#include <stdio.h>    // used to print something in the terminal and to read/write files
#include <stdlib.h>    // used for malloc : to reserve memory and free to release it => ressource 9
#include <string.h>   // for string characters
#include <sys/stat.h>   // mkdir : create a folder

// ressouce 6
#ifdef _WIN32      // create a folder on windows/linux/mac               
#include <direct.h>                 
#define MKDIR(path) _mkdir(path)    
#else                               
#define MKDIR(path) mkdir(path, 0777) 
#endif                              

#define FIRSTNAME "emma"      
#define LASTNAME  "fazilleau"   
#define STUDENTID "260ADB193"   

// fonction : we implement this fonction to read a file
// the whole file in one buffer because it is faster to read once instead of one call per character (if the file is big it's better)
// ressource 1
// ressource 7

static char *lire_fichier(const char *path, size_t *len) {  // return the len
    FILE *f = fopen(path, "rb");     // opens the file - 'rb' = binary
    if (!f) return NULL;           // if we can't open the file the code return NULL
                              // the size must be known before reading so we allocate the buffer once and prevent copying
    fseek(f, 0, SEEK_END);   // read the file to determine the size of it
    long size = ftell(f);               
    fseek(f, 0, SEEK_SET);     // reads the file
    if (size < 0) {     // the "if" checks if the size is valid, if it's not the file is closed and the code return NULL
        fclose(f);                       
        return NULL;                     
    }
// ressource 9
    char *buf = malloc((size_t)size + 1);  // allocates the memory => size of the file and +1 for '\0'
    if (!buf) {    // if we couldn't allocate the memory we close the file and return NULL
        fclose(f);      // if mallloc fails we close the file to prevent a crash                
        return NULL;                    
    }
// ressource 2

    *len = fread(buf, 1, (size_t)size, f);   // *len is the number of the character that we read
    buf[*len] = '\0';      // we put '\0' at the end so that we never read outside buf
    fclose(f);       
    return buf;      
}

// fonction - determine if a character is a space or tabulation
// only spaces and tabs are authorised between tokens

static int is_blank(char c) {       
    return c == ' ' || c == '\t';    //  || means or
}

static int is_digit(char c) {  //says if a character is a digit
    return c >= '0' && c <= '9';     // the character must be between '0' and '9'
}

// gives the position of the pointer
static long position(const char *debut, const char *p) {  // => ressource 7
    return (long)(p - debut) + 1;    // p - debut -> number of characters before p and +1 because position starts at 1
}

// fonction - skips spaces or tabulation

static void skip_blanks(const char **p, const char *end) {      // p is the pointer => ressource 7
    while (*p < end && is_blank(**p))   // as long as we're not at the end and the charactre is not a space or tab we increment
        (*p)++;                         
}

// fonction - reads a int number

static long long lire_nombre(const char **p, const char *end) {  // reads the number beginning with *p position
    long long valeur = 0;        //  we initialize with 0
    while (*p < end && is_digit(**p)) {    // the loop continues as long as the text is not finished and the character is a digit
        valeur = valeur * 10 + (**p - '0');   
        (*p)++;                     
    }
    return valeur;   // return the read number
}

// here we define the level of priority of each operator (like said in the subject)
// expr    := term { ('+' | '-') term }
// term    := primary { ('*' | '/') primary }
// primary := NUMBER | '(' expr ')'

typedef struct {   
    const char *debut; // start of the file 
    const char *p; // where p is reading 
    const char *end;  // end of the current line
    long pos_erreur;   // position of the error
} Parseur;

static int erreur(Parseur *ps, const char *ou) {  // saves the place of the error
    ps->pos_erreur = position(ps->debut, ou);  // we convert the pointer into a position in the file
    return 0;
}

// ressource 8

static int expr(Parseur *ps, long long *res);  // primaire calls expr, expr written below


static int primaire(Parseur *ps, long long *res) {    // reads one number or the expression between () 
    skip_blanks(&ps->p, ps->end);    // ignores the spaces before the number
    if (ps->p >= ps->end)     // the line is finished  we need a number
        return erreur(ps, ps->end);    //erreur for missing operand  => ressource 5

    if (*ps->p == '(') { 
        ps->p++;    // we skip the '('
        if (!expr(ps, res)) return 0;    // inside the () we apply the same rule for priority 
        skip_blanks(&ps->p, ps->end);  
        if (ps->p >= ps->end || *ps->p != ')')   
            return erreur(ps, ps->p);    // there is an error if a ) is missing   => ressource 5
        ps->p++;    // we skip the ) so the callers continue after it
        return 1;
    }
    
    if (!is_digit(*ps->p))  // something else instead of number
        return erreur(ps, ps->p);    // invalid character => ressource 5
    *res = lire_nombre(&ps->p, ps->end);   // reads the digits and moves ps->p after them
    return 1; 
}
// ressource 8
static int terme(Parseur *ps, long long *res) {   // handles the multiplication and division cause priority
    if (!primaire(ps, res)) return 0;     // if there is an error we stop
    for (;;) { 
        skip_blanks(&ps->p, ps->end); 
        if (ps->p >= ps->end) break;    // we stop at the end of the line
        char operator = *ps->p;    // analysesthe next character
        if (operator != '*' && operator != '/')  // if it's not * or / we break
            break; 
        ps->p++;      // we skip the operator
        skip_blanks(&ps->p, ps->end);  // on ignore les espaces
        const char *debut_droite = ps->p;  
        long long droite = 0;   // contains the right operand
        if (!primaire(ps, &droite)) return 0;    // if there is an error we stop  => ressource 5
        if (operator == '*') {    // multiplication
            *res = *res * droite;   // we update the result 
        } else {    // pour la division
            if (droite == 0)   // if we divide by 0 there is an error
                return erreur(ps, debut_droite); 
            *res = *res / droite;  
        }
    }
    return 1;    
}
// ressource 8

static int expr(Parseur *ps, long long *res) {   // it handles the addition and soustraction, same principle but with + -
    if (!terme(ps, res)) return 0; 
    for (;;) {  
        skip_blanks(&ps->p, ps->end);  // ignores the spaces
        if (ps->p >= ps->end) break; 
        char operator = *ps->p; 
        if (operator != '+' && operator != '-')   // if it's not plus or minus we stop => ressource 5
            break; 
        ps->p++;  
        long long droite = 0; 
        if (!terme(ps, &droite)) return 0;  // error = stop  => ressource 5
        if (operator == '+') 
            *res = *res + droite;
        else 
            *res = *res - droite;
    }
    return 1; 
}



// fonction : most important fonction to calculate a lign
// pos_erreur = gives the position of the error
// we return 1 if its valid and 0 if not

static int eval_line(const char *debut, const char *p, const char *end,
                     long long *resultat, long *pos_erreur) {   // p => beginning of the lign
    skip_blanks(&p, end);         // skips the blank in the begininng of the line
  
    Parseur ps = { debut, p, end, 0 }; 
    long long total = 0;  // contains the result of the line
    if (!expr(&ps, &total)) {   // we start with + - lowest priority
        *pos_erreur = ps.pos_erreur;  // if there is an error we give its position
        return 0;
    }
    skip_blanks(&ps.p, end);  // ignores spaces
    if (ps.p < end) {  // there is a valid expression but something is left (like character or 2 numbers next to each other)
        *pos_erreur = position(debut, ps.p);  // the parser couldn't use the element
        return 0; 
    }

    *resultat = total;    // we give the total
    return 1;
}

// fonction : analyse if the line is empty
// if there is a blank line there is no output

static int is_blank_line(const char *p, const char *end) {  
    skip_blanks(&p, end);  // we skips all the spaces
    return p >= end;    
}


// main fonction

int main(int argc, char **argv) {     // argc -> number of arguments, argv -> list of arguments
    const char *outdir = NULL;    // output folder
    const char *input = NULL;  // input file

    for (int i = 1; i < argc; i++) {    // we loop on the arguments 
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {     // -o needs a value after, without i+1 we would read outside argv
            i++;             
            outdir = argv[i];  // name of the output folder
        } else {                        
            input = argv[i]; // name of the input file
        }
    }

    if (!outdir || !input) {       // if we didn't receive the information we print a help message => ressource 5
        fprintf(stderr, "usage: calc -o OUTDIR input.txt\n"); 
        return 1; 
    }

    size_t len = 0;    // this variable contains the number of characters of the file
    char *buf = lire_fichier(input, &len);   // read the input file
    if (!buf) {       // if we can't find the file we print an error => ressource 5
        fprintf(stderr, "calc: cannot read '%s'\n", input); 
        return 1; 
    }

    MKDIR(outdir); // we create the output folder

    const char *name = input;   // we look for the name of the file
    for (const char *c = input; *c; c++) {    // we loop on each character
        if (*c == '/' || *c == '\\')   // if the separator is one of these '/' , '\\' the name start after that
            name = c + 1; 
    }                                   

    char base[256];   // table to stock the name
    snprintf(base, sizeof base, "%s", name);  // we copy the name in the base
    char *dot = strrchr(base, '.');  // look for the last point
    if (dot)   // if we find the point we stop
        *dot = '\0'; 

    char outpath[1024]; // table for the complete direction of the output file, it needs to be big for the folder and name
    snprintf(outpath, sizeof outpath, "%s/%s_%s_%s_%s.txt",  // we assemble the parts 
             outdir, base, FIRSTNAME, LASTNAME, STUDENTID);    // 5 infos that we have to insert

// ressource 3

    FILE *out = fopen(outpath, "w");  // open or create the output file
    if (!out) {    // if there is an error we print an error message => ressource 5
        fprintf(stderr, "calc: cant write '%s'\n", outpath); 
        free(buf); // we free the memory
        return 1; 
    }

    const char *p = buf; // p -> begining of the line
    const char *buf_end = buf + len;  // last character of the file

    while (p < buf_end) {  // as long as we have characters
        const char *fin_ligne = memchr(p, '\n', (size_t)(buf_end - p));  // we look for '\n'
        if (!fin_ligne)  // if we can't find
            fin_ligne = buf_end;  // the line goes until the end of the file

        if (!is_blank_line(p, fin_ligne)) {   // if the line isn't empty we calculate the result
            long long result = 0;     // will contain the result of the line
            long pos_erreur = 0;      // will contain the position of the error if there is one

            if (!eval_line(buf, p, fin_ligne, &result, &pos_erreur)) {   // if eval_line returns 0 there is an error
                FILE *reouvert = freopen(outpath, "w", out);   // we reopen the output file in "w" mode : this erases the results already written
                if (!reouvert) {              // if we can't rewrite the file we print an error message => ressource 5
                    fprintf(stderr, "calc: cannot write '%s'\n", outpath);
                    free(buf);
                    return 1;
                }
                out = reouvert;  // we continue with the reopened file
                fprintf(out, "ERROR:%ld\n", pos_erreur); // the file contains the error
                break;  // stop
            }
            fprintf(out, "%lld\n", result); // we print the result 
        }

        p = fin_ligne + 1;  // we move on to the next line
    }

    fclose(out);  // we close the output file and we free the memory
    free(buf);  // ressource 9
    return 0; // always return 0
}

