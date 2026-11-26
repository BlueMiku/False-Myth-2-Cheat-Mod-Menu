# Helpers to drive the FM2 testbed game: dot-source this file.  (testbed only; never the Steam folder)
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;using System.Runtime.InteropServices;
public class FM2Win{
 [DllImport("user32.dll")]public static extern bool GetWindowRect(IntPtr h,out RECT r);
 [DllImport("user32.dll")]public static extern bool PrintWindow(IntPtr h,IntPtr dc,uint f);
 [DllImport("user32.dll")]public static extern bool SetForegroundWindow(IntPtr h);
 [DllImport("user32.dll")]public static extern bool ShowWindow(IntPtr h,int c);
 [DllImport("user32.dll")]public static extern IntPtr GetForegroundWindow();
 [DllImport("user32.dll")]public static extern uint GetWindowThreadProcessId(IntPtr h,out uint pid);
 [DllImport("user32.dll")]public static extern bool AttachThreadInput(uint a,uint b,bool f);
 [DllImport("kernel32.dll")]public static extern uint GetCurrentThreadId();
 [DllImport("user32.dll")]public static extern uint MapVirtualKey(uint c,uint t);
 [DllImport("user32.dll")]public static extern void keybd_event(byte vk,byte sc,uint fl,UIntPtr ex);
 [StructLayout(LayoutKind.Sequential)]public struct RECT{public int L,T,R,B;}
}
"@
$script:TB = "F:\Personal Projjects\FM2\testbed"
$script:SHOTDIR = "C:\Users\Joseph\AppData\Local\Temp\claude\f01b0f5ff8e5b5f0\9ee46d06-fe0d-459f-90da-98fb5cd5841d\scratchpad"
function Start-TestGame { Stop-TestGame; $p = Start-Process "$TB\Game.exe" -WorkingDirectory $TB -PassThru; Start-Sleep 6; $p }
function Stop-TestGame { Get-Process Game -ErrorAction SilentlyContinue | Stop-Process -Force; Start-Sleep 1 }
function Get-GameProc { Get-Process Game -ErrorAction Stop | Select-Object -First 1 }
function Focus-Game {
  $h = (Get-GameProc).MainWindowHandle
  $fg = [FM2Win]::GetForegroundWindow(); $pid2 = 0
  $t1 = [FM2Win]::GetWindowThreadProcessId($fg,[ref]$pid2); $t2 = [FM2Win]::GetCurrentThreadId()
  [FM2Win]::AttachThreadInput($t2,$t1,$true) | Out-Null
  [FM2Win]::ShowWindow($h,9) | Out-Null; [FM2Win]::SetForegroundWindow($h) | Out-Null
  [FM2Win]::AttachThreadInput($t2,$t1,$false) | Out-Null
  Start-Sleep -Milliseconds 300
}
# Press a key by Windows virtual-key code (hold ms)
function Press-Key([int]$vk,[int]$hold=120) {
  $sc = [FM2Win]::MapVirtualKey($vk,0)
  [FM2Win]::keybd_event([byte]$vk,[byte]$sc,0,[UIntPtr]::Zero); Start-Sleep -Milliseconds $hold
  [FM2Win]::keybd_event([byte]$vk,[byte]$sc,2,[UIntPtr]::Zero); Start-Sleep -Milliseconds 150
}
function Shot([string]$name) {
  $h = (Get-GameProc).MainWindowHandle
  $r = New-Object FM2Win+RECT; [FM2Win]::GetWindowRect($h,[ref]$r) | Out-Null
  $w=$r.R-$r.L; $hh=$r.B-$r.T
  $bmp = New-Object Drawing.Bitmap $w,$hh; $g=[Drawing.Graphics]::FromImage($bmp); $dc=$g.GetHdc()
  [FM2Win]::PrintWindow($h,$dc,2) | Out-Null; $g.ReleaseHdc($dc)
  $out = "$SHOTDIR\$name.png"; $bmp.Save($out); $bmp.Dispose(); $out
}
$VK = @{ Enter=0x0D; Esc=0x1B; Z=0x5A; X=0x58; Up=0x26; Down=0x28; Left=0x25; Right=0x27; F9=0x78; F10=0x79; F11=0x7A; F6=0x75; F7=0x76; Space=0x20; Shift=0x10 }
function Deploy-CommonEvent([string]$src) { Stop-TestGame; Copy-Item $src "$TB\Data\BasicData\CommonEvent.dat" -Force }
# boot -> pass prompts -> Continue -> slot 1 -> field map
function Boot-ToMap { Start-TestGame | Out-Null; Focus-Game; foreach($i in 1..5){ Press-Key $VK.Enter; Start-Sleep 4 }; Start-Sleep 8 }
# Extended-key aware press/hold (Insert, arrows, etc. need KEYEVENTF_EXTENDEDKEY=1)
function Key-Down([int]$vk,[bool]$ext=$false){ $sc=[FM2Win]::MapVirtualKey($vk,0); $fl=0; if($ext){$fl=1}; [FM2Win]::keybd_event([byte]$vk,[byte]$sc,$fl,[UIntPtr]::Zero) }
function Key-Up([int]$vk,[bool]$ext=$false){ $sc=[FM2Win]::MapVirtualKey($vk,0); $fl=2; if($ext){$fl=3}; [FM2Win]::keybd_event([byte]$vk,[byte]$sc,$fl,[UIntPtr]::Zero) }
function Tap([int]$vk,[bool]$ext=$false,[int]$hold=200){ Key-Down $vk $ext; Start-Sleep -Milliseconds $hold; Key-Up $vk $ext; Start-Sleep -Milliseconds 250 }
# Hold a key, screenshot the top-left readout, release; returns a Bitmap crop
function Hold-Crop([int]$vk,[bool]$ext=$false,[string]$name="hc"){
  Key-Down $vk $ext; Start-Sleep -Milliseconds 400
  $p=Shot $name; Key-Up $vk $ext; Start-Sleep -Milliseconds 450
  $b=[Drawing.Bitmap]::FromFile($p); $c=$b.Clone([Drawing.Rectangle]::new(0,30,360,50),$b.PixelFormat); $b.Dispose(); $c
}
$VKX = @{ Insert=0x2D; Delete=0x2E; Home=0x24; End=0x23; PgUp=0x21; PgDn=0x22 }
