#include "ngx_config.h"
#include <stdio.h>
#include "ngx_conf_file.h"
#include "nginx.h"
#include "ngx_core.h"
#include "ngx_string.h"
#include "ngx_palloc.h"
#include "ngx_array.h"

volatile ngx_cycle_t  *ngx_cycle;

void ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
            const char *fmt, ...)
{
}

void dump_pool(ngx_pool_t* p)
{
    while (p) {
        printf("pool = 0x%x\n", p);
        printf("p->d \n");
        printf("    .last = 0x%x\n", p->d.last);
        printf("    .end  = 0x%x\n", p->d.end);
        printf("    .next = 0x%x\n", p->d.next);
        printf("    .failed = 0x%x\n", p->d.failed);
        printf(" .max  = 0x%x\n", p->max);
        printf(" .current = 0x%x\n", p->current);
        printf(" .chain = 0x%x\n", p->chain);
        printf(" .large = 0x%x\n", p->large);
        printf(" .cleanup = 0x%x\n", p->cleanup);
        printf(" .log = 0x%x\n", p->log);
        printf("available pool memory = %d\n", p->d.end - p->d.last);
        p = p->d.next;
    }

}


void dump_array(ngx_array_t* a)
{
    if (a) {
        printf("array = 0x%x\n", a);
        printf("  .elts = 0x%x\n", a->elts);
        printf("  .nelts = %d\n", a->nelts);
        printf("  .size = %d\n", a->size);
        printf("  .nalloc = %d\n", a->nalloc);
        printf("  .pool = 0x%x\n", a->pool);

        printf("elements: ");
        int *ptr = (int*)(a->elts);
        for (; ptr < (int*)(a->elts + a->nalloc * a->size); )
        {
            printf("%d  ", *ptr++);
        }
        printf("\n");
    }
}

int main()
{
    ngx_pool_t *pool;
    int i;

    printf("--------------------------------\n");
    printf("create a new pool:\n");
    printf("--------------------------------\n");
    pool = ngx_create_pool(1024, NULL);
    dump_pool(pool);

    printf("--------------------------------\n");
    printf("alloc an array from the pool:\n");
    printf("--------------------------------\n");
    ngx_array_t *a = ngx_array_create(pool, 5, sizeof(int));

    for (i = 0; i < 5; i++) {
        int *ptr = ngx_array_push(a);
        *ptr = 2*i;
    }

    dump_array(a);

    ngx_array_destroy(a);
    ngx_destroy_pool(pool);
    return 0;
}

