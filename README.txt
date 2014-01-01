名称 : foobar 2000 歌词显示插件
作者 : Excalibur

说明 : Lyrics show panel比较简洁。于是自己用OpenGL模仿QQ music的歌词软件做了这个东西。
支持windows vista以上系统(需要开启Aero)。本人只测过 foobar 2000 v1.2.9

foo_gl_lyrics之外的文件夹都是来自foobar 2000的sdk，sln文件也位于此目录。

编译环境 :
Windows 7专业版 
visual studio 2010 pro
SDK-2011-03-11(foobar sdk)
Microsoft Windows SDK v7.1

更新日志(部分)
2014-01-01 : 引入libpng(1.6.8)和zlib(1.2.7)库，增加对png纹理的支持

2013-12-02 : 增加选择字体以及字体颜色的功能，参考Issue #10

2013-11-24 : 修改窗口位置，使得窗口位置能够自适应屏幕大小。
增加Aero效果未启用的警告。
修改component信息

2013-11-11 : 默认歌词目录改为foobar安装目录下面的 lyrics
完成下载歌词功能

2013-10-30 : 目前歌词文件目录写死为：C:\\Users\\fireco007\\AppData\\Roaming\\foobar2000\\lyrics\\
这实际上就是lyrics show panel的歌词存放目录
字体目前也是在代码中写死的，用的微软雅黑字体。


快速开始
1：下载源代码
2：打开foo_gl_lyrics/foo_gl_lyrics.sln
3：编译出dll并将dll拷贝至foobar安装目录下的components子目录。
4：启动foobar，确认view菜单下的foobar2000 Lyrics为选中状态


Email : c265n46@163.com | c04n05@gmail.com
