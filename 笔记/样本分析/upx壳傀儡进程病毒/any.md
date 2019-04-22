文件：C:\test\ProcessHowllowingPacked.exe<br>
文件大小	4096字节<br>
文件类型	PE32 executable (DLL) (console) Intel 80386, for MS Windows<br>
MD5	2469C7E897B343350B6277171E7E0DCF<br>
查壳<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/upx%E5%A3%B3.png)<br>
<br>
病毒使用VC2015编写的 upx3.0的壳<br>
<br>
<br>
在进入原程序入口dump这时upx已经解压缩完成<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/%E8%84%B1%E5%A3%B3.png)<br>
<br>
<br>
脱完壳修复iat后运行,可以看到创建挂起进程colorcpl.exe<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/%E5%88%86%E6%9E%90.png)<br>
<br>
最后设置线程eip为宿主进程oep,关闭原程序。<br>
整个流程是这样的：<br>
1、创建一个挂起的colorcpl.exe进程<br>
2、找colorcpl.exe进程的OEP备用<br>
3、在colorcpl.exe中开辟几块空间 存放自己的PeLoader、PE文件的ShellCode、参数结构<br>
4、将PeLoader、ShellCode、参数结构写入colorcpl.exe中<br>
5、Hook colorcpl.exe的OEP 改为PeLoader<br>
6、恢复colorcpl.exe进程执行PeLoader加载ShellCode并运行<br>
调用的winapi情况：<br>
GetThreadContext获取进程Context,eax为进程入口点，ebx指向进程peb<br>
ZwUnmapViewOfSection卸载colorcpl.exe进程内存空间数据<br>
VirtualAlloc分配内存空间<br>
WriteProcessMemory将代码写入到分配的内存<br>
SetThreadContext设置挂起进程状态<br>
ResumeThread唤醒进程运行<br>

