#include <string.h>
#include <sys/types.h>
#include <system/mm.h>
#include <fs/fs.h>
#include <system/init.h>

static struct dir_entry *devfs_mount(struct file_system_type *fs_type, int dev_num);
static int f_op_write(struct file_descriptor *fd, char *buf, int nbyte);
static int f_op_read(struct file_descriptor *fd, char *buf, int nbyte);

static int rootfs_lookup(struct dir_entry *dir, const char *name, int len, struct dir_entry **res_dir);

struct file_system_type devfs_fs_type = {
    name: "devfs",
    mount: devfs_mount,
    next: NULL
};
struct file_operation devfs_f_op={
    NULL,
    f_op_read,
    f_op_write,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
struct inode_operation devfs_inode_op = {
    NULL,
    devfs_lookup,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

static struct dir_entry *devfs_mount(struct file_system_type *fs_type, int dev_num)
{
    struct super_block *sb = get_block(dev_num);
    fs_type->sb_table=sb;

    sb->fs_type = fs_type;

    struct inode *new_inode=get_inode();
    new_inode->sb=sb;
    new_inode->dev_num=sb->dev_num;
    new_inode->f_op=&devfs_f_op;
    new_inode->inode_op = &devfs_inode_op;
    new_inode->mode = FILE_MODE_DIR;

    struct dir_entry *new_dir=get_dir();
    new_dir->name[0]='/';
    new_dir->dev_num=sb->dev_num;
    new_dir->inode=new_inode;
    new_dir->parent=new_dir;
    new_dir->sb=sb;

    sb->root_dir=new_dir;
    sb->root_inode=new_inode;

    return new_dir;
}

void init_devfs()
{
    register_filesystem(&devfs_fs_type);
    sys_mkdir("/dev", 1);
}

static int f_op_write(struct file_descriptor *fd, char *buf, int nbyte)
{
    return nbyte;
}

static int f_op_read(struct file_descriptor *fd, char *buf, int nbyte)
{
    return nbyte;
}

int devfs_lookup(struct dir_entry *dir, const char *name, int len, struct dir_entry **res_dir)
{
    return 0;
}

core_initcall(init_devfs);
