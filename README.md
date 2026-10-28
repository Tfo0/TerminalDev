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

## 安装到其他 Windows 机器

要求：Windows 10 2004 (19041) 及以上，x64。

1. 从 [Releases](../../releases) 下载 `WindowsTerminalDev-0.0.1.0-x64.zip` 并解压
2. 右键 `Install.ps1` → 使用 PowerShell 运行（需管理员权限）
   - 或在管理员 PowerShell 中执行：`powershell -ExecutionPolicy Bypass -File Install.ps1`
3. 脚本会自动完成：信任签名证书 → 安装 WinUI 依赖 → 安装 MSIX

安装完成后在开始菜单找到 **Windows Terminal (Dev Build)**。

## 手动安装

```powershell
# 管理员 PowerShell
Import-Certificate -FilePath .\TerminalDev.cer -CertStoreLocation Cert:\LocalMachine\TrustedPeople
Add-AppxPackage .\Microsoft.UI.Xaml.2.8.appx
Add-AppxPackage .\WindowsTerminalDev_0.0.1.0_x64.msix
```

## 卸载

```powershell
Get-AppxPackage WindowsTerminalDev | Remove-AppxPackage
```
