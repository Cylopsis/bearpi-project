# js_sys_module Subsystems/Components

-   [Introduction](#Introduction)
-   [Directory](#Directory)
-   [Description](#Description)
    -   [Interface description](#Interface description)
    -   [Interface instructions](#Interface instructions)

-   [Related warehouse]](#Related warehouse])

## Introduction
Process is mainly used to obtain the relevant ID of the process, obtain and modify the working directory of the process, exit and close the process. The childprocess object can be used to create a new process. The main process can obtain the standard input and output of the child process, send signals and close the child process. 
## Directory

```
base/compileruntime/js_sys_module/
├── Class:PROCESS                   # class of PROCESS
├── Uid                             # attribute of Uid
├── Gid                             # attribute of Gid
├── EUid                            # attribute of EUid
├── EGid                            # attribute of EGid
├── Groups                          # attribute of Groups
├── Pid                             # attribute of Pid
├── Ppid                            # attribute of Ppid
├── chdir()                         # method of chdir
├── uptime()                        # method of uptime
├── kill()                          # method of kill
├── abort()                         # method of abort
├── on()                            # method of on
├── getTid()                        # method of getTid方法
├── getStartRealtime()              # method of getStartRealtime
├── getAvailableCores()             # method of getAvailableCores
├── getPastCputime()                # method of getPastCputime
├── isIsolatedProcess()             # method of isIsolatedProcess
├── is64Bit()                       # method of is64Bit
├── isAppUid()                      # method of isAppUid
├── getUidForName()                 # method of getUidForName
├── getThreadPriority()             # method of getThreadPriority
├── getSystemConfig()               # method of getSystemConfig
├── getEnvironmentVar()             # method of getEnvironmentVar
├── exit()                          # method of exit
├── cwd()                           # method of cwd
├── off()                           # method of off
├── runCmd()                        # method of runCmd
└─── Class:CHILDPROCESS             # class of CHILDPROCESS
    ├── close()                     # method of close
    ├── kill()                      # method of kill
    ├── getOutput()                 # method of getOutput
    ├── getErrorOutput()            # method of getErrorOutput
    ├── wait()                      # method of wait
    ├── killed                      # attribute of killed
    ├── pid                         # attribute of pid
    ├── ppid                        # attribute of ppid
    └── exitCode                    # attribute of exitCode
```

## Description

### Interface description
| Interface name | description |
| -------- | -------- |
| getUid() :number | returns the digital user ID of the process. |
| getGid() :number | returns the numeric group ID of the process. |
| getEUid() :number | returns the numeric valid user identity of the process. |
| getEGid() :number | returns the numeric valid group ID of the node.js process. |
| getGroups() :number[] |  returns an array with supplementary group ID. |
| getPid() :number | returns the PID of the process. |
| getPpid() :number |  returns the PID of the parent process of the current process. |
| chdir(dir:string) :void | change the current working directory of the node.js process. |
| uptime() :number |  returns the number of seconds the current system has been running. |
| Kill(pid:number, signal:number) :boolean | send the signal to the identified process PID, and true means the sending is successful. |
| abort() :void | cause the node.js process to exit immediately and generate a core file. |
| on(type:string ,listener:EventListener) :void | used to store events triggered by users. |
| exit(code:number):void | cause the node.js process to exit immediately. |
| cwd():string |  returns the current working directory of the node.js process. |
| off(type: string): boolean | clear the events stored by the user. True means the clearing is successful. |
| runCmd(command : string, options?: RunOptions): ChildProcess |through runcmd, you can fork a new process to run a shell and return the childprocess object. The first parameter command refers to the shell to be run, and the second parameter options refers to some running parameters of the child process. These parameters mainly refer to timeout, killsignal and maxbuffer. If timeout is set, the child process will send a signal killsignal after timeout is exceeded. Maxbuffer is used to limit the maximum stdout and stderr sizes that can be received. |
| wait()： Promise | is used to wait for the child process to run and return the promise object, whose value is the exit code of the child process. |
| getOutput(): Promise |  used to get the standard output of the child process. |
| getErrorOutput(): Promise | used to get the standard error output of the child process. |
| getTid() :number | Returns the TID of the process. |
| getStartRealtime() :number | Gets the real time elapsed (in milliseconds) from system startup to process startup. |
| getAvailableCores() :number[] | Gets the CPU kernel available to the current process on the multi-core device. |
| getPastCputime() :number | Gets the CPU time (in milliseconds) from the start of the process to the current time. |
| isIsolatedProcess(): boolean | Check if the process is quarantined. |
| is64Bit(): boolean | Check whether the process is running in a 64 bit environment. |
| isAppUid(v:number): boolean | Checks whether the specified uid belongs to a specific application. |
| getUidForName(v:string): number | Obtain the user group ID to which the user belongs according to the user name |
| getThreadPriority(v:number): number | Gets the thread priority based on the specified TID. |
| getSystemConfig(name:number): number | Gets the configuration of the system according to the specified system configuration name. |
| getEnvironmentVar(name:string): string | Obtain the corresponding value according to the name of the environment variable. |
| close(): void | used to close the running child process. |
| kill(signo: number): void |  used to send signals to child processes. |
| readonly killed: boolean | indicates whether the signal is sent successfully, and true indicates that the signal is sent successfully. |
| readonly exitCode: number | indicates the exit code of the child process. |
| pid | represents the child process ID. |
| ppid | represents the main process ID. |

### Interface instructions

Example of using interface：
1.getUid() 
```
getUid(){
    var res =  Process.getUid;
    console.log("-------"+res);
}
```
2.getGid()
```
getGid(){
    var result = Process.getGid;
    console.log("-------"+result);
}
```
3.getEuid()
```
getEuid(){
    var ans = Process.getEuid;
    console.log("-------"+ans);
}
```
4.getEgid()
```
getEgid(){
    var resb = Process.getEgid;
    console.log("-------"+resb);
}
```
5.getGroups()
```
getGroups(){
    var answer = Process.getGroups;
    console.log("-------"+answer);
}
```
6.getPid() 
```
getPid(){
    var result = Process.getPid;
    console.log("-----"+result);
}
```
7.getPpid()
```
getPpid(){
    var result = Process.getPpid;
    console.log("---------"+result);
}
```
8.chdir()
```
chdir(){
    Process.chdir("123456");
}
```
9.uptime()
```
uptime(){
    var num = Process.uptime();
    console.log("---------"+num);
}
```
10.kill()
```
kill(){
    var ansu = Process.kill(5,23);
    console.log("------"+ansu);
}
```
11.abort()
```
abort(){
    Process.abort();
}
```
12.on()
```
on(){
    function add(num){
        var value = num + 5;
        return value;
    }
    Process.on("add",add);
}
```
13.exit()
```
exit(){
    Process.exit(15);
}
```
14.Cwd()
```
Cwd(){
    var result = Process.cwd();
    console.log("----"+result);
}
```
15.off()

```
off(){
    var result =  Process.off("add");
    console.log("---------"+result);
}
```
16.runCmd()
```
runCmd(){
    var child = process.runCmd('echo abc')
    // killSignal can be a number or a string
    var child = process.runCmd('echo abc;', {killSignal : 'SIGKILL'});
    var child = process.runCmd('sleep 5; echo abc;', {timeout : 1, killSignal : 9, maxBuffer : 2})
}
```
17.wait()
```
wait()
{
    var child = process.runCmd('ls')
    var status = child.wait();
    status.then(val => {
        console.log(val);
    })
}
```
18.getOutput()
```
getOutput(){
    var child = process.runCmd('echo bcd;');
    var res = child.getOutput();
    child.wait();
    res.then(val => {
        console.log(val);
    })
}
```
19.getErrorOutput()
```
getErrorOutput(){
    var child = process.runCmd('makdir 1.txt'); // execute an error command
    var res = child.getErrorOutput();
    child.wait();
    res.then(val => {
        console.log(val);
    })
}
```
20.close()
```
close(){
    var child =  process.runCmd('ls; sleep 5s;')
    var result = child.close()
    console.log(child.exitCode);
}
```
21.kill()
```
kill(){
    var child =  process.runCmd('ls; sleep 5s;')
    var result = child.kill('SIGHUP');
    child.wait();
    var temp = child.killed;
    console.log(temp);
}
```
22.killed
```
{
    var child = process.runCmd('ls; sleep 5;')
    child.kill(3);
    var killed_ = child.killed;
    console.log(killed_);
    child.wait();
}
```
23.exitCode
```
{
    var child = process.runCmd('ls; sleep 5;')
    child.kill(9);
    child.wait();
    var exitCode_ = child.exitCode;
    console.log(exitCode_);
}
```
24.pid
```
pid
{
    var child = process.runCmd('ls; sleep 5;')
    var pid_ = child.pid;
    console.log(pid_);
    child.wait();
}
```
25.ppid
```
ppid
{
    var child = process.runCmd('ls; sleep 5;')
    var ppid_ = child.ppid;
    console.log(ppid_);
    child.wait();
}
```
26.getTid()
```
getTid(){
    var ansu = Process.getTid();
    console.log("------"+ansu);
}
```
27.isIsolatedProcess()
```
isIsolatedProcess(){
    var ansu = Process.isIsolatedProcess()();
    console.log("------"+ansu);
}
```
28.isAppUid()
```
isAppUid(){
    var ansu = Process.isAppUid(10000);
    console.log("------"+ansu);
}
```
29.is64Bit()
```
is64Bit(){
    var ansu = Process.is64Bit();
    console.log("------"+ansu);
}
```
30.getUidForName()
```
getUidForName(){
	var buf = "root";
    var ansu = Process.getUidForName(buf);
    console.log("------"+ansu);
}
```
31.getEnvironmentVar()
```
getEnvironmentVar(){
    var ansu = Process.getEnvironmentVar('USER');
    console.log("------"+ansu);
}
```
32.getAvailableCores()
```
getAvailableCores(){
    var ansu = Process.getAvailableCores();
    console.log("------"+ansu);
}
```
33.getThreadPriority()
```
getThreadPriority(){
	var result = Process.getTid();
    var ansu = getThreadPriority(result);
    console.log("------"+ansu);
}
```
34.getStartRealtime()
```
getStartRealtime(){
    var ansu = Process.getStartRealtime();
    console.log("------"+ansu);
}
```
35.getPastCputime()
```
getPastCputime(){
    var ansu = Process.getPastCputime();
    console.log("------"+ansu);
}
```
36.getSystemConfig()
```
getSystemConfig(){
    var _SC_ARG_MAX = 0;
    var ansu = Process.getSystemConfig(_SC_ARG_MAX)
    console.log("------"+ansu);
}
```


## Related warehouse

[js_sys_module subsystem](https://gitee.com/OHOS_STD/js_sys_module)

[base/compileruntime/js_sys_module/](base/compileruntime/js_sys_module-readme.md)
