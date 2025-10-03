#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/delay.h>

static DECLARE_WAIT_QUEUE_HEAD(my_wait_queue); // Create a static empty wait queue with a head
static DEFINE_SPINLOCK(my_lock); // Create a spinlock
static atomic_t wake_up_in_progress = ATOMIC_INIT(0); // Create an atomic variable to track the wake-up progress

static int enter_wait_queue(wait_queue_entry_t *thr) // Function to handle each thread entering the wait queue
{
    prepare_to_wait_exclusive(&my_wait_queue, thr, TASK_INTERRUPTIBLE); // Add the process to the wait queue and set its state to TASK_INTERRUPTIBLE
    pr_info("Thread (PID: %d, comm: %s) added to wait queue and set TASK_INTERRUPTIBLE\n", current->pid, current->comm);
    schedule(); // Yield the CPU to allow other threads to execute
    finish_wait(&my_wait_queue, thr); // Remove the thread from the wait queue after it is woken up
    kfree(thr); // Free the allocated resources for the thread
    pr_info("Thread (PID: %d, comm: %s) woke up from wait queue\n", current->pid, current->comm);

    atomic_set(&wake_up_in_progress, 0); // Mark the wake-up process as complete, allowing the next thread to be woken up

    return 1;
}

static int clean_wait_queue(void) // Function to clean up the wait queue
{
    while (waitqueue_active(&my_wait_queue)) { // Check if the wait queue is empty
        spin_lock(&my_lock); // Acquire the spinlock

        if (atomic_read(&wake_up_in_progress) == 0) { // Check if only one thread is being woken up
            atomic_set(&wake_up_in_progress, 1); // Mark the wake-up process as in progress
            __wake_up(&my_wait_queue, TASK_INTERRUPTIBLE, 1, NULL); // Wake up one thread in TASK_INTERRUPTIBLE state
            pr_info("Waking up one thread from wait queue\n");
        }

        spin_unlock(&my_lock); // Release the spinlock

        while (atomic_read(&wake_up_in_progress)) { // Release the CPU to allow the awakened thread to execute
            msleep(10);
        }
    }
    return 1;
}

SYSCALL_DEFINE1(call_my_wait_queue, int, id) // Define a system call with one parameter `id`
{
    wait_queue_entry_t *new_thread;
    switch (id) {
    case 1: // Add a thread to the wait queue
        new_thread = kmalloc(sizeof(*new_thread), GFP_KERNEL);
        if (!new_thread)
            return -ENOMEM; // Return an error if memory allocation fails
        init_wait_entry(new_thread, current); // Initialize the wait queue entry for the current process
        return enter_wait_queue(new_thread); // Add the thread to the wait queue
    case 2: // Clean up the wait queue
        return clean_wait_queue();
    default: // Handle invalid `id` values
        return 0;
    }
}
