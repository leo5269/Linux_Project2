# Linux_Project2
Linux OS 課程的第二個project

leo_project2.c 實作 Question 2 的 system call 
// 宣告一個 static wait queue, 在 linux 是用雙向循環 linked list maintain的，有兩種資料結構，一個是 wait_queue_head 為這個 linked list 的 head；另一個是 wait_queue_entry 為這個 linked list 的所有 node(thread)
// 過程中宣告一個 wait_queue 的 spinlock 來控制每次 wait_queue 只能有一個 thread 的進出，用 atomic 變數強制實現 FIFO 的機制

enter_wait_queue(wait_queue_entry_t *ptr)
將 user process 送過來的每一條 thread 都加進 wait_queue 再設成 TASK_INTERRURPT 的 state,然後call schedule() 交出 CPU 的控制權，讓 user process 可以執行接下來的 thread, 執行 finish_wait() 將喚醒的 thread 移出 wait_queue.

clean_wait_queue(void)
呼叫 __wake_up() 確保將還在 TASK_INTERRUPT 的 thread 都喚醒，並移除 wait_queue 回到 ready queue 等待拿回 CPU 的控制權.

project2.c 為 Question 2 的 code
create 出一條一條的 thread 再送到 kernel 的 system call 中，觀察 wait queue 的 thread 進出為 FIFO 的情形，會了解到 process 的 TASK_struct() 的member, wait_queue 的資料結構，function, 觀念以及 schedule() 排班的觀念。  
