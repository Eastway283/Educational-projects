#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

/* Этот код содержит решения упражнений 8.6 - 8.8 из книги K&R 2-е издание */
/* Написано на linux. переносимость не гарантируется                       */

typedef long Align; /* для выравнивания по границе long */

union header {             /* заголовок блока */
    struct {
        union header *ptr; /* следующий блок, если есть */
        unsigned size;     /* размер этого блока */
    } s;
    Align x;               /* принудительное выравнивание блоков */
};

typedef union header Header;

static  Header base; /* пустой список для начала */
static Header *freep = NULL;    /* начало списка */

#define NALLOC 1024 /* минимально запрашиваемое количество блоков */

/* morecore: запрос дополнительной памяти у системы */
static Header *morecore(unsigned nu)
{
    char *cp;
    void my_free(void *);
    Header *up;

    if (nu < NALLOC)
        nu = NALLOC;
    cp = sbrk(nu * sizeof(Header));
    if (cp == (char *) -1) /* места в памяти нет */
        return NULL;
    up = (Header *) cp;
    up->s.size = nu;
    my_free((void *) (up + 1));
    return freep;
}

/* my_malloc: функция распределения памяти */
void *my_malloc(unsigned nbytes)
{
    if (nbytes == 0)
        return NULL;
    Header *p, *prevp;
    unsigned nunits;

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    if ((prevp = freep) == NULL) { /* списка ещё нет */
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) { /* достаточный размер */
            if (p->s.size == nunits) /* в точности */ 
                prevp->s.ptr = p->s.ptr;
            else {              /* "отрезаем" хвост */
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            return (void *) (p + 1);
        }
        if (p == freep) /* ссылается на сам список */
            if ((p = morecore(nunits)) == NULL)
                return NULL; /* не осталось памяти */
    }
}

/* my_calloc: функция распределения и инициализации памяти */
void *my_calloc(unsigned n, unsigned size)
{
    if (n != 0 && size != 0 && n > SIZE_MAX / size)
        return NULL;
    unsigned total = n * size;
    void *value = my_malloc(total);
    if (value != 0)
        memset(value, 0, total);
    return value;
}


/* my_free: помещение блока ap в список свободных блоков */
void my_free(void *ap)
{
    if (ap == NULL)
        return;
    Header *bp, *p;

    bp = (Header *)ap - 1; /* указатель на заголовок */
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break; /* освобождаемый блок в начале или в конце */

    if (bp + bp->s.size == p->s.ptr) { /* к верхнему соседу */
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else
        bp->s.ptr = p->s.ptr;
    if (p + p->s.size == bp) { /* к нижнему соседу */
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else 
        p->s.ptr = bp;

    freep = p;
}

/* bfree: освобождение блока ap из n символов */
unsigned bfree(void *p, unsigned n)
{
    Header *hp;

    if (n < sizeof(Header)) /* слишком мало, для использования */
        return 0;
    hp = (Header *) p;
    hp->s.size = n / sizeof(Header);
    my_free((void *) (hp + 1));
    return hp->s.size;
}

int main(int argc, char *argv[])
{


    return 0;
}
