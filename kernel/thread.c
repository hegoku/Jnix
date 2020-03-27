#include <system/mm.h>
#include <system/page.h>
#include <system/thread.h>
#include <string.h>

struct thread *thread_table=0;
struct thread *current_thread=0;
unsigned int pid=0;

int thread_int()
{
    current_thread=thread_table;
}

int thread_create(unsigned char *name, void *handle)
{
    struct thread *th=kzmalloc(sizeof(struct thread));
    th->pid=pid;
    th->parent_pid=-1;
    pid++;

    unsigned char *np=name;
    unsigned char th_np=name;
    while(*np!='\0') {
        th_np=np;
        th_np++;
        np++;
    }

    th->page_dir=(struct PageDir *)__va(PAGE_DIR_BASE);

    if (thread_table==0) {
        thread_table=th;
        th->next=th;
        th->prev=th;
    } else {
        th->next=thread_table;
        th->prev=thread_table->prev;
        thread_table->prev->next=th;
        thread_table->prev=th;
    }
    current_thread=th;
}
