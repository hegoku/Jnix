#include <system/tty.h>
#include <system/page.h>
#include <arch/i386/io.h>


#define MAJOR_NR 4

TTY tty_table[TTY_NUM];
TTY *current_tty;

static int do_request();

void init_tty()
{
    for (int i=0; i < TTY_NUM;i++) {
        tty_table[i]=tty_create(i);
    }
    current_tty = &tty_table[0];
}

TTY tty_create(unsigned char id)
{
    TTY tty;
    tty.id = id;
    tty.inbuf_count = 0;
    tty.inbuf_head = tty.inbuf_tail = tty.in_buff;

    CONSOLE console;
    int v_mem_size = V_MEM_SIZE >> 1;

    int con_v_mem_size = v_mem_size / TTY_NUM;
    console.original_addr = id*con_v_mem_size;
    console.v_mem_limit = con_v_mem_size;
    console.current_start_addr = console.original_addr;
    console.cursor = console.current_start_addr;
    tty.console = console;

    return tty;
}

unsigned int tty_write(int mi_dev, char* buf, int len)
{
    TTY *tty = &tty_table[mi_dev];
    char *p = buf;
    int i = len;
    // return len;
    // DispStr(buf);
    // if ((disp_pos/2) >= 80*25)
    // {
    //     disp_pos = 0;
    // }
    // return i;
    while (i)
    {
        console_out_char(&(tty->console), *p++);
        i--;
    }
    return len;
}

static void console_out_char(CONSOLE* console, char ch)
{
    unsigned char *p_vmem = (unsigned char *)(__va(V_MEM_BASE) + console->cursor * 2);
    switch (ch) {
        case '\n':
            if (console->cursor < console->original_addr +
                console->v_mem_limit - SCREEN_WIDTH) {
                console->cursor = console->original_addr + SCREEN_WIDTH * 
                    ((console->cursor - console->original_addr) /
                    SCREEN_WIDTH + 1);
            }
            break;
        case '\b':
            if (console->cursor > console->original_addr) {
                console->cursor--;
                *(p_vmem-2) = ' ';
                *(p_vmem-1) = DEFAULT_CHAR_COLOR;
            }
            break;
        default:
            if (console->cursor < console->original_addr + console->v_mem_limit - 1) {
                *p_vmem++ = ch;
                *p_vmem++ = DEFAULT_CHAR_COLOR;
                console->cursor++;
            }
            break;
    }

    while (console->cursor >= console->current_start_addr + SCREEN_SIZE)
    {
        // console->cursor = 0;
        scroll_screen(console, SCR_DN);
    }

    flush(console);
}

static void flush(CONSOLE *console)
{
    console_set_cursor(console->cursor);
    set_console_start_addr(console->current_start_addr);
}

static void console_set_cursor(unsigned int position)
{
    asm("cli");
    outb(CURSOR_H, CRTC_ADDR_REG);
    outb((position >> 8) & 0xFF, CRTC_DATA_REG);
	outb(CURSOR_L, CRTC_ADDR_REG);
	outb(position & 0xFF, CRTC_DATA_REG);
    asm("sti");
}

static void set_console_start_addr(unsigned int addr)
{
    asm("cli");
    outb(START_ADDR_H, CRTC_ADDR_REG);
    outb((addr>>8)&0xff, CRTC_DATA_REG);
    outb(START_ADDR_L, CRTC_ADDR_REG);
    outb(addr&0xff, CRTC_DATA_REG);
    asm("sti");
}

void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
        if (p_con->current_start_addr > p_con->original_addr)
        {
            p_con->current_start_addr -= SCREEN_WIDTH;
        }
    }else if (direction == SCR_DN) {
        if (p_con->current_start_addr + SCREEN_SIZE < p_con->original_addr + p_con->v_mem_limit)
        {
            p_con->current_start_addr += SCREEN_WIDTH;
        }
	}

	set_console_start_addr(p_con->current_start_addr);
	console_set_cursor(p_con->cursor);
}

void clear_screen(TTY *tty)
{
    unsigned char *p_vmem;
    for (int i = tty->console.original_addr; i < tty->console.v_mem_limit/2;i++) {
        p_vmem = (unsigned char *)(__va(V_MEM_BASE) + i * 2);
        *p_vmem++ = '\0';
        *p_vmem++ = DEFAULT_CHAR_COLOR;
    }

    tty->console.current_start_addr = tty->console.original_addr;
    tty->console.cursor = tty->console.current_start_addr;
    set_console_start_addr(tty->console.current_start_addr);
    console_set_cursor(tty->console.cursor);    
}