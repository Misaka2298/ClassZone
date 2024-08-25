# ClassZone

[![stars](https://img.shields.io/github/stars/Misaka2298/ClassZone?label=Stars)](https://github.com/Misaka2298/ClassZone)
[![Release](https://img.shields.io/github/v/release/Misaka2298/ClassZone?style=flat-square&color=%233fb950&label=正式版)](https://github.com/Misaka2298/ClassZone/releases/latest) 
[![Downloads](https://img.shields.io/github/downloads/Misaka2298/ClassZone/total?style=social&label=下载量&logo=github)](https://github.com/Misaka2298/ClassZone/releases/latest)

ClassZone 是一款面向微信的每日班级信息报告机器人，与多款班级桌面信息软件兼容。

御坂球的处女作，阅读本项目的代码就会像赤了三进式一样，请谨慎阅读。

名字是喝蜜雪冰城时一拍大腿想出来的。


> [!warning]
> 锐意开发中，**Coming s∞n**.

受限于本人的技术力，部分代码来源于AIGC。

### 分支说明
本项目分为**Master**，**ClassClient**与**Server**三个分支。

- **Master**为默认分支，仅作为展示页存储README与LICENSE。

- [**ClassClient**](https://github.com/Misaka2298/ClassZone/tree/ClassClient)为在班级运行的版本，用来每天截图作业条并上传至Server。

- [**Server**](https://github.com/Misaka2298/ClassZone/tree/Server)为在服务器运行的版本，用来实现其他功能。

### 功能
- [ ] 每日自动往指定群聊中发送次日课程表，从[ClassIsland](https://github.com/ClassIsland/ClassIsland)或[ElectronClassSchedule](https://github.com/EnderWolf006/ElectronClassSchedule)的配置文件中获取课程表。
- [ ] 每日自动往指定群聊中发送今日作业条，从[StickyHomeworks](https://github.com/HelloWRC/StickyHomeworks)中获取作业条。
- [ ] 每日自动往指定群聊中发送每日一言，可自行设定每日一言API。

### 第三方库
- [Nlohmann/Json](https://github.com/nlohmann/json)
