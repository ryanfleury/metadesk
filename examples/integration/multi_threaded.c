/* 
** Example: multi threaded parse
**
** TODO
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

#include <Windows.h>

//~ multi-threaded parse setup ////////////////////////////////////////////////

// for this intrinsic we're assume pre-increment behavior

#if MD_OS_WINDOWS
# define atomic_inc_then_eval_u64(p) InterlockedIncrement64((LONG64*)p)
#else
# error Not implemented for this compiler
#endif

typedef struct TaskData
{
    MD_u64 task_max;
    char **tasks;
    
    volatile MD_u64 task_counter;
    volatile MD_u64 thread_counter;
} TaskData;

typedef struct ThreadData
{
    TaskData *task;
    MD_Arena *arena;
    MD_Node *list;
    MD_MessageList errors;
} ThreadData;

void
parse_worker_loop(ThreadData *thread_data)
{
    TaskData *task = thread_data->task;
    for (;;)
    {
        MD_u64 task_index = atomic_inc_then_eval_u64(&task->task_counter) - 1;
        if (task_index >= task->task_max)
        {
            break;
        }
        MD_String8 file_name = MD_S8CString(task->tasks[task_index]);
        MD_ParseResult parse = MD_ParseWholeFile(thread_data->arena, file_name);
        MD_MessageListConcat(&thread_data->errors, &parse.errors);
        MD_PushNewReference(thread_data->arena, thread_data->list, parse.node);
    }
    
    atomic_inc_then_eval_u64(&task->thread_counter);
}

#if MD_OS_WINDOWS
DWORD
parse_worker_win32(LPVOID parameter)
{
    parse_worker_loop((ThreadData*)parameter);
    return(0);
}
#else
# error Not implemented for this OS
#endif


//~ main //////////////////////////////////////////////////////////////////////

int
main(int argc, char **argv)
{
#if 1
    char *argv_dummy[8] = {
        0,
        "W:\\metadesk\\examples\\intro\\hello_world.mdesk",
        "W:\\metadesk\\examples\\intro\\labels.mdesk",
        "W:\\metadesk\\examples\\intro\\sets.mdesk",
        "W:\\metadesk\\examples\\type_metadata\\types.mdesk",
        "W:\\metadesk\\examples\\type_metadata\\bad_types.mdesk",
        "W:\\metadesk\\examples\\expr\\expr_intro.mdesk",
        "W:\\metadesk\\examples\\expr\\expr_c_like.mdesk",
    };
    argc = 8;
    argv = argv_dummy;
#endif
    
    // make sure we have something to parse
    if (argc <= 1)
    {
        fprintf(stderr, "pass at least one input file");
        exit(1);
    }
    
    // setup the shared task data
    TaskData task = {0};
    task.task_max = argc - 1;
    task.tasks = argv + 1;
    
    // setup the per-thread data
#define THREAD_COUNT 2
    ThreadData threads[THREAD_COUNT] = {0};
    for (int i = 0; i < THREAD_COUNT; i += 1)
    {
        threads[i].task = &task;
        threads[i].arena = MD_ArenaAlloc();
        threads[i].list = MD_MakeList(threads[i].arena);
    }
    
    // launch the worker threads
    for (int i = 1; i < THREAD_COUNT; i += 1)
    {
#if MD_OS_WINDOWS
        HANDLE handle = CreateThread(0, 0, parse_worker_win32, threads + i, 0, 0);
        CloseHandle(handle);
#else
# error Not implemented for this OS
#endif
    }
    
    parse_worker_loop(&threads[0]);
    
    // wait for all threads to be finished
    for (;;)
    {
        MD_u64 thread_counter = task.thread_counter;
        if (thread_counter >= THREAD_COUNT)
        {
            break;
        }
#if MD_OS_WINDOWS
        Sleep(0);
#else
# error Not implemented for this OS
#endif
    }
    
    // print results
    for (int i = 0; i < THREAD_COUNT; i += 1)
    {
        fprintf(stdout, "on thread %d:\n", i);
        
        // print the name of each root
        for (MD_EachNode(root_it, threads[i].list->first_child))
        {
            MD_Node *root = MD_ResolveNodeFromReference(root_it);
            fprintf(stdout, "%.*s\n", MD_S8VArg(root->string));
        }
        
        // print the errors from this thread
        MD_MessageList errors = threads[i].errors; 
        for (MD_Message *message = errors.first;
             message != 0;
             message = message->next)
        {
            MD_CodeLoc loc = MD_CodeLocFromNode(message->node);
            MD_PrintMessage(stdout, loc, message->kind, message->string);
        }
    }
    
    // combine results
    MD_Arena *arena = threads[0].arena;
    MD_Node *list = threads[0].list;
    MD_MessageList errors = threads[0].errors; 
    for (int i = 1; i < THREAD_COUNT; i += 1)
    {
        MD_ArenaDefaultAbsorb(arena, threads[i].arena);
        MD_ListConcatInPlace(list, threads[i].list);
        MD_MessageListConcat(&errors, &threads[i].errors);
    }
}
