#!/bin/bash
#第一讲 初始化远端库
cd parent_dir		#进入项目父目录
mkdir gitDemo		#创建项目目录 gitDemo
cd gitDemo			#进入项目目录
git init			#初始化空的 git 仓库
touch README.md   
git add README.md	#这两行添加简单的 README.md 文件
git commit -m "first commit" #提交时附加的信息
git remote add origin https://coding.net/codingTutorial/gitDemo.git  #添加一个名为 origin 的远端（url为git地址）
git push -u origin master #将该目录下的文件推送到远端（origin）上的 "master" 分支


#第二讲 增加到远端库
git clone https://git.coding.net/durongze/lanchat.git
git add .
git commit    进入 VIM ， 去除 add file 之前的各个#号，然后输入 :wq
git push origin master

#第三讲 修改远端库
git add .
git commit    进入 VIM ， 去除 add file 之前的各个#号，然后输入 :wq
git push 

