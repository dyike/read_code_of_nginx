#include "ngx_config.h"
#include <stdio.h>
#include "ngx_conf_file.h"
#include "nginx.h"
#include "ngx_core.h"
#include "ngx_string.h"
#include "ngx_palloc.h"
#include "ngx_list.h"

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


void dump_list_part(ngx_list_t* list, ngx_list_part_t* part)
{
    int *ptr = (int*)(part->elts);
    int loop = 0;
    printf("  .part = 0x%x\n", &(list->part));
    printf("    .elts = 0x%x  ", part->elts);
    printf("(");
    for (; loop < list->nalloc - 1; loop++) {
        printf("%d, ", ptr[loop]);
    }
    printf("%d)\n", ptr[loop]);
    printf("    .nelts = %d\n", part->nelts);
    printf("    .next = 0x%x", part->next);
    if (part->next)
        printf(" -->\n");
    printf(" \n");
}

void dump_list(ngx_list_t* list)
{
    if (list) {
        printf("list = 0x%x\n", list);
        printf("  .last = 0x%x\n", list->last);
        printf("  .part = %d\n", &(list->part));
        printf("  .size = %d\n", list->size);
        printf("  .nalloc = %d\n", list->nalloc);
        printf("  .pool = 0x%x\n", list->pool);

        printf("elements: \n");
        ngx_list_part_t *part = &(list->part);
        while (part) {
            dump_list_part(list, part);
            part = part->next;
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
    printf("alloc an list from the pool:\n");
    printf("--------------------------------\n");
    ngx_list_t *list = ngx_list_create(pool, 5, sizeof(int));

    if(NULL == list) {
        return -1;
    }
    for (i = 0; i < 5; i++) {
        int *ptr = ngx_list_push(list);
        *ptr = 2*i;
    }

    dump_list(list);

    ngx_destroy_pool(pool);
    return 0;
}


