/* 
** Example: multi threaded parse
**
** This example shows how to arrange a multi-threaded Metadesk parser. The
** strategy used is to make each *.mdesk file into an independent task.
**
** The goal in this example is to have all of the files parsed and visible at
** the same time by the end. Another conceivable way to make a multi-threaded
** Metadesk parser would be to use each parse as a temporary and throw them
** away after extracting out the important parts.
**
** This example depends directly on OS headers, intrinsics, and functions,
** so you'll have to re-interpret those parts to whatever OS you are targeting.
**
*/

//~ includes and globals //////////////////////////////////////////////////////

#include "md.h"
#include "md.c"

#if MD_OS_WINDOWS
# include <Windows.h>
#else
# error Not implemented for this OS
#endif

//~ multi-threaded parse setup ////////////////////////////////////////////////

// for this intrinsic we're assume pre-increment behavior

#if MD_OS_WINDOWS
# define atomic_inc_then_eval_u64(p) InterlockedIncrement64((LONG64*)p)
#else
# error Not implemented for this OS
#endif

// @notes We use one structure that describes the whole multi-threaded parse
//  work load, and carries the shared state values for synchronizing the
//  worker threads.
typedef struct TaskData
{
    // the set of tasks
    MD_u64 task_max;
    char **tasks;
    
    // synchronization
    volatile MD_u64 task_counter;
    volatile MD_u64 thread_counter;
} TaskData;

// @notes Each thread gets it's own thread data.
//
// The library doesn't make any of it's data structures thread safe but all of
// the calls are thread safe so long as different threads are operating on
// different data structures. So we arrange for each worker thread to get it's
// own arena, and set of lists for collecting parse results.
typedef struct ThreadData
{
    // shared
    TaskData *task;
    
    // unique-to-thread
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
        // atomically get the next unhandled task index
        MD_u64 task_index = atomic_inc_then_eval_u64(&task->task_counter) - 1;
        if (task_index >= task->task_max)
        {
            break;
        }
        
        // load and parse the file specified by this task.
        MD_String8 file_name = MD_S8CString(task->tasks[task_index]);
        MD_ParseResult parse = MD_ParseWholeFile(thread_data->arena, file_name);
        MD_MessageListConcat(&thread_data->errors, &parse.errors);
        MD_PushNewReference(thread_data->arena, thread_data->list, parse.node);
    }
    
    // atomically count the threads as they finish
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
    //  (no worker thread 0)
    for (int i = 1; i < THREAD_COUNT; i += 1)
    {
#if MD_OS_WINDOWS
        HANDLE handle = CreateThread(0, 0, parse_worker_win32, threads + i, 0, 0);
        CloseHandle(handle);
#else
# error Not implemented for this OS
#endif
    }
    
    // let the main thread act as thread 0
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
    
    // @notes In this example we are done, but in some cases it might be useful
    //  to merge the results of a multi-threaded parse to make it as if it was
    //  a single threaded parse, and it turns out this is quite easy to do from
    //  here.
    //
    //  We merge all of the arenas together so that we can handle all of the
    //  memory with a single arena moving forward. This relies on the default
    //  arena implementation which has an 'absorb' operation. If you plug in 
    //  your own arena via oerrides it's up to that implementation what your
    //  options are for this part.
    //
    //  The list of roots and the list of messages can be concatenated.
    //
    //  All of these operations (absorb and the concat operations) move
    //  information out of the right hand operand and into the left hand
    //  operand, leaving the individual pieces from the worker threads invalid
    //  after the merge.
    
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
