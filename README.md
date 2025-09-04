# Xiao's Ziphacker

## 功能介绍
加密Zip文件的暴力破解工具，目前已基本可用。

## 使用方法

### 命令行程序
1. 下载release中的zip文件并解压
2. cmd进入程序目录
3. 运行指令`XiaosZiphacker.exe [你的参数]`（参数相关信息可以通过`-h`查看）

示例：`XiaosZiphacker.exe -n -u -l -t "D:/zipneedhack.zip" -r 1-6`

说明：
- `-n`：将数字0~9加入字符集
- `-u`：将大写字母A~Z加入字符集
- `-l`：将小写字母a~z加入字符集
- `-t`：指定下一个参数为目标zip文件路径
- `"D:/zipneedhack.zip"`：目标zip文件路径
- `-r`：指定下一个（或两个）参数为尝试的密码长度范围
- `1-6`：密码长度范围为1~6（等价于`1 6`）

## 自行构建（VS2022）
1. git clone或者下载源代码
2. 使用VS2022打开文件夹
3. 选择release配置进行生成
4. 进入解决方案资源管理器的CMake目标视图
5. 右键CreatePackage，选择生成
6. 打包好的程序会位于`./Xiao-s-Ziphackerbuild/release/package`

## 已知异常用例
1. 通过特殊编码可被识别为`.mp4`文件的`.zip`文件
2. 以仅存储方式压缩的无法特异性识别其类型的文件