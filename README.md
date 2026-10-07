# BreakoutGame（ブロック崩し）

ND1 評価課題01：Unreal Engine 5.8 で作ったブロック崩しです。

- クラス・名前：ND1_LE4B_05 ウノ リュウト
- エンジン：Unreal Engine 5.8（Windows 64bit）
- ロジックはすべて C++（`Source/BreakoutGame`）。配置・見た目・アセットの設定は Blueprint の子クラスとレベルで持たせています
- GitHub：`ryuto-alt/BreakoutGame`（ブランチ `評価課題01`、同じ内容を `main` にも反映）

## 起動方法

提出 zip を展開し、`Windows/BreakoutGame.exe` を起動します（ウィンドウ 1280x720）。タイトル画面で Space を押すと始まります。

## 操作

| キー | 動作 |
|---|---|
| A / D | パドルを左右に移動 |
| Space | ボール発射 ／ クリア後は次のステージへ ／ ゲームオーバー後はやり直し ／ タイトルでゲーム開始 |

## 遊び方

- ボールを打ち返してブロックをすべて壊すとクリアです。クリア後に Space で次のステージへ進みます。3 ステージ全部クリアすると「ALL CLEAR!!」が出て、Space でタイトルに戻ります
- 残りボールは最初 3 個（左上の `LeftBall`）。すべて落とすとゲームオーバーです
- ブロックの数字は耐久値（Hp）。当たるたびに減り、色も変わります（緑 1 ／ 黄 2 ／ 橙 3 ／ 赤 4 ／ 紫 5）
- 灰色の数字なしブロックは壊れません（クリアには数えません）
- ブロックを壊すと、ときどきアイテムが落ちてきます。パドルで受けると効果が出ます

| アイテム | 効果 |
|---|---|
| **A**（ピンク） | ボールが 1 個増える |
| **S**（水色） | 場にあるボールがそれぞれ 1 個ずつ、左右反転した向きに分裂する |
| **P**（オレンジ） | 約 5 秒間、ボールがオレンジ色になってブロックを貫通する（通るたびにダメージ） |

- パドルのどこで受けるかで返る角度が変わります。端で受けるほど斜めに返ります（狙い撃ち）

## 実装した評価項目

| # | 評価項目 | 実装した場所 |
|---|---|---|
| 1 | 提出規則 | `ND1_LE4B_05_ウノ_リュウト_Test01.zip` に、パッケージ化したゲーム一式と、この内容の `readme.md` を入れて提出 |
| 2 | GameClear / GameOver | `ABreakoutGameManager`（`AddBrokenBlockNum` で全ブロック破壊 → `ViewClearWidget`、`MissArea` に触れて残りなし → `ViewGameOverWidget`）。表示は `UBreakoutClearWidget`（「GameClear!!」が上下にふわっと 3 回）と `UBreakoutGameOverWidget`（「GameOver!」）。クリア後に落ちたボールではゲームオーバーにならない |
| 3 | Space でリセット / レベル遷移 | パドルが `IA_Action`（Space、`Started`）を受けて `GameManager->Action()`。ゲームオーバー中は `LevelReset`（今のレベルを読み直し）、クリア済みなら `OpenNextLevel`（`NextLevelName`）、それ以外はボール発射。Level1 → Level2 → Level3 → TitleLevel |
| 4 | GameOver の表示アニメーション | `UBreakoutGameOverWidget`：文字色が白 → 赤（2 秒）。さらに大きく出て縮むポップ、左右の揺れ、「Push SPACE to Restart」の点滅 |
| 5 | ブロックの Hp 数字 | `ABreakoutBlock::HpText`（`UTextRenderComponent`、カメラ側の面に表示）。`Hp` はレベル上のインスタンスごとに設定できる |
| 6 | Hp による色の変化 | `ABreakoutBlock::ColorTable`（Hp 1〜5）と `BlockMaterial`（VectorParameter `BaseColor`）の動的マテリアル。`ReloadHp` で数字と色を更新 |
| 7 | BGM と SE | BGM：`Breakout_BGM`（ループ、`GameManager` の BeginPlay で再生）。SE：ボールが当たるたびに `Breakout_SE_Knock`（ピッチを少し変える）。ほかにブロック破壊・アイテム取得・クリア・ゲームオーバーの音 |
| 8 | パドル位置による反射 | `ABreakoutPaddle::GetTopNormal`（天面なら当たった位置で法線を最大 `MaxTiltNormalDeg` = 20 度傾ける）と `ABreakoutBall::ClampDirection`（水平すぎる向きを防ぐ、`MinHorizontalAngleDeg` = 20 度） |
| 9 | ボールを増やすアイテム | `ABreakoutAddBallItem`（BP `AddBallItem`）の A。ブロックが壊れたとき `ItemDropRate`（0.3）の確率で落ち、パドルで受けると `GameManager->GenerateBall()` |
| 10 | ボールの持ち越し | `UBreakoutGameInstance::LeftBallNum`。`OpenNextLevel` で `LeftBallNum + InGameBallNum`（場のボールも含む）を入れ、次のレベルの `GameManager` が BeginPlay で受け取る。スコアも一緒に持ち越す |
| 11 | 創意工夫 | 下の「アレンジした箇所」を参照 |

## アレンジした箇所（創意工夫）

**ゲームの中身**
- **壊れないブロック**（Unbreakable）：灰色で数字なし。壊れず、クリア条件にも数えない。Level2 と Level3 で使用
- **分裂アイテム S**：場にあるボールすべてが、左右反転した向きにもう 1 個ずつ増える（上限 10 個）
- **貫通アイテム P**：約 5 秒間、ボールがオレンジ色になりブロックをすり抜けながらダメージを与える（壁とパドルでは跳ね返る）。時間切れでブロックの中に埋まらないよう、重なっている間は貫通を続ける
- **アイテムをランダムに**：ブロックを壊すと A / S / P のどれかがランダムに落ちる
- **複数ステージ**：Level1（かんたん）／ Level2（壊れないブロックあり）／ Level3（山形の配置で Hp 最大 5、壊れないブロックあり）。クリア後は次のステージへ、最後は「ALL CLEAR!!」からタイトルへ戻る
- **ステージが進むほどボールが速い**：1000 → 1150 → 1300
- **スコア**：ブロックに当たると +10、壊すと +100、アイテム取得で +50。ステージをまたいで持ち越す
- **タイトル画面**：「BREAKOUT」と点滅する「Push SPACE」。Space でゲーム開始

**見た目・演出**
- ネオン調のデザイン：暗い背景、水色の壁、明るい色の Unlit マテリアル（ライトに左右されない）
- 画面表示：左上に `LeftBall`、上中央に `STAGE`、右上に `SCORE`
- ブロックを壊すと、色つきの小さなかけらが飛び散るパーティクル（C++ の `ABreakoutDebris`）。壊したとき・クリア・ゲームオーバーでカメラが揺れる
- ボールのヒット音はピッチが毎回少し変わる。ブロック破壊・アイテム取得・クリア・ゲームオーバーにも別の効果音（`Tools/gen_sounds.py` で自作）
- クリア画面：「GameClear!!」が 3 回ふわっと動き、下に「Push SPACE to Next Stage」（最終ステージは「ALL CLEAR!!」と「Push SPACE to Title」）
- GameOver 画面：ポップ・揺れ・白から赤へ・Push SPACE の点滅

## 技術メモ

- C++ 17、UE 5.8、Enhanced Input。ウィジェットは Widget Blueprint を使わず、すべて C++（`RebuildWidget` と `NativeTick`）で作っている
- 移動はすべて Tick での Sweep 移動（物理は使わない）。Ball は `AddActorWorldOffset(..., Sweep=true, &Hit)` で当たりを取り、`Bounce` の中で Block・Paddle を直接呼ぶ（Hit イベントに頼らない）
- 主なクラス：`ABreakoutPaddle` / `ABreakoutBall` / `ABreakoutBlock` / `ABreakoutGameManager` / `AMissArea` / `ABreakoutAddBallItem` / `ABreakoutDebris` / `UBreakoutGameInstance` / `ABreakoutTitleManager` と、`UBreakout*Widget`（Clear / GameOver / GameInfo / Title）。スライドの名前の Blueprint（`Paddle` `Ball` `Block` `GameManager` `AddBallItem` `BreakoutGame` `BreakoutGameInstance`）はこれらの子クラス
- コリジョン：オブジェクトチャンネル `Item` / `Ball` / `MissArea` とプリセットを `Config/DefaultEngine.ini` に定義。ボール同士・ボールとアイテムはぶつからない。ブロックは `WorldDynamic`、壁は `WorldStatic` にして、貫通ボールがブロックだけをすり抜けるようにしている
- スライドとの違い：レベルブループリントの代わりに、GameManager の BeginPlay でウィジェットを作る／タイトルは `ABreakoutTitleManager` が Space を受ける。BGM は SoundCue ではなく、ループ設定した wav を `SpawnSound2D` で再生。アイテムの落下率はスライドの 0.5 ではなく 0.3
- `Tools/gen_content.py`：エディタの Python で Blueprint・マテリアル・サウンド取り込み・入力アセット・全レベルを作り直す（`Tools/run_python.ps1` で実行）。ステージの配置は先頭の `STAGES`（数字 = Hp、`#` = 壊れないブロック）
- `Tools/gen_sounds.py`：標準ライブラリだけで BGM と効果音の wav を生成（`py -I Tools/gen_sounds.py`、出力は `Tools/SourceAudio`）
- 録画用の起動オプション（配布版では使わない）：`-autoplay`（パドルの自動操作）、`-automiss`（わざとミスする）、`-autoseed=N`（自動操作とアイテム抽選の乱数を固定）、`-startballs=N`（最初の残りボール数）、`-uiframes`（UI も写る連番画像を保存）、`-debugnormals`（パドル天面の法線の矢印）。`Tools/record.ps1` と `Tools/mkgif.sh` で GIF にしている
- パッケージ化：`Config/DefaultGame.ini` に Shipping・配布用・フルリビルド・エディタコンテンツ除外・クックするマップ（TitleLevel / Level1〜3）を設定。`Tools/package.ps1` が `RunUAT BuildCookRun` を呼ぶ
- ビルド（エディタ）：`build.ps1`。PowerShell のスクリプトは PowerShell 7（`pwsh`）で実行する

## 開発の流れ（ブランチ）

`01_01`（Paddle と Ball）→ `01_02`（Block・GameManager・クリア/ゲームオーバー）→ `01_03`（リスポーン・残ボール・リセット）→ `01_04`（Hp・色・音・次のレベル・持ち越し）→ `01_05`（狙い撃ち）→ `01_06`（アイテム・コリジョン）→ `01_07`（タイトル）→ `評価課題01`（仕上げ・アレンジ・パッケージ化）。
