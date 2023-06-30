# 操作系统实验

## 实验1-5 略



## 实验六

### 1. server与client通信

```shell
./server 
```

```shell
./client 124.*.*.* 10002 
```



### 2. Web Server

Web Server，能够响应PC机浏览器的HTTP请求，并通过HTTP响应向PC机浏览器传输静态网页和图片

图片：

```
http://124.*.*.*:8080/root/bz1.jpg
```

网页：

```
http://124.*.*.*:8085/root/bribri/index.html
```



### 3. 多进程或多线程

通过创建子进程或创建子线程，让Web Server可以同时处理多个PC浏览器的请求

server2.c代码69行

```
http://124.*.*.*:8080/root/bribri/index.html
```

```
http://124.*.*.*:8080/root/bribri/life.html
```

```
http://124.*.*.*:8080/root/bribri/jilu.html
```



## 实验七

### 打印hello

编译内核模块
加载内核模块
查询系统中已加载的内核模块
卸载内核模块
修改内核模块代码，利用printk函数实现格式化输出
printk(KERN_ALERT "Hello,Kernel!\n");
通过dmesg查看内核模块打印的log

makefile

```
ifeq ($(KERNELRELEASE),)
KERNELDIR ?= /lib/modules/$(shell uname -r)/build  
PWD := $(shell pwd)
modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
else
  obj-m := mymodule.o
endif
 
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions Module* modules*
```

```
make
insmod mymodule.ko
demsg
lsmod | grep mymodule 
rmmod mymodule.ko
dmesg
```

### 打印进程信息

在实验内容一的基础上将内核模块扩展为字符型设备驱动，在file_operation中实现open，read，write和ioctl方法编译测试应用参考代码，通过文件I/O操作设备文件，触发设备驱动中的方法调用在被触发的方法首先获取init进程的进程控制块，然后调用内核函数完成对进程控制块链表的遍历，并且通过printk对进程的PID，进程状态，进程优先级，父进程ID等信息进行打印输出（需要包含linux/sched.h，linux/init_task.h）struct task_struct *p;p=&init_task;for_each_process(p)

makefile

```
ifeq ($(KERNELRELEASE),)
        KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
        $(MAKE) -C $(KERNELDIR) M=$(PWD) modules

else
        obj-m := mychardev2.o
endif

clean:
        rm -rf *.o ~ core .depend ..cmd .ko .mod.c .tmp_versions Module modules

dev:
        #加载模块
        insmod mychardev2.ko
        #获取主设备号和次设备号
        #MAJOR=$(shell cat /proc/devices | awk '{if ($$2=="mychardev2") print $$1}')
        MINOR=0
        MAJOR=244
        #创建设备文件节点
        sudo mknod /dev/mychardev2 c 244 0
        sudo chmod 666 /dev/mychardev2

rmdev:
        #卸载模块
        rmmod mychardev2
        #删除设备文件节点
        sudo rm -f /dev/mychardev2

```

```
make
make dev
./main
dmesg
make rmdev
```

```
 cat /proc/devices  
```



### 打印到终端（选做）

在实验内容二的基础上通过在内核模块与应用程序之间共享内存来传递进程信息，从而让应用程序可以调用printf在标准输出上输出进程信息



## 实验九

### ARM汇编设计

进入main.c 后，先调用汇编程序Assembly_ADD，然后在汇编程序中会调用C程序C_transfer，然后汇编程序执行完返回main.c
Assembly_ADD：实现三个数相加
加数1：Assembly_ADD的第一个传入参数
加数2：汇编程序自定义数据段变量
加数3：立即数
Assembly_ADD第二个参数需要采用多种寻址方式
三数相加结果存入R0，作为汇编程序返回值



### 处理器状态

main.c调用汇编函数，汇编函数先切换为用户线程模式，然后通过SVC进入中断模式，中断模式中调用汇编函数切换为特权级线程模式，中断处理结束后返回用户态线程模式



### 蜂鸣器和串口

1、编写程序，识别KEY_UP键的按下，在按下后通过控制BEEP（PF8）输出高电平来让蜂鸣器鸣叫
2、编写程序，实现通过通过串口输出字符串功能
3、编写程序，识别KEY0、KEY1、KEY2三个键的按下，在识别后对应控制串口输出“KEY0 is pressed”, “KEY1 is pressed” ， “KEY2 is pressed” 字符串



## 实验十

### 多任务切换

在起始任务中用OSTaskCreate创建三个优先级不同的任务
任务1：负责控制LED0的闪烁
任务2：负责控制LED1的闪烁
任务3：负责串口输出自定义数据
要求：在任务1、2、3中通过OSTimeDly或OSTaskSuspend挂起自己，然后其他任务通过OSTaskResume来唤醒的方式，实现按照时间片轮转循环调度的效果



### ucos信号量机制

–编写程序通过起始任务创建4个任务，其中2个生产者，2个消费者。

–生产者向缓冲区写入数据，并通过二值信号量通知消费者

–消费者从缓冲区读出数据，并通过串口进行输出

–生产者与生产者，消费者与消费者，生产者与消费者之间都通过互斥量来保证对缓冲区的互斥访问

可通过挂起与唤醒实现一种读写顺序



编写程序通过起始任务创建4个任务，其中2个生产者，2个消费者。
生产者1负责检测按键的按下，并通过四个不同的信号量（对应四个不同的按键）通知消费者1
生产者2负责检测串口输入的字符，然后写入缓冲区，然后通过信号量通知消费者2
消费者1等待生产者1的信号量，并根据等待信号量的不同分别点亮LED0，LED1，控制蜂鸣器啸叫以及通过串口输出字符串
消费者2等待生产者2的信号量，然后读取缓冲区，并且将缓冲区中的字符反向显示
通过挂起与唤醒实现：生产者1->消费者1->生产者2->消费者2->生产者1-> 



### part 3

![](https://pic.imgdb.cn/item/649e81531ddac507cc36626b.png)





## 实验11

ucosii.h

```
struct os_tcb *OSTSnext;
struct os_tcb *OSTSprev;
INT8U OSTSLen;
INT8U OSTSCurLen;
```

+OSTaskCreate和OSTaskCreateExt的参数修改



ostaskcreate.c(ucore.c)

参数：

```
INT8U  OSTaskCreate (void   (*task)(void *p_arg),
                     void    *p_arg,
                     OS_STK  *ptos,
                     INT8U    prio,
					INT16U   id,
					 INT8U    TSlen)
```

删除了优先级必须唯一的判定

```c
if (OSTCBPrioTbl[prio] == (OS_TCB *)0) { /* Make sure task doesn't already exist at this priority  */
        //OSTCBPrioTbl[prio] = OS_TCB_RESERVED;/* Reserve the priority to prevent others from doing ...  */
                                             /* ... the same thing until task is created.              */
        OS_EXIT_CRITICAL();
        psp = OSTaskStkInit(task, p_arg, ptos, 0u);             /* Initialize the task's stack         */
        err = OS_TCBInit(prio, psp, (OS_STK *)0, id, 0u, (void *)0, 0u,TSlen);
        if (err == OS_ERR_NONE) {
					  OS_ENTER_CRITICAL();
					  OSTaskCtr++;
			  	  OS_EXIT_CRITICAL();
            if (OSRunning == OS_TRUE) {      /* Find highest priority task if multitasking has started */
                OS_Sched();
            }
        } else {
            OS_ENTER_CRITICAL();
            OSTCBPrioTbl[prio] = (OS_TCB *)0;/* Make this priority available to others                 */
            OS_EXIT_CRITICAL();
        }
        return (err);
    }
		else 
		{
			psp=(OS_STK*)OSTaskStkInit(task,p_arg,ptos,0);      //init stack
			err=OS_TCBInit(prio,psp,(OS_STK*)0,id,0u,(void*)0,0,TSlen);
			if(err==OS_ERR_NONE)
			{
				OS_ENTER_CRITICAL();
				OSTaskCtr++;
				OS_EXIT_CRITICAL();
				if(OSRunning==OS_TRUE)
					OS_Sched();
				return (err);
			}
		}
```



(oscore.c)

参数

```c
INT8U  OS_TCBInit (INT8U    prio,
                   OS_STK  *ptos,
                   OS_STK  *pbos,
                   INT16U   id,
                   INT32U   stk_size,
                   void    *pext,
                   INT16U   opt,
									 INT8U    TSlen)
```

```c
ptcb->OSTSLen=TSlen;
ptcb->OSTSCurLen=TSlen;
ptcb->OSTSnext=(OS_TCB*)0;
ptcb->OSTSprev=(OS_TCB*)0;
```

```c
//将同优先级任务头插法加入链表
if(OSTCBPrioTbl[prio]==(OS_TCB*)0)
				{
					 OSTCBPrioTbl[prio] = ptcb;
					 ptcb->OSTSnext=ptcb;
					 ptcb->OSTSprev=ptcb;
					
					ptcb->OSTCBNext    = OSTCBList;                    /* Link into TCB chain                      */
          ptcb->OSTCBPrev    = (OS_TCB *)0;
          if (OSTCBList != (OS_TCB *)0) {
            OSTCBList->OSTCBPrev = ptcb;
          }
					OSTCBList               = ptcb;
          OSRdyGrp               |= ptcb->OSTCBBitY;         /* Make task ready to run                   */
          OSRdyTbl[ptcb->OSTCBY] |= ptcb->OSTCBBitX;
				}
				else 
				{
						ptcb->OSTSnext=OSTCBPrioTbl[prio]->OSTSnext;
					  OSTCBPrioTbl[prio]->OSTSnext=ptcb;
				  	ptcb->OSTSprev=OSTCBPrioTbl[prio];
				  	(ptcb->OSTSnext)->OSTSprev=ptcb;
					
					  ptcb->OSTCBNext = (OS_TCB *)0;
	      	  ptcb->OSTCBPrev = (OS_TCB *)0;
				}
```



OSTIMETICK

```c
if(OSTCBCur->OSTSCurLen>0)
				{
					//printf("id is %d,time:%d\n",OSTCBCur->OSTCBId,OSTCBCur->OSTSCurLen);
					if(--OSTCBCur->OSTSCurLen==0)
					{
						if(OSTCBCur->OSTSnext != OSTCBCur)
						{
						//	printf(" id:%d\n",OSTCBCur->OSTCBId);
							ptcb=OSTCBCur->OSTSnext;
							while(ptcb->OSTCBId!=OSTCBCur->OSTCBId)
							{
						//		printf(" id:%d\n",ptcb->OSTCBId);
								ptcb=ptcb->OSTSnext;
							}

							OSTCBCur->OSTSCurLen=OSTCBCur->OSTSLen;
							OSTCBPrioTbl[OSTCBCur->OSTCBPrio]=OSTCBCur->OSTSnext;
							
							OSTCBPrioTbl[OSTCBCur->OSTCBPrio]->OSTCBNext=OSTCBCur->OSTCBNext;
							OSTCBPrioTbl[OSTCBCur->OSTCBPrio]->OSTCBPrev=OSTCBCur->OSTCBPrev;
							if (OSTCBCur->OSTCBNext != (OS_TCB *)0) {
				         OSTCBCur->OSTCBNext->OSTCBPrev = OSTCBPrioTbl[OSTCBCur->OSTCBPrio];    
				      }
							if (OSTCBCur->OSTCBPrev != (OS_TCB *)0) {
				         OSTCBCur->OSTCBPrev->OSTCBNext = OSTCBPrioTbl[OSTCBCur->OSTCBPrio];    
				      }
							OSTCBCur->OSTCBPrev = (OS_TCB *)0;
				      OSTCBCur->OSTCBNext = (OS_TCB *)0;
							
							
						//	printf("prio become %d\n",OSTCBPrioTbl[OSTCBCur->OSTCBPrio]->OSTCBId);
					//		printf(" id:%d\n",OSTCBPrioTbl[OSTCBCur->OSTCBPrio]->OSTCBId);
							ptcb=OSTCBPrioTbl[OSTCBCur->OSTCBPrio]->OSTSnext;
							while(ptcb->OSTCBId!=OSTCBPrioTbl[OSTCBCur->OSTCBPrio]->OSTCBId)
							{
					//			printf(" id:%d\n",ptcb->OSTCBId);
								ptcb=ptcb->OSTSnext;
							}
							
							/* (2) update OSTCBList if TCBcur is OSTCBList*/
	                if (OSTCBCur == OSTCBList) {
					           OSTCBList = OSTCBCur->OSTSnext;   
					        }			

					    /* (3) Compute X, Y, BitX and BitY */
					       OSTCBCur->OSTSnext->OSTCBY = OSTCBCur->OSTSnext->OSTCBPrio >> 3;	 
					       OSTCBCur->OSTSnext->OSTCBBitY = (INT8U)(1u << (OSTCBCur->OSTSnext->OSTCBPrio & 0x07)); 
					       OSTCBCur->OSTSnext->OSTCBX = OSTCBCur->OSTSnext->OSTCBPrio & 0x07;                        
					       OSTCBCur->OSTSnext->OSTCBBitX = (INT8U)(1u << (OSTCBCur->OSTSnext->OSTCBX));
					
				    /* (4) set task to be ready */
					       if (OSTCBCur->OSTSnext->OSTCBDly == 0) {
						    OSRdyGrp |= OSTCBCur->OSTSnext->OSTCBBitY;
						    OSRdyTbl[OSTCBCur->OSTSnext->OSTCBY] |= OSTCBCur->OSTSnext->OSTCBBitX;
					   }	
							
						//	OSPrioCur=0;
						}
					}
				}
				
				OS_EXIT_CRITICAL();
```



OSINTEXIT和OS_SCHED

改变调度的要求，不是优先级值不同了才能调度

```c
if (OSRunning == OS_TRUE) {
        OS_ENTER_CRITICAL();
        if (OSIntNesting > 0u) {                           /* Prevent OSIntNesting from wrapping       */
            OSIntNesting--;
        }
        if (OSIntNesting == 0u) {                          /* Reschedule only if all ISRs complete ... */
            if (OSLockNesting == 0u) {                     /* ... and not locked.                      */
                OS_SchedNew();
                OSTCBHighRdy = OSTCBPrioTbl[OSPrioHighRdy];
							//printf("int exit:now id %d,high id %d\n",OSTCBCur->OSTCBId,OSTCBHighRdy->OSTCBId);
                if (OSTCBHighRdy != OSTCBCur) {          /* No Ctx Sw if current task is highest rdy */
#if OS_TASK_PROFILE_EN > 0u
                    OSTCBHighRdy->OSTCBCtxSwCtr++;         /* Inc. # of context switches to this task  */
#endif
                    OSCtxSwCtr++;                          /* Keep track of the number of ctx switches */
                    OSIntCtxSw();                          /* Perform interrupt level ctx switch       */
                }
            }
        }
        OS_EXIT_CRITICAL();
    }
```

```c
//OS_SCHED
if (OSIntNesting == 0u) {                          /* Schedule only if all ISRs done and ...       */
        if (OSLockNesting == 0u) {                     /* ... scheduler is not locked                  */
            OS_SchedNew();
            OSTCBHighRdy = OSTCBPrioTbl[OSPrioHighRdy];
					//printf("high prio %d\n",OSPrioHighRdy);
					//printf("shced:now id %d,high id %d\n",OSTCBCur->OSTCBId,OSTCBHighRdy->OSTCBId);
            if (OSTCBPrioTbl[OSPrioHighRdy] != OSTCBCur) {          /* No Ctx Sw if current task is highest rdy     */
#if OS_TASK_PROFILE_EN > 0u
                OSTCBHighRdy->OSTCBCtxSwCtr++;         /* Inc. # of context switches to this task      */
#endif
                OSCtxSwCtr++;                          /* Increment context switch counter             */
							  //printf("make change\n");
                OS_TASK_SW();                          /* Perform a context switch                     */
            }
        }
    }
```



OSTASKDEL

```c
//lsy
		if(ptcb->OSTSnext!=ptcb)  //还有同优先级，那就手动移指针
		{
			OSTCBCur->OSTSnext->OSTCBNext=OSTCBCur->OSTCBNext;
			OSTCBCur->OSTSnext->OSTCBPrev=OSTCBCur->OSTCBPrev;
		 if (OSTCBCur->OSTCBNext != (OS_TCB *)0) {
				 OSTCBCur->OSTCBNext->OSTCBPrev = OSTCBCur->OSTSnext;    
			}
			if (OSTCBCur->OSTCBPrev != (OS_TCB *)0) {
				  OSTCBCur->OSTCBPrev->OSTCBNext = OSTCBCur->OSTSnext;    
			}
			OSTCBPrioTbl[prio]=OSTCBCur->OSTSnext;
		  ptcb1=OSTCBPrioTbl[prio];
			while(ptcb1->OSTSnext->OSTCBId!=OSTCBCur->OSTCBId)
			{
					//printf("loop id:%d\n",ptcb1->OSTCBId);
					ptcb1=ptcb1->OSTSnext;
			}
			//printf("connect :%d\n",ptcb1->OSTCBId);
			//printf("before :%d\n",ptcb1->OSTSnext->OSTCBId);
			//printf("get :%d\n",OSTCBCur->OSTSnext->OSTCBId);
			ptcb1->OSTSnext=OSTCBCur->OSTSnext;
		//	printf("get :%d\n",ptcb1->OSTSnext->OSTCBId);
			OSTCBCur->OSTSnext->OSTCBY = OSTCBCur->OSTSnext->OSTCBPrio >> 3;	 
		  OSTCBCur->OSTSnext->OSTCBBitY = (INT8U)(1u << (OSTCBCur->OSTSnext->OSTCBPrio & 0x07)); 
			OSTCBCur->OSTSnext->OSTCBX = OSTCBCur->OSTSnext->OSTCBPrio & 0x07;                        
			OSTCBCur->OSTSnext->OSTCBBitX = (INT8U)(1u << (OSTCBCur->OSTSnext->OSTCBX));
			if (OSTCBCur->OSTSnext->OSTCBDly == 0) {
					OSRdyGrp |= OSTCBCur->OSTSnext->OSTCBBitY;
					OSRdyTbl[OSTCBCur->OSTSnext->OSTCBY] |= OSTCBCur->OSTSnext->OSTCBBitX;
			}	
			
			OSTCBCur->OSTCBPrev = (OS_TCB *)0;
			OSTCBCur->OSTCBNext = (OS_TCB *)0;
		}
		else 
	{
		//lsy
		OSTCBPrioTbl[prio] = (OS_TCB *)0;                   /* Clear old priority entry                    */
		
    if (ptcb->OSTCBPrev == (OS_TCB *)0) {               /* Remove from TCB chain                       */
        ptcb->OSTCBNext->OSTCBPrev = (OS_TCB *)0;
        OSTCBList                  = ptcb->OSTCBNext;
    } else {
        ptcb->OSTCBPrev->OSTCBNext = ptcb->OSTCBNext;
        ptcb->OSTCBNext->OSTCBPrev = ptcb->OSTCBPrev;
    }
	}
```

