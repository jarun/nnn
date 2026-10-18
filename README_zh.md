> **语言**：本文档为简体中文翻译，[点此查看英文原版 README.md](README.md)。

<h3 align="center"><img src="misc/logo/logo-128x128.png" alt="nnn"><br>nnn - <i>让你的生产力飙升！</i></h3>

<p align="center">
<a href="https://github.com/jarun/nnn/releases/latest"><img src="https://img.shields.io/github/release/jarun/nnn.svg?maxAge=600&label=rel" alt="Latest release" /></a>
<a href="https://repology.org/project/nnn/versions"><img src="https://repology.org/badge/tiny-repos/nnn.svg?header=repos" alt="Availability"></a>
<a href="https://circleci.com/gh/jarun/workflows/nnn"><img src="https://img.shields.io/circleci/project/github/jarun/nnn.svg?label=CircleCI" alt="Circle CI Status" /></a>
<a href="https://github.com/jarun/nnn/actions"><img src="https://github.com/jarun/nnn/actions/workflows/ci.yml/badge.svg?branch=master" alt="GitHub CI Status" /></a>
<a href="https://en.wikipedia.org/wiki/Privacy-invasive_software"><img src="https://img.shields.io/badge/privacy-✓-crimson?maxAge=2592000" alt="Privacy Awareness" /></a>
<a href="https://github.com/jarun/nnn/blob/master/LICENSE"><img src="https://img.shields.io/badge/©-BSD%202--Clause-important.svg?maxAge=2592000" alt="License" /></a>
</p>

<p align="center"><a href="https://github.com/user-attachments/assets/541ca36d-ae26-49fb-97da-d1f7a12d4b9a"><img src="https://github.com/user-attachments/assets/a8ee4689-1552-4fb1-997e-b16fe8ef5086"></a></p>

<h3 align="center">[<a
href="https://github.com/jarun/nnn#features">功能</a>] $~~$ [<a
href="https://github.com/jarun/nnn#quickstart">快速上手</a>] $~~$ [<a
href="https://github.com/jarun/nnn/tree/master/plugins#nnn-plugins">插件</a>] $~~$ [<a
href="https://github.com/jarun/nnn/wiki">Wiki</a>]</h3>

`nnn`（_n³_）是一款功能完备的终端文件管理器。它体积小巧（约 150 KiB），[几乎零配置](https://github.com/jarun/nnn#quickstart)且[速度极快](https://github.com/jarun/nnn/wiki/Performance)。

它的设计力求低调无扰，以智能的工作流契合你的思路。

`nnn` 能够分析磁盘占用、批量重命名、启动应用、挑选文件。插件仓库里有大量插件可进一步扩展其能力，例如[实时预览](https://github.com/jarun/nnn/wiki/Live-previews)、（卸载）挂载磁盘、查找与列出、文件/目录 diff、上传文件。一个[补丁框架](https://github.com/jarun/nnn/tree/master/patches)收录了体量可观、性质偏主观的用户提交补丁。

独立的 (neo)vim 插件 —— [nnn.vim](https://github.com/mcchrish/nnn.vim)、[vim-floaterm nnn wrapper](https://github.com/voldikss/vim-floaterm#nnn) 以及 [nnn.nvim](https://github.com/luukvbaal/nnn.nvim)（仅限 Neovim）。

可运行于树莓派、[Termux](https://www.youtube.com/embed/AbaauM7gUJw)（Android）、Linux、macOS、BSD、Haiku、Cygwin、WSL，跨各类桌面环境（DE）或纯 CLI 环境。

[_(更多用例)_](https://github.com/jarun/nnn/wiki/Basic-use-cases#the_nnn-magic)

<a href="https://build.demoshell.com/launch?snapshot=demoshell%2Ftui%3Annn"><img src="https://build.demoshell.com/v1/embed/badge.svg" alt="Live demo by Demoshell" align="absmiddle"></a>

## 功能特性

- 质量
  - 注重隐私（未经确认不收集用户数据）
  - POSIX 兼容，遵循 Linux 内核编码风格
  - 高度优化，集成静态分析代码
- 精简
  - 通常常驻内存不足 3.5MB
  - 支持 8 色（以及 xterm 256 色）
  - 注重磁盘 I/O（很少的磁盘读写）
  - 不使用 FPU（全部为整数运算，连文件大小也是）
  - 以快速的行重绘将屏幕刷新降到最低
  - 小巧的二进制文件（通常约 150KB）
  - 适用于更小终端与更小巧形态的单列模式
  - 可魔改 —— 按需编译启用/禁用特性与依赖
- 便携
  - 语言无关的插件
  - 提供静态二进制（无需安装）
  - 依赖库极少，易于编译
  - 无配置文件，以最少的配置和合理的默认值运行
  - 用于备份配置的插件
  - 在许多打包平台（packagers）上广泛可用
  - 支持触控，便捷的手持设备快捷键
  - Unicode 支持
- 模式
  - 精简（默认）、详细
  - 磁盘占用分析器（block/apparent）
  - 文件挑选器、(neo)vim 插件
- 导航
  - 过滤并可在唯一匹配时自动进入目录
  - *键入即导航*（Turbo 导航/始终过滤）模式
  - 以可见的相对偏移跳转到某个条目
  - 上下文（*又称*标签页/工作区）可自定义颜色
  - 会话、书签、标记并访问某个目录
  - 远程挂载（需要 `sshfs`、`rclone`）
  - 熟悉的快捷键（方向键、<kbd>~</kbd>、<kbd>-</kbd>、<kbd>@</kbd>）、快速查找
  - 退出时执行 `cd`（*轻松*集成到 shell）
  - 打开文件并完成选择后继续到下一个文件
- 搜索
  - 即时过滤，支持*边输入边搜索*
  - 模糊、正则（POSIX/PCRE2）与字符串（默认）过滤器
  - 用于打开或编辑文件的子树搜索插件
- 排序
  - 默认按有序纯数字名称排序（访问 `/proc`）
  - 不区分大小写的版本（*又称*自然）排序
  - 按名称、访问/更改/修改（默认）时间、大小、扩展名排序
  - 反向排序
  - 目录级别的独立排序
- 类型（Mimes）
  - 内置目录与文本文件预览器
  - 基于 FIFO 的预览器插件，具备丰富的 mime 支持
  - 使用桌面默认程序打开，或指定自定义打开方式
  - 文件专属颜色（或极简的_用上下文颜色标记目录_）
  - 支持图标与 Emoji（自定义并编译启用）
  - 用于图片、视频与音频缩略图的插件
  - 创建、列出、解压（至）、挂载（基于 FUSE）归档
  - 将所有文本文件用 `$EDITOR` 打开的选项
- 便利
  - 详尽的文件统计与 mime 信息
  - 以热键运行插件和自定义命令
  - 符合 FreeDesktop 规范的回收站工具集成
  - 跨目录的文件/全部/范围选择
  - 创建（含父目录）、重命名、对文件与目录生成副本
  - 启动时创建新文件或目录（树）
  - 针对所选内容或目录的批量重命名器
  - 从标准输入或插件读取文件路径列表流
  - 对所选内容执行复制（另存为）、移动（另存为）、删除、归档、创建链接
  - 轻松将系统剪贴板中的路径复制或移动到当前目录
  - 在 `cp`、`mv`、`rm` 完成时更新目录并提示
  - 选择时将文件路径复制到系统剪贴板
  - 启动应用、运行命令、派生 shell、切换可执行位
  - 在提示符或派生的 shell 中访问上下文路径/文件
  - 在可配置的空闲超时后锁定终端
  - 捕获并在帮助界面中显示某个程序的输出
  - 对屏幕阅读器和盲文显示器的基础支持

## 快速上手

### 新手

1. [安装](https://github.com/jarun/nnn/wiki/Usage#installation) `nnn`，建议优先使用包管理器。
2. 运行以下命令以生成（并可选地添加到你的 rc 文件中）一个用于上手的 shell 函数。
   ```sh
   sh -c "$(curl -Ls https://raw.githubusercontent.com/jarun/nnn/master/misc/quickstart.sh)"
   ```

使用你所选择的 shell 函数名，以你偏好的配置运行 `nnn`。

### 高级用户

1. [安装](https://github.com/jarun/nnn/wiki/Usage) `nnn` 以及你需要的依赖。
2. [安装插件](https://github.com/jarun/nnn/tree/master/plugins#installation)。
3. 配置[退出时 cd](https://github.com/jarun/nnn/wiki/Basic-use-cases#configure-cd-on-quit)。
4. 默认使用桌面程序打开。使用 `-e` 在终端内打开文本文件。可选地[分离式打开](https://github.com/jarun/nnn/wiki/Basic-use-cases#detached-text)。
5. 在 CLI 环境中，将 [`NNN_OPENER`](https://github.com/jarun/nnn/wiki/Usage#configuration) 设为 [`nuke`](https://github.com/jarun/nnn/blob/master/plugins/nuke)。使用选项 `-c`。对于 GUI 应用，可以考虑 [xopen](https://github.com/jarun/xopen)。
6. [同步子 shell](https://github.com/jarun/nnn/wiki/Basic-use-cases#sync-subshell-pwd) 将 `$PWD` 同步到 `nnn`。
7. 使用 `-x` 将选择同步到剪贴板、在 `cp`、`mv`、`rm` 时显示提示并设置 xterm 标题。
8. 与 `ls` 说再见！`alias l='nnn -dDe'` :sunglasses:
9. 访问[实时预览](https://github.com/jarun/nnn/wiki/Live-previews)与[故障排查](https://github.com/jarun/nnn/wiki/Troubleshooting) Wiki 页面。

不要死记硬背！方向键、<kbd>/</kbd>、<kbd>q</kbd> 就够用了。<kbd>Tab</kbd> 用于创建和/或循环上下文。<kbd>?</kbd> 列出快捷键。

[![Wiki](https://img.shields.io/badge/RTFM-nnn%20Wiki-important?maxAge=2592000)](https://github.com/jarun/nnn/wiki)

## 视频

[![](https://github.com/user-attachments/assets/e93f7571-8b8d-4703-bef1-93fc804adf7d)](https://www.youtube.com/embed/-knZwdd1ScU)

- [nnn file manager on Termux (Android)](https://www.youtube.com/embed/AbaauM7gUJw)
- [NNN File Manager](https://www.youtube.com/embed/1QXU4XSqXNo)
- [This Week in Linux 114 - TuxDigital](https://www.youtube.com/watch?v=5W9ja0DQjSY&t=2059s)
- [nnn file manager basics - Linux](https://www.youtube.com/embed/il2Fm-KJJfM)
- [I'M GOING TO USE THE NNN FILE BROWSER! 😮](https://www.youtube.com/embed/U2n5aGqou9E)
- [NNN: Is This Terminal File Manager As Good As People Say?](https://www.youtube.com/embed/KuJHo-aO_FA)
- [nnn - A File Manager (By Uoou, again.)](https://www.youtube.com/embed/cnzuzcCPYsk)

## 媒体报道

- [AddictiveTips](https://www.addictivetips.com/ubuntu-linux-tips/navigate-linux-filesystem/)
- [ArchWiki](https://wiki.archlinux.org/index.php/Nnn)
- [FOSSMint](https://www.fossmint.com/nnn-linux-terminal-file-browser/)
- [gHacks Tech News](https://www.ghacks.net/2019-11-01/nnn-is-an-excellent-command-line-based-file-manager-for-linux-macos-and-bsds/)
- Hacker News [[1](https://news.ycombinator.com/item?id=18520898)] [[2](https://news.ycombinator.com/item?id=19850656)]
- [It's FOSS](https://itsfoss.com/nnn-file-browser-linux/)
- [Linux Format Issue 265; Manage files with nnn](https://linuxformat.com/archives?issue=265)
- LinuxLinks [[1](https://www.linuxlinks.com/nnn-fast-and-flexible-file-manager/)] [[2](https://www.linuxlinks.com/bestconsolefilemanagers/)] [[3](https://www.linuxlinks.com/excellent-system-tools-nnn-portable-terminal-file-manager/)]
- [Linux Magazine; FOSSPicks](https://www.linux-magazine.com/Issues/2017/205/FOSSPicks/(offset)/15)
- [Make Tech Easier](https://www.maketecheasier.com/nnn-file-manager-terminal/)
- [Opensource.com](https://opensource.com/article/22/12/linux-file-manager-nnn)
- [Open Source For You](https://www.opensourceforu.com/2019/12/nnn-this-feature-rich-terminal-file-manager-will-enhance-your-productivity/)
- [PCLinuxOS Magazine Issue June 2021](https://pclosmag.com/html/Issues/202106/page08.html)
- [Suckless Rocks](https://suckless.org/rocks/)
- [Ubuntu Full Circle Magazine Issue 135; Review: nnn](https://fullcirclemagazine.org/issue-135/)
- [Using and Administering Linux: Volume 2: Zero to SysAdmin: Advanced Topics](https://books.google.com/books?id=MqjDDwAAQBAJ&pg=PA32)
- [Wikipedia](https://en.wikipedia.org/wiki/Nnn_(file_manager))

## 开发者

- [Arun Prakash Jana](https://github.com/jarun) (Copyright © 2016-2026)
- [0xACE](https://github.com/0xACE)
- [Anna Arad](https://github.com/annagrram)
- [KlzXS](https://github.com/KlzXS)
- [Léo Villeveygoux](https://github.com/leovilok)
- [Luuk van Baal](https://github.com/luukvbaal)
- [NRK](https://codeberg.org/NRK)
- [Sijmen J. Mulder](https://github.com/sjmulder)
- 以及其他贡献者

前往 [Tracker](https://github.com/jarun/nnn/issues/1546) 话题帖，查看进行中的功能以及一切待认领的项目。欢迎[讨论](https://github.com/jarun/nnn/discussions)新想法或功能增强请求。
