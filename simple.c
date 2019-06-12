#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_LICENSE("Dual BSD/GPL");


int p=0;

//function to find the oldest children of a given task, recursively
static void findOldestChild(int level, struct task_struct *task) {
  struct list_head *list;
  struct task_struct *task_child;
  struct task_struct *oldest = NULL;
  struct timespec rtime;
  list_for_each(list, &task->children){
    //get a list of children
    task_child = list_entry(list, struct task_struct, sibling);
    //finding the oldest child
    if (oldest == NULL || task_child->start_time > rtime.tv_nsec) {
      oldest = task_child;
      rtime.tv_nsec=task_child->start_time;
    }
    if (level < 100) {
      findOldestChild(++level, task_child); //recursive call to find oldest children of child processes
    } else if (level >= 100) {
      // printk(KERN_INFO "Level 100 reached :(\n");
    }
  }
  if (oldest != NULL) {
    printk(KERN_INFO "Oldest child of %s[%d] is processID: %s[%d]\n",task->comm, task->pid, oldest->comm, oldest->pid);
  }

}

// void findOldestChild(int level, struct task_struct *task)
// {
//   struct task_struct *child;
//   struct task_struct *oldestchild;
//   struct list_head *list;
//   struct timespec rtime;
//   rtime.tv_nsec=0;
//   // printk("name: %s, pid: [%d], state: %li time: %lldns\n", task->comm, task->pid, task->state, task->start_time);
//   list_for_each(list, &task->children) {
//     child = list_entry(list, struct task_struct, sibling);
//     // printk("name: %s, pid: [%d], time: %lldns\n", child->comm, child->pid ,child->start_time);
//     if(child->start_time > rtime.tv_nsec){
//       oldestchild=child;
//       rtime.tv_nsec=child->start_time;
//     }
//
//   }
//   printk("OLDEST name: %s, pid: [%d] time: %lldns\n", oldestchild->comm, oldestchild->pid,oldestchild->start_time);
//   //find_oldest(child);
// }

static int simple_init(void)
{
  struct task_struct* task;
  task=pid_task(find_vpid(p), PIDTYPE_PID);
  findOldestChild(0, task);
  return 0;
}

static void simple_cleanup(void)
{
  printk(KERN_WARNING "bye ...\n");
}

module_init(simple_init);
module_exit(simple_cleanup);
module_param(p, int, 0);
