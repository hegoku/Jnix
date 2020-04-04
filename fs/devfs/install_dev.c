#include <fs/fs.h>

struct dev char_dev_table;
struct dev block_dev_table;

int install_dev(int number, const char *name, int type, struct file_operation *fop)
{
    struct dev dev={
        name:name,
        type:type,
        f_op:fop
    };
    if (type==0) {
        char_dev_table[number]=dev;
    } else {
        block_dev_table[number]=dev;
    }
    return 0;
}