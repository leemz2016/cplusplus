样本来源于http://iec56w4ibovnb4wc.onion/Library/ ShadowHammer。<br>
关于华硕供应链攻击和之前的cclear差不多,入侵之后窃取证书源码,替换源程序<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/%E8%AF%81%E4%B9%A6.png)<br>
<br>
根据嘶吼的分析可知, 攻击者已经将Shellcode插入Setup.exe中，并修改代码，从而实现重定向执行。现在着重分析下shellcode解密与提取<br>
ida中sub_51B908函数为<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/58b90b.png)<br>
<br>
拷贝shellcode前16字节到分配的内存中,在偏移1B802函数中对这16字节每一位进行移位操作,最后得到真正shellcode大小为0x5610字节<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/16%E5%AD%97%E8%8A%82.png)<br>
<br>
之后就是调用virtualalloc分配内存将shellcode加载到写入<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/write.png)<br>
然后又一次调用偏移1B802处的函数对0x5610字节大小的shellcode每一个字节进程移位操作，最后执行shellcode入口偏移0FAB处<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/2.png)<br>
