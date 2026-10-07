# BreakoutGame

ND1 ブロック崩し（Unreal Engine 5.8）。

- ロジックは C++（`Source/BreakoutGame`）、配置や見た目は Blueprint 子クラス（`Content/Blueprints`）とレベル（`Content/Maps`）。
- `Tools/gen_content.py` はエディタの Python で Blueprint・入力アセット・レベルを作り直すスクリプト。
- `Tools/record.ps1` は録画用。起動オプション `-autoplay` を付けるとパドルが自動でボールを追う（デモ録画用）。

## 操作

| キー | 動作 |
|---|---|
| A / D | パドルを左右に移動 |
| Space | ボール発射（ゲームオーバー後はやり直し） |

## 01_01 PaddleとBallの作成

- Level1（ライト・空・フォグ・外壁4枚・カメラ・PlayerStart）
- GameMode `BreakoutGame`（DefaultPawnClass = Paddle）
- Paddle：Enhanced Input（IA_Move / IMC_InGame）で左右移動、Sweep で壁にめり込まない
- Ball：斜めに飛び、壁とパドルで反射（MirrorVectorByNormal）

## 01_02 Block・GameManager・クリア/ゲームオーバー

- `ABreakoutBlock`（BP `Block`）：ボールが当たると消える。BeginPlay で GameManager に自分を登録（AddBlockNum）
- `ABreakoutGameManager`（BP `GameManager`、レベルの (0,0,0) に1つ配置）：BlockNum / BrokenBlockNum / bIsCleared を管理。全ブロックを壊すと `ViewClearWidget`
- `AMissArea`（(0,0,120)、BoxExtent 40,1500,40、タグ `MissArea`）：ボールが触れると `MissCount` → `GameOver!`。クリア後は出さない
- ウィジェットは Widget Blueprint を使わず C++ で作成（`RebuildWidget` で CanvasPanel + TextBlock を組み立て）
  - `UBreakoutClearWidget`：「GameClear!!」。FloatAnimation は NativeTick で Y を 0→-50→0（2秒）× 3回
  - `UBreakoutGameOverWidget`：「GameOver!」
  - ウィンドウが 960x540 のためスライドのフォントサイズ（200/180）ではなく 110/100 にしている
- PrintString の代わりに `UE_LOG` を使用
- 録画：`-automiss` を付けるとパドルがボールの着地点から逃げてミスを再現。`-uiframes`（GameMode）は UMG が写る録画用で、`record.ps1 -Capture "-uiframes"` で使う（`-dumpmovie` は UI を写さない）
- スクリプトは PowerShell 7（`pwsh -File ...`）で実行すること（Windows PowerShell 5.1 だと日本語コメントで param が壊れる）

## 01_03 リスポーン・残ボール・リセット

- ボールはレベルに置かず、GameManager が `SpawnBall` で生成（`RespawnLocationActor`＝TargetPoint (0,0,1000) の位置。`SpawnLocationActor` にインスタンスで指定）
- 同時に1個まで（`bIsBallSpawned`）、残り `LeftBallNum`（既定3）。発射のたびに1減る
- Space（`IA_Action`、`IMC_InGame` に追加）を Paddle が `Started` で受けて `GameManager->Action()`。ゲームオーバーなら `LevelReset`（現在のレベルを再読込）、それ以外は `SpawnBall`
- 残り0でミスすると GameOver（白→赤を2秒）と「Push SPACE to Restart」を表示
- 左上の「LeftBall : n」は `UBreakoutGameInfoWidget`。スライドではレベルブループリントの BeginPlay で作っていたが、C++ ではレベルBPを使わず GameManager の BeginPlay で作成。Text のバインドは NativeTick で毎フレーム更新して同等にしている
- 録画：`-automiss` でボールを3回ミス → ゲームオーバー → 自動で Space を押して再開

## 01_04 耐久値・色・音・次レベル・ボール持ち越し

- Block に `Hp`（レベル上で個別に設定）。当たるたびに -1 して `ReloadHp`、0 以下で壊れる
  - `UTextRenderComponent`（黒、WorldSize 150、(-51,0,0)・Yaw 180）で残り Hp を表示。Cube はスケールされているので、文字は Root の子にしている
  - `BlockMaterial`（VectorParameter `BaseColor`）を `OnConstruction` で動的マテリアルにし、`ColorTable[Hp-1]`（緑・黄・橙・赤・紫の5色）を設定
- SE / BGM：`Tools/gen_sounds.py`（標準ライブラリだけ）で `Tools/SourceAudio/Breakout_SE_Knock.wav` / `Breakout_BGM.wav` を生成し、`gen_content.py` が `/Game/Sounds` にインポート（BGM は Looping）
  - Ball は反射のたびに `PlaySound2D`。BGM は SoundCue をレベルに置く代わりに GameManager の BeginPlay で `SpawnSound2D`
  - 元のスライドは ogg + SoundCue だが、ここでは生成した wav を直接使っている
- `NextLevelName`（GameManager のインスタンスで指定）。`Action` は ゲームオーバー → やり直し / クリア済み → `OpenNextLevel` / それ以外 → `SpawnBall`
- `UBreakoutGameInstance`（BP `BreakoutGameInstance`、`DefaultEngine.ini` の `GameInstanceClass` に設定）：`LeftBallNum`（既定 -1）と `IsValidBallNum`。`OpenNextLevel` で残りボールを入れ、次レベルの GameManager が BeginPlay で受け取る
  - スライドは `LeftBallNum + 1`（場のボール分）だが、場にボールがなければ足さないようにしている
  - ゲームオーバーからのやり直しは持ち越しを -1 に戻し、レベルの既定値から始める
- Level1（Hp 1〜3 の3個、次は Level2）、Level2（Hp 2〜5 の7個、次は Level1）。配置は `gen_content.py` の `LAYOUT_LEVEL1/2`
- 録画用オプション：`-startballs=N`（最初のレベルの残りボール数）、`-autoseed=N`（自動プレイの乱数を固定）。クリア後は自動プレイが約2秒後に Space を押して次のレベルへ進む

## 01_05 狙い撃ち

- パドル天面の当たった位置で反射用の法線を傾ける
  - `ABreakoutPaddle::GetTopNormal(HitLocation, Normal, bIsHitTopSurface, OutNormal)`：Normal.Z > 0.7 を天面とみなし、`GetActorBounds` の Origin / BoxExtent から t = -1（左端）〜+1（右端）を出して X 軸まわりに `t * MaxTiltNormalDeg`（既定20度）傾ける。右端ほど +Y 側に傾け、右で受けると右へ返る
  - 天面以外（側面など）は従来どおり衝突法線で反射
- `ABreakoutBall`：コンポーネントタグ `Player` のパドルの天面に当たったときだけ、傾けた法線で反射 → `ClampDirection`（上向きを 0 度とした atan2 を `±(90 - MinHorizontalAngleDeg)` に制限、既定20度）。それ以外は通常の反射。SE は両方で鳴る
- デバッグ：起動オプション `-debugnormals` で天面の法線を青い矢印21本で描画（既定はオフ。スライドでは確認後に Tick から外す）
- 録画：自動プレイは受ける位置を 左端 → 中央 → 右端 と順番に変えて返す角度の違いを見せる

## 01_06 アイテム（ボール追加）・コリジョン整理

- `Config/DefaultEngine.ini` の `[/Script/Engine.CollisionProfile]` にオブジェクトチャンネル `Item`（既定 Ignore）/ `Ball`（Block）/ `MissArea`（Ignore）とプリセット `Item` / `Ball` / `MissArea` を追加し、`Pawn` は Item に Overlap（`EditProfiles`）。Ball は Ball と Item を無視するので、ボール同士はぶつからない
  - 適用：MissArea ＝ `MissArea`、Paddle の Cube ＝ `Pawn`、Ball の Sphere ＝ `Ball`、アイテム ＝ `Item`
- GameManager：`bIsBallSpawned` を `InGameBallNum`（場のボール数）に変更。`SpawnBall` は `InGameBallNum == 0 && LeftBallNum > 0`、`GenerateBall` は無条件で1個増やす（`LeftBallNum` を消費しない）。`MissCount` はクリア前だけ `InGameBallNum--` し、`LeftBallNum <= 0 && InGameBallNum <= 0` でゲームオーバー。`OpenNextLevel` は `LeftBallNum + InGameBallNum` を持ち越す
- `ABreakoutAddBallItem`（BP `AddBallItem`）：Sphere が Root（タグ `Item`）、手前に「A」の TextRender。`Speed`（既定600）で真下に落下し、`MissArea` タグで消え、`Player` タグ（パドル）に触れると `GenerateBall` して消える。色は BlockMaterial の動的マテリアル
- ブロックは壊れたとき `ItemDropRate`（既定 0.3、インスタンスで変更可）の確率でアイテムを落とす。スライドの数値は Weight 0.5（本文は10%）で食い違っているため、既定値は 0.3 にしている
- アイテムは見やすいよう Sphere のスケールを 1.2 にしている（ボールより少し大きい）
- 録画：自動プレイは、どのボールよりも低い位置にアイテムがあれば受けに行く。ブロックの落とす乱数は `-autoseed=N` で固定される
- `gen_content.py`：`BreakoutGameInstance` は `DefaultEngine.ini` で起動時に読まれるため、既にあれば作り直さない

## 01_07 タイトル

- `/Game/Maps/TitleLevel`：他のレベルと同じライト・空・外壁・カメラに、背景のブロックを並べたタイトル。GameMode は `ABreakoutTitleGameMode`（パドルを出さない）
- `ABreakoutTitleManager`（レベルに1つ配置）：`UBreakoutTitleWidget` を作って表示し、Space（`IA_Action`、`Started`）で `NextLevelName`（既定 `Level1`）を開く。スライドではレベルブループリントの BeginPlay（CreateWidget）と SpaceBar キーイベントだった部分を C++ のアクタに置き換えている。新しいゲームの開始なので持ち越しボール数は -1 に戻す
- `UBreakoutTitleWidget`：「BREAKOUT」（ふわふわ上下）と、点滅する「Push SPACE」。標準フォントは日本語を含まないので英語表記にしている
- 01_08（パッケージ化）の準備：TitleManager の BeginPlay で `UGameUserSettings` を Windowed 1280x720 にして `ApplySettings`。エディタ起動と、録画（`-dumpmovie` / `-uiframes` / `-benchmark`）では行わず、録画は 960x540 のまま
- `DefaultEngine.ini`：`GameDefaultMap` を `TitleLevel` に変更（`EditorStartupMap` は Level1 のまま）
- Level2 のクリア後は `TitleLevel` へ戻る
- 録画：自動プレイはタイトルで約2秒後に Space を押す
