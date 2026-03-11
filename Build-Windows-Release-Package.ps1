Param(
    [parameter(Mandatory = $true)][string]$Version
)

# 1. MSBuildの準備 (Actions環境なら何もしない)
if (-not (Get-Command "MSBuild.exe" -ErrorAction SilentlyContinue)) {
    $msbuild_path = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin'
    $Env:Path = $msbuild_path + ";" + $Env:Path
}

function BuildPackage ($package_name, $package_unique_files, $build_conf) {
    Write-Host "--- Starting Build: $package_name ($build_conf) ---" -ForegroundColor Cyan

    # 2. ビルド実行
    # PlatformはWin32で成功しているため維持
    MSBuild.exe .\VisualStudio\Hengband.sln /t:Rebuild /p:Configuration=$build_conf /p:Platform=Win32
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed for $build_conf"; exit 1
    }

    # 3. 作業用フォルダの作成
    $tempDir = New-Item -ItemType Directory -Path (Join-Path $env:TEMP ([Guid]::NewGuid().ToString()))
    $destDir = New-Item -ItemType Directory -Path (Join-Path $tempDir $package_name)

    # 4. ファイルのコピー (堅牢な方法)
    # EXEはルートにあることがログで判明しているため、リネームしてコピー
    if (Test-Path ".\Hengband.exe") {
        Copy-Item -Path ".\Hengband.exe" -Destination (Join-Path $destDir "tangband.exe")
    }
    
    # PDBはVisualStudioフォルダ内にある可能性が高い
    $pdbPath = ".\VisualStudio\Hengband\Release\Hengband.pdb"
    if (Test-Path $pdbPath) {
        Copy-Item -Path $pdbPath -Destination (Join-Path $destDir "tangband.pdb")
    }

    # ルートのドキュメント類
    foreach ($f in @(".\readme_angband", ".\THIRD-PARTY-NOTICES.txt")) {
        if (Test-Path $f) { Copy-Item $f -Destination $destDir }
    }
    foreach ($f in $package_unique_files) {
        if (Test-Path $f) { Copy-Item $f -Destination $destDir }
    }

    # 5. libフォルダのコピー
    if (Test-Path ".\lib") {
        # libフォルダそのものをコピー
        Copy-Item -Path ".\lib" -Destination $destDir -Recurse -Exclude Makefile.am, *.raw, .gitattributes
    }

    # 6. 不要ファイルの削除 (エラーが出ても無視する設定)
    # -ErrorAction SilentlyContinue を付けることで、フォルダがなくても止まらないようにします
    $cleanupPaths = @(
        "$destDir\lib\save\*",
        "$destDir\lib\user\*",
        "$destDir\lib\xtra\music\*"
    )
    foreach ($path in $cleanupPaths) {
        if (Test-Path $path) {
            Remove-Item -Path $path -Recurse -Force -Exclude delete.me, music.cfg, readme.txt -ErrorAction SilentlyContinue
        }
    }

    # 7. Zip作成
    $zipPath = Join-Path (Get-Location) "${package_name}.zip"
    if (Test-Path $zipPath) { Remove-Item $zipPath }
    
    Write-Host "Creating Zip: $zipPath"
    Compress-Archive -Path "$destDir" -DestinationPath $zipPath -Force

    # 8. 後片付け
    Remove-Item -Path $tempDir -Recurse -Force
}

# 実行
BuildPackage -package_name "Tangband-$Version-jp" -package_unique_files @(".\readme.md", ".\autopick.txt") -build_conf "Release"
BuildPackage -package_name "Tangband-$Version-en" -package_unique_files @(".\readme-eng.md", ".\autopick_eng.txt") -build_conf "English-Release"
