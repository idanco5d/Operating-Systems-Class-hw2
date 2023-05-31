#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/module.h>

asmlinkage long sys_hello(void) {
 printk("Hello, World!\n");
 return 0;
}

asmlinkage long sys_set_weight(int weight) {
	if (weight < 0) {
		return -EINVAL;
	}	
	current->weight = weight;
	return 0;
}

asmlinkage long sys_get_weight(void) {
	return current->weight;
}

asmlinkage long sys_get_ancestor_sum(void) {
	long sum = 0;
	struct task_struct *current_task = current;
	while (current_task->parent && current_task->pid > 1) {
		sum += current_task->weight;
		current_task = current_task->parent;
	}
	if (current) {
		sum += current_task->weight;
	}
	return sum;
}

long pid_of_max_weight_descendant(long* weight, struct task_struct *current_task) {
    if (list_empty(&current_task->children)) {
        *weight = current_task->weight;
        return current_task->pid;
    }
    struct task_struct *current_child;
    struct list_head *children_head;
    long pid_of_max;
    long max_weight;
    if (current_task->pid == current->pid) {
        pid_of_max = 0;
        max_weight = 0;
    }
    else {
        pid_of_max = current_task->pid;
        max_weight = current_task->weight;
    }
    list_for_each(children_head,&current_task->children) {
        current_child = list_entry(children_head, struct task_struct, sibling);
        long max_weight_of_curr = 0;
        pid_t pid_max_of_curr = pid_of_max_weight_descendant(&max_weight_of_curr, current_child);
        if (max_weight_of_curr > max_weight) {
            pid_of_max = pid_max_of_curr;
            max_weight = max_weight_of_curr;
        }
        if (max_weight_of_curr == max_weight && pid_max_of_curr < pid_of_max) {
            pid_of_max = pid_max_of_curr;
        }
    }
    *weight = max_weight;
    return pid_of_max;
}

asmlinkage long sys_get_heaviest_descendant(void) {
	struct task_struct *current_task = current;
	if (!current_task || list_empty(&current_task->children)) {
		return -ECHILD;	
	}
	long weight_heaviest = 0;
	return pid_of_max_weight_descendant(&weight_heaviest, current_task);
}