#include <linux/module.h>
#include <linux/tracepoint.h>

/* ---------------------------------------------修改用于其他tracepoint时不用动的部分--------------------------------------------------- */

/* 用于管理感兴趣的tracepoint，即我要使用的tracepoint */
struct interest_tracepoint
{
    void *callback; /* 指向tracepoint要执行的回调函数 */
    /* 对于每个tracepoint，内核都会使用一个struct tracepoint结构体来管理，这个指针就指向这个结构体 */
    struct tracepoint *ptr;
    char is_registered; /* 记录我想使用的tracepoint对应回调函数是否已经注册, 0表示否，1表示是 */
};

/* 用于生成一个struct interest_tracepoint结构体，并初始化，参数：
tracepoint_name：要使用的tracepoint名称，比如sched_switch*/
#define INIT_INTEREST_TRACEPOINT(tracepoint_name) \
    static struct interest_tracepoint tracepoint_name##_tracepoint = {.callback = NULL, .ptr = NULL, .is_registered = 0};

/* 该宏用于生成for_each_kernel_tracepoint的回调函数，前者用于遍历整个内核的tracepoint表。
for_each_kernel_tracepoint每找到一个表项就会调用该回调函数。宏参数：
tracepoint_name：你想要找到的tracepoint的名称，比如sched_switch。
生成的回调函数使用的参数由for_each_kernel_tracepoint对回调函数的规定所决定。生成后函数的参数：
tp：找到的struct tracepoint 的指针
priv: 注册回调函数时传入的自定义数据指针，我注册时会传入用于管理我要使用的tracepoint的
struct interest_tracepoint指针*/
#define TRACEPOINT_FIND(tracepoint_name)                                             \
    static void tracepoint_name##_tracepoint_find(struct tracepoint *tp, void *priv) \
    {                                                                                \
        if (!strcmp(#tracepoint_name, tp->name))                                     \
        {                                                                            \
            ((struct interest_tracepoint *)priv)->ptr = tp;                          \
            return;                                                                  \
        }                                                                            \
    }

/* 用于注销一个tracepoint的回调函数 */
static void clear_tracepoint(struct interest_tracepoint *interest)
{

    /* 判断我们的回调函数是否已经注册 */
    if (interest->is_registered)
    { /* 进来，说明我们的回调函数是注册了的 */
        /* 注销我们在对应tracepoint上注册的回调函数 */
        tracepoint_probe_unregister(interest->ptr, interest->callback, NULL);
    }
}

/* --------------------------------------修改以下部分用于其他tracepoint------------------------------------------------------------------- */

/* 生成并初始化struct interest_tracepoint sched_switch */
INIT_INTEREST_TRACEPOINT(sched_switch)

/* 生成sched_switch_tracepoint_find函数 */
TRACEPOINT_FIND(sched_switch)

/* tracepoint: sched_switch触发时的回调函数，由我们自行挂载到该tracepoint上。
每个tracepoint的回调函数可接受的参数由include/trace/events/xxx.h 中的TRACE_EVENT宏定义确定。
对于tracepoint: sched_switch，参数定义在include/trace/events/sched.h 中的
TRACE_EVENT(sched_switch 中，为TP_PROTO(bool preempt, struct task_struct *prev, struct task_struct *next)。
因此本回调函数除了第一个参数外，其余参数就是由该TP_PROTO宏中的内容决定，无法更改参数内容与顺序。
void *data参数是在注册该回调函数时提供的额外数据，可以指向任何数据结构，取决于具体需求。
例如，如果在注册该回调函数时指定void *data指向某个特定结构体，那么每当该回调函数被触发时，
就可以通过data参数访问该结构体。这提供了一种灵活的方式来传递额外信息给回调函数，
避免了使用全局变量或其他潜在的问题引入方法。 */
static void sched_switch_tracepoint_callback(void *data, bool preempt, struct task_struct *prev, struct task_struct *next)
{
    /* 自定义代码 */
}

static int __init tracepoint_init(void)
{

    /* 在我们自己的struct interest_tracepoint中记录管理的tracepoint要调用的回调函数 */
    sched_switch_tracepoint.callback = sched_switch_tracepoint_callback;

    /* tracepoint并不是直接导出供外部模块直接引用的（http://lkml.iu.edu/hypermail/linux/kernel/1504.3/01878.html)）,
    内核模块不能直接通过外部符号来访问tracepoint。所以需要用for_each_kernel_tracepoint遍历tracepoint表的方式来找到指定tracepoint。
    这需要我们注册一个回调函数用于比对找到的tracepoint条目是不是我们想要的tracepoint*/
    for_each_kernel_tracepoint(sched_switch_tracepoint_find, &sched_switch_tracepoint);

    /* 判断有没有找到想要使用的tracepoint对应的struct tracepoint指针 */
    if (!sched_switch_tracepoint.ptr)
    {
        pr_info("sched_switch's struct tracepoint not found\n");
        return 0;
    }

    /* 在内核管理sched_switch的struct tracepoint中填上我们的回调函数，这就完成了回调函数的注册 */
    tracepoint_probe_register(sched_switch_tracepoint.ptr, sched_switch_tracepoint.callback, NULL);

    sched_switch_tracepoint.is_registered = 1; /* 记录我们的回调函数已经注册 */

    return 0;
}

static void __exit tracepoint_exit(void)
{
    clear_tracepoint(&sched_switch_tracepoint);
}

module_init(tracepoint_init);
module_exit(tracepoint_exit);
MODULE_LICENSE("GPL");
