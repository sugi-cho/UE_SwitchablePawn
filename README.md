# SwitchablePawn

`SwitchablePawn` は、Play 中に FirstPerson / ThirdPerson / VR の Pawn を切り替えるための Unreal Engine Runtime Plugin です。

## 対象

- Unreal Engine 5.7
- C++ プロジェクト推奨
- Enhanced Input 使用
- VR は OpenXR 使用

## UE プロジェクトへの導入

1. UE プロジェクト直下に `Plugins` フォルダを作成します。
2. この `SwitchablePawn` フォルダを `Plugins/SwitchablePawn` に配置します。
3. `.uproject` に Plugin を追加します。

```json
{
  "Name": "SwitchablePawn",
  "Enabled": true
}
```

4. 必要に応じて依存 Plugin も有効化します。

```json
{
  "Name": "EnhancedInput",
  "Enabled": true
},
{
  "Name": "OpenXR",
  "Enabled": true
}
```

5. `.uproject` を右クリックして `Generate Visual Studio project files` を実行します。
6. Editor target をビルドします。

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" YourProjectEditor Win64 Development -Project="C:\Path\To\YourProject\YourProject.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 基本設定

GameMode または World Settings で PlayerController を次に設定します。

- `ASwitchablePlayerController`

Default Pawn は未設定、または Plugin 側の Pawn を指定します。

- `ASwitchableFirstPersonCharacter`
- `ASwitchableThirdPersonCharacter`
- `ASwitchableVRCharacter`

`ASwitchablePlayerController` は開始時に `DefaultMode` の Pawn を生成して Possess します。

## Level 設定

Level に `ASwitchablePawnStart` を配置します。

主な設定:

- `bUseAsDefaultStart`: 起動時の位置として使う
- `PresetPoints`: 名前付きテレポート位置。`TargetActor` に配置済み Actor を指定し、その Actor の Transform を使う

Blueprint / C++ から次の API で移動できます。

```cpp
TeleportToStartPoint("Lobby");
TeleportToStartPointByIndex(0);
```

## 入力

既定では PlayerController が Runtime fallback の Enhanced Input Mapping を作成します。

既定キー:

- `F1`: FirstPerson
- `F2`: ThirdPerson
- `F3`: VR
- `WASD`: 移動
- `Mouse`: Look
- `Space`: Jump
- `RightMouseButton`: VR teleport aim / confirm
- `Gamepad_RightTrigger`: VR teleport confirm

プロジェクト側で独自の `InputMappingContext` / `InputAction` を指定することもできます。

## モデル

ThirdPerson の body mesh、VR の hand mesh は Plugin 内にコピーして使用できます。

差し替え用 property:

- `ASwitchableThirdPersonCharacter::BodyMesh`
- `ASwitchableVRCharacter::HandSkeletalMesh`

最終的な Plugin は、UE Template プロジェクトや Template asset が無い状態でも動作できる必要があります。

## 実装済み機能

- FirstPerson / ThirdPerson / VR の Pawn 切り替え
- 切り替え時の Transform / Velocity / ControlRotation 引き継ぎ
- FirstPerson / ThirdPerson の移動と Jump
- VR の HMD Camera / MotionController / Hand mesh slot
- VR line trace teleport
- NavMesh projection による teleport destination 判定
- `ASwitchablePawnStart` による default start と preset teleport

## 検証

この Plugin の変更後は、最低限 Editor target の C++ build を確認します。

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" UE_SwitchablePawnEditor Win64 Development -Project="C:\Users\git\source\sugi-cho\UE_SwitchablePawn\UE_SwitchablePawn.uproject" -WaitMutex -NoHotReloadFromIDE
```

Shader や Editor load の確認が必要な場合は `UnrealEditor-Cmd.exe` で別途ログ確認します。

## 注意

- Git ルートはこの `SwitchablePawn` フォルダです。
- UE プロジェクト直下の `Source` / `Content` / `Config` はこの Plugin repo の管理対象ではありません。
- `Binaries` / `Intermediate` / `Saved` は Git 管理しません。
