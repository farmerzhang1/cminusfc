# 一些 tips
## 如何 debug
1. 安装cmake tools
2. 选择下方一栏的target
3. 如果需要设置命令行参数，在[workspace的设置](.vscode/settings.json)中加入一行
    ```json
    "cmake.debugConfig": {
        "args": ["arg1", "arg2"]
    }
    ```
4. 设置断点，点击下方小虫子，launch debugger
## 观察修改了啥
打开侧边栏，展开最底下的timeline
