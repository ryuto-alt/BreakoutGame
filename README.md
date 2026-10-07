# BreakoutGame

ND1 ブロック崩し（Unreal Engine 5.8）。

- ロジックは C++（`Source/BreakoutGame`）、配置や見た目は Blueprint 子クラス（`Content/Blueprints`）とレベル（`Content/Maps`）。
- `Tools/gen_content.py` はエディタの Python で Blueprint・入力アセット・レベルを作り直すスクリプト。
- `Tools/record.ps1` は録画用。起動オプション `-autoplay` を付けるとパドルが自動でボールを追う（デモ録画用）。

## 操作

| キー | 動作 |
|---|---|
| A / D | パドルを左右に移動 |

## 01_01 PaddleとBallの作成

- Level1（ライト・空・フォグ・外壁4枚・カメラ・PlayerStart）
- GameMode `BreakoutGame`（DefaultPawnClass = Paddle）
- Paddle：Enhanced Input（IA_Move / IMC_InGame）で左右移動、Sweep で壁にめり込まない
- Ball：斜めに飛び、壁とパドルで反射（MirrorVectorByNormal）
