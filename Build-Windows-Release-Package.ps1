Param(
    # パッケージに付加するバージョン
    [parameter(Mandatory = $true)][string]$Version
)

# GitHub Actions環境では MSBuild へのパスは自動で通るため、
# ローカル実行時のみパスを通すようにガードをかけます
if (-not (Get-Command "MSBuild.exe" -ErrorAction SilentlyContinue)) {
    $msbuild_path = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin'
    $Env:Path = $msbuild_path + ";" + $Env:Path
}

function BuildPackage ($package_name, $package_unique_files, $build_conf) {
    Write-Host "Building $build_conf..." -ForegroundColor Cyan
    
    # バイナリをリビルド
    MSBuild.exe .\VisualStudio\Hengband.sln /t:Rebuild /p:Configuration=$build_conf /p:Platform=Win32

    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed for $build_conf"
        exit 1
    }

    # 生成されたバイナリの場所を指定 (VSの標準出力先)
    # プロジェクトの構造に合わせて調整してください
    $outDir = "."
    
    # もし English-Release の場合、出力先フォルダ名が異なる場合があるため補正
    if ($build_conf -eq "English-Release") {
        $outDir = "."
    }

    # 作業用テンポラリフォルダ
    $tempDir = New-TemporaryFile | ForEach-Object { Remove-Item $_; New-Item $_ -ItemType Directory }
    $tangbandDir = Join-Path $tempDir $package_name
    New-Item $tangbandDir -ItemType Directory

# --- 修正箇所 ---
    # 1. ビルドで生成された実際のファイル名（Hengband.exe）を指定
    # 2. コピー先で tangband.exe にリネームする
    
    $exeSource = ".\Hengband.exe"
    if (Test-Path $exeSource) {
        Copy-Item -Verbose -Path $exeSource -Destination "$tangbandDir\tangband.exe"
    } else {
        Write-Error "Hengband.exe not found in root directory!"
        exit 1
    }

    # PDB（デバッグ情報）ファイルも同様に処理（もしあれば）
    $pdbSource = ".\VisualStudio\Hengband\$build_conf\Hengband.pdb"
    if (Test-Path $pdbSource) {
        Copy-Item -Verbose -Path $pdbSource -Destination "$tangbandDir\tangband.pdb"
    }
    # ----------------
    Remove-Item -Verbose -Exclude delete.me -Recurse -Path $tangbandDir\lib\save\*, $tangbandDir\lib\user\*
    Remove-Item -Verbose -Exclude music.cfg, readme.txt, *.mp3 -Path $tangbandDir\lib\xtra\music\*

    # zipアーカイブ作成
    $package_path = Join-Path $(Get-Location) "${package_name}.zip"
    Get-ChildItem -Path $tempDir | Compress-Archive -Force -Verbose -DestinationPath $package_path

    # 作業用テンポラリフォルダ削除
    Remove-Item -Recurse -Force $tempDir
}

# 日本語版 (YAMLの指定に合わせて T を大文字に)
BuildPackage -package_name Tangband-$Version-jp -package_unique_files .\readme.md, .\autopick.txt -build_conf Release
# 英語版
BuildPackage -package_name Tangband-$Version-en -package_unique_files .\readme-eng.md, .\autopick_eng.txt -build_conf English-Release
