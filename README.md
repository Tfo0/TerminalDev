# TerminalDev

基于 Windows Terminal 源码的自定义构建（Dev 分支，`WindowsTerminalDev` 身份，可与商店版共存）。

## 特性

- **类似 Edge 的竖状标签栏**：新增窗口级主题属性 `tabPosition`（`top` / `bottom` / `left` / `right`），设为 `left` 即可让标签栏像 Edge 垂直标签页一样竖排显示在窗口左侧
- **配置文件分层结构**：沿用 WT 的分层配置体系（`defaults.json` → `settings.json` → `profiles.defaults` → 各 profile）；`tabPosition` 位于 theme 层，随主题整体切换，不污染全局设置

## 启用竖状标签栏

在 `settings.json` 中定义一个 theme 并启用：

```json
"themes": [
    {
        "name": "vertical",
        "window": { "tabPosition": "left" }
    }
],
"theme": "vertical"
```

想恢复顶部标签栏，把 `theme` 换回默认主题即可。

## 安装方式 A：免管理员（无管理员权限且域策略未禁用旁加载的机器）

适用于没有管理员权限、但域策略未限制旁加载的普通用户。

1. 下载 `WindowsTerminalDev-0.0.1.9-x64-noadmin.zip` 并解压
2. 在 PowerShell 中执行：

```powershell
powershell -ExecutionPolicy Bypass -File .\Install-NoAdmin.ps1
```

脚本会把程序复制到 `%LOCALAPPDATA%\Programs\TerminalDev` 并为当前用户注册（散包注册，**不需要证书、不需要管理员权限**）。

注意：
- 安装后**不要删除或移动** `%LOCALAPPDATA%\Programs\TerminalDev` 文件夹，程序从该位置运行
- 若报错 `0x80073CFF`（需要开发者许可证或启用旁加载的系统），说明域策略禁用了旁加载 → 见下方"域策略受限机器"
- 卸载：运行 `Uninstall.ps1`

## 安装方式 B：MSIX 安装包（有管理员权限的机器）

要求：Windows 10 1809 (17763) 及以上，x64。

1. 下载 `WindowsTerminalDev-0.0.1.9-x64.zip` 并解压
2. 右键 `Install.ps1` → 使用 PowerShell 运行（需管理员权限，用于信任签名证书）
   - 或在管理员 PowerShell 中执行：`powershell -ExecutionPolicy Bypass -File Install.ps1`
3. 脚本会自动完成：信任签名证书 → 安装 WinUI 依赖 → 安装 MSIX

手动安装：

```powershell
# 管理员 PowerShell
Import-Certificate -FilePath .\TerminalDev.cer -CertStoreLocation Cert:\LocalMachine\TrustedPeople
Add-AppxPackage .\Microsoft.UI.Xaml.2.8.appx
Add-AppxPackage .\WindowsTerminalDev_0.0.1.9_x64.msixbundle
```

卸载：

```powershell
Get-AppxPackage WindowsTerminalDev | Remove-AppxPackage
```

## 域策略受限机器（报 0x80073CFF / 无管理员权限）

散包注册和证书信任都被锁时，本机用户态无解，需要域管理员做一次 GPO 配置（全公司机器一次到位，之后普通用户即可自行安装）：

1. **信任签名证书**：组策略 → 计算机配置 → Windows 设置 → 安全设置 → 公钥策略 → 「受信任的根证书颁发机构」（或「受信任人」）→ 导入 Release 包中的 `TerminalDev.cer`
2. **允许受信任应用安装**：组策略 → 计算机配置 → 管理模板 → Windows 组件 → 应用包部署 → 「允许安装所有受信任的应用」设为"已启用"（或「允许开发 Windows 应用商店应用并从集成开发环境(IDE)安装这些应用」启用旁加载）
3. 策略下发后，普通用户无需管理员即可双击安装 `WindowsTerminalDev_0.0.1.9_x64.msixbundle`，或运行方式 B 的 `Install.ps1`（去掉管理员需求）

安装完成后在开始菜单找到 **Windows Terminal (Dev Build)**。
