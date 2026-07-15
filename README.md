# OpenXR WinUtils

SteamVR のダッシュボードにパネルとして表示される Windows 向けユーティリティオーバーレイです。
OVR Advanced Settings のような常駐オーバーレイアプリとして動作し、最初の機能として
「実行中の OVR Advanced Settings を再起動する」ボタンを備えています。

## 特徴

- SteamVR ダッシュボードにオーバーレイパネルとして表示（通常のウィンドウは開きません）
- Dear ImGui による GUI を D3D11 のオフスクリーンテクスチャに描画し、OpenVR オーバーレイへ提出
- ボタン一つで `AdvancedSettings.exe` を終了させ、`IVRApplications::LaunchApplication` で再起動
- 初回起動時に SteamVR へアプリケーションマニフェストを自己登録

## ビルド方法

### 必要環境

- Windows 10/11 (x64)
- Visual Studio 2022 (Desktop development with C++ ワークロード)
- CMake 3.21 以上
- インターネット接続（OpenVR SDK と Dear ImGui を `FetchContent` で取得します）

### 手順

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

ビルドが成功すると `build/bin/Release/` に以下が生成されます。

- `OpenXRWinUtils.exe`
- `openvr_api.dll`（自動コピー）
- `manifest.vrmanifest`（自動コピー）
- `icon.png`（自動コピー、ダッシュボードのサムネイル用アイコン）

Visual Studio から開く場合は、上記の `cmake -S . -B build -A x64` で生成された
`build/OpenXRWinUtils.sln` を開いてビルドしても構いません。

## インストール・使い方

### 自動登録（推奨）

`OpenXRWinUtils.exe` を一度実行すると、実行ファイルと同じフォルダにある
`manifest.vrmanifest` を SteamVR に自動登録します（`IVRApplications::AddApplicationManifest`）。
SteamVR が起動している状態でビルド成果物一式を任意のフォルダに置いて実行してください。

登録後は SteamVR の「起動時に実行するアプリケーション」設定に
`OpenXR WinUtils` が現れるので、必要に応じて自動起動を有効にしてください。

### 手動登録（vrpathreg）

`vrpathreg` を使って手動でマニフェストを登録することもできます。

```powershell
"%STEAMVR_PATH%\bin\win64\vrpathreg.exe" adddashboardoverlaypath "<インストール先フォルダ>"
```

もしくは SteamVR の `openvrpaths.vrpath` に記載されているアプリケーション設定
ディレクトリへ `manifest.vrmanifest` へのパスを追加しても登録できます。

### 使い方

1. SteamVR を起動し、`OpenXRWinUtils.exe` を実行します（バックグラウンドに常駐し、ウィンドウは表示されません）。
2. SteamVR ダッシュボードを開くと、左側のアプリ一覧に `OpenXR WinUtils` のアイコンが表示されます。
3. アイコンを選択するとオーバーレイパネルが開きます。
4. 「Restart OVR Advanced Settings」ボタンを押すと、実行中の `AdvancedSettings.exe` を終了し、
   少し待ってから OVR Advanced Settings を再起動します。結果（成功/失敗と使用したアプリケーションキー）は
   パネル下部のステータス欄に表示されます。

## 実装メモ

- OpenVR オーバーレイの入力は `IVROverlay::PollNextOverlayEvent` で取得し、`VREvent_MouseMove` /
  `VREvent_MouseButtonDown` / `VREvent_MouseButtonUp` / `VREvent_ScrollDiscrete` を
  Dear ImGui の `AddMousePosEvent` / `AddMouseButtonEvent` / `AddMouseWheelEvent` に変換しています。
- OVR Advanced Settings のアプリケーションキーは起動時に固定せず、`IVRApplications` に登録されている
  全アプリケーションキーを走査して `advsettings` または `1009850`（Steam 版のアプリ ID）を含むものを検索します。
  見つからない場合は `steam.overlay.1009850` → `matzman666.advsettings` の順にフォールバックします。
- ダッシュボードが非表示の間は描画・テクスチャ提出をスキップし、ポーリング間隔を伸ばして CPU 負荷を抑えています。

## ライセンス

このリポジトリは [OpenVR SDK](https://github.com/ValveSoftware/openvr) と
[Dear ImGui](https://github.com/ocornut/imgui) を CMake `FetchContent` で取得して使用します。
それぞれのライセンスに従ってください。
