<#
  check_backend_access.ps1
  Run diagnostics to help find why backend is not reachable from mobile/external network.
  Usage: run in an elevated or normal PowerShell session from project root.
#>

Write-Host "== DashBlocks backend access diagnostics ==`n" -ForegroundColor Cyan

function Run-Section($title, [ScriptBlock]$body) {
    Write-Host "-- $title --" -ForegroundColor Yellow
    try {
        & $body
    }
    catch {
        Write-Warning ("Error running section: $($_)")
    }
    Write-Host "`n"
}

Run-Section "Check docker command" {
    if (Get-Command docker -ErrorAction SilentlyContinue) {
        Write-Host "docker available: $(docker --version)" -ForegroundColor Green
    }
    else {
        Write-Warning "docker CLI not found in PATH"
    }
}

Run-Section "List dashblocks-game container" {
    docker ps --filter "name=dashblocks-game" --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"
}

Run-Section "Inspect port bindings (NetworkSettings.Ports)" {
    try {
        docker inspect dashblocks-game --format '{{json .NetworkSettings.Ports}}'
    }
    catch {
        Write-Warning "docker inspect failed (container may not exist)"
    }
}

Run-Section "Show recent container logs (tail 200)" {
    try { docker logs dashblocks-game --tail 200 } catch { Write-Warning ("docker logs failed: $($_)") }
}

Run-Section "Check listening inside container (ss/netstat)" {
    try {
        docker exec -it dashblocks-game sh -c "ss -ltnp 2>/dev/null || netstat -ltnp 2>/dev/null"
    }
    catch {
        Write-Warning ("docker exec failed or container shell not available: $($_)")
    }
}

Run-Section "Test local HTTP (localhost:5000)" {
    try {
        $r = Invoke-WebRequest -Uri http://localhost:5000/ -UseBasicParsing -TimeoutSec 5 -ErrorAction Stop
        Write-Host "HTTP OK: Status $($r.StatusCode)" -ForegroundColor Green
    }
    catch {
        Write-Warning "Local HTTP request failed: $($_.Exception.Message)"
    }
}

Run-Section "Get host IPv4 addresses" {
    try {
        $ips = Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.IPAddress -ne '127.0.0.1' -and $_.IPAddress -notlike '169.*' } | Select-Object IPAddress, InterfaceAlias, PrefixOrigin
        if (-not $ips) { Write-Warning "No non-loopback IPv4 addresses found via Get-NetIPAddress" } else { $ips | Format-Table -AutoSize }
    }
    catch {
        Write-Warning ("Get-NetIPAddress failed: $($_). Falling back to ipconfig")
        ipconfig | findstr /R "IPv4"
    }
}

Run-Section "Test host IP connectivity on port 5000" {
    try {
        $hostIps = @(Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.IPAddress -ne '127.0.0.1' -and $_.IPAddress -notlike '169.*' } | Select-Object -ExpandProperty IPAddress)
        if (-not $hostIps -or $hostIps.Count -eq 0) { $hostIps = (ipconfig | Select-String 'IPv4' | ForEach-Object { ($_ -split ':')[-1].Trim() }) }
        foreach ($ip in $hostIps) {
            if (-not $ip) { continue }
            Write-Host "Testing $ip:5000 ..."
            try {
                $t = Test-NetConnection -ComputerName $ip -Port 5000 -WarningAction SilentlyContinue
                if ($t.TcpTestSucceeded) { Write-Host "OK -> $ip:5000 reachable" -ForegroundColor Green } else { Write-Warning "$ip:5000 NOT reachable (TcpTestSucceeded=$($t.TcpTestSucceeded))" }
            }
            catch {
                Write-Warning ("Test-NetConnection failed for {0}: {1}" -f $ip, $_)
            }
        }
    }
    catch {
        Write-Warning ("Host IP test failed: $($_)")
    }
}

Run-Section "Firewall quick check (Inbound rules mentioning 5000)" {
    try {
        $rules = Get-NetFirewallRule -Direction Inbound -ErrorAction SilentlyContinue | Where-Object { ($_ | Get-NetFirewallPortFilter -ErrorAction SilentlyContinue).LocalPort -contains '5000' }
        if ($rules) { $rules | Select-Object DisplayName, Enabled, Action | Format-Table -AutoSize } else { Write-Host "No explicit inbound firewall rule for port 5000 found." }
    }
    catch {
        Write-Warning ("Get-NetFirewallRule failed (might require elevated privileges): $($_)")
    }
}

Write-Host "== Diagnostic tips ==" -ForegroundColor Cyan
Write-Host "- If localhost curl works but host IP tests fail: likely Windows firewall or Docker networking issue." 
Write-Host "- If docker inspect shows no 5000/tcp binding, recreate container with -p 5000:5000." 
Write-Host "- For external internet access, set router port forwarding to host:5000 or use ngrok: 'ngrok http 5000'" -ForegroundColor Yellow

Write-Host "Finished diagnostics." -ForegroundColor Cyan
