$ws = New-Object -ComObject WScript.Shell
$lnk = $ws.CreateShortcut("C:\Users\Monesy\Desktop\KiCad 10.0.lnk")
Write-Output $lnk.TargetPath
