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
