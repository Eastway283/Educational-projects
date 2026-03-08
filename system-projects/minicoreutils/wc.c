#include <stdio.h>
#include <ctype.h>

#define ON 1 
#define OFF 0

/* wc: подсчет символов, слов, строк */ 
int main(int argc, char **argv)
{
    FILE *file;
    unsigned lineno, wordno, charno;
    int c, LineCount, WordCount, CharCount;
    lineno = wordno = charno = 0;
    LineCount = WordCount = CharCount = OFF;

    while (--argc > 0 && (*++argv)[0] == '-')
        while (c = *++argv[0])
            switch (c) {
                case 'l':
                    LineCount = ON;
                    break;
                case 'w':
                    WordCount = ON;
                    break;
                case 'c':
                    CharCount = ON;
                    break;
                default:
                    fprintf(stderr, "wc: illegal option %c\n", c);
                    argc = 0;
                    break;
            }

    if (argc != 1) {
        fprintf(stderr, "Usage: wc [-l -w -c] filename");
        return 1;
    }

    file = fopen(*argv, "r");
    if (!file) {
        fprintf(stderr, "wc: can't open file %s\n", *argv);
        return 1;
    }

    int in_word = 0;
    while ((c = getc(file)) != EOF) {
        charno++;
        if (c == '\n')
            lineno++;
        if (isspace(c))
            in_word = 0;
        else {
            if (!in_word) {
                wordno++;
                in_word = 1;
            }
        }
    }
    fclose(file);

    if (!LineCount && !WordCount && !CharCount)
        LineCount = WordCount = CharCount = ON;

    if (LineCount)
        printf("lines: %u\n", lineno);
    if (WordCount)
        printf("words: %u\n", wordno);
    if (CharCount)
        printf("chars: %u\n", charno);

    return 0;
}
