# TerminalDev

Windows Terminal 自定义构建（Dev 分支，`WindowsTerminalDev` 身份，可与商店版共存）。

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