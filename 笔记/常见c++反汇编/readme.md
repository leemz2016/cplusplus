if-else分支
=========
与常量进行比较时debug版<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/if-else%E7%AE%80%E5%8D%95%E5%B8%B8%E9%87%8F%E6%AF%94%E8%BE%83debug.png)<br>
if-else 分支用的都是反比jle<br>
if-else分支的特点如下<br>
cmp exx,xx<br>
jxx xxxxxxxx<br>
分支1<br>
jmp xxxxxxxx<br>
分支2<br>
<br>
<br>
release版<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/if-else%E7%AE%80%E5%8D%95%E5%8F%98%E9%87%8F%E6%AF%94%E8%BE%83%20release.png)<br>
编译器在预处理前检测到判断条件是一个常量,所以去掉了那些不可达的分支<br>
<br>
<br>
以变量为判断条件的if-else分支<br>
debug版与常量差不多形式<br>
release版<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/if-else%E5%8F%98%E9%87%8F%E6%AF%94%E8%BE%83release.png)<br>
以变量为判断条件的if-else分支,相同功能的分支会被合并,不能得到原结构<br>
<br>
<br>

do-while循环
==========
有一个有条件判断的向上跳转<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/do-while.png)<br>
特点总结:<br>
tag:xxx<br>
xxxxx<br>
cmp xxx,xxx<br>
jxx tag<br>
<br>
<br>
while循环
=======
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/while.png)<br>
特点总结：<br>
while_tag:<br>
xxx<br>
cmp xxx,xxx<br>
jxx while-end-tag<br>
xxxx<br>
cmp xxx,xxx<br>
jxx while_tag<br>
xxx<br>
while_end-tag<br>


