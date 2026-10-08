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
- ステージの最初は「STAGE n → READY → GO!!」。Space を押すと、ボールが 3 個、扇状に飛び出します（残りボールを 1 個使います。左上の `LeftBall`）。すべて落として残りもなくなるとゲームオーバーです
- ブロックの数字は耐久値（Hp）。当たるたびに減り、色も変わります（緑 1 ／ 黄 2 ／ 橙 3 ／ 赤 4 ／ 紫 5）
- 灰色の数字なしブロックは壊れません（クリアには数えません）
- ブロックを壊すとコンボが増えます（右側に COMBO。増えるほど文字が大きくなる）。コンボが 10 増えるごとにスコア倍率が +1。**20 コンボで FEVER**（画面上部に大きな「FEVER!!」、ブロックとボールが虹色に光り、背景が脈打ち、BGM が少し速くなる）。コンボは「場のボールがすべてなくなったとき」か「約 3 秒ブロックを壊さなかったとき」に 0 に戻ります（残り時間に合わせて COMBO の文字が薄くなる。ボールを 1 個落としただけでは続く）。FEVER はコンボが戻ると終わります
- ブロックを壊すと高い確率（60%）でアイテムが落ちてきます。パドルで受けると効果が出ます

| アイテム | 効果 |
|---|---|
| **A**（ピンク） | ボールが 1 個増える |
| **S**（水色） | 場にあるボールがそれぞれ **2 個ずつ**増える（左右反転した向き + 少しずれた向き） |
| **P**（オレンジ） | 約 5 秒間、ボールがオレンジ色になってブロックを貫通する（通るたびにダメージ） |
| **M**（青紫） | パドルの上から **5 個**を扇状に発射する |

- ボールは同時に最大 60 個まで。パドルのどこで受けるかで返る角度が変わります（端で受けるほど斜めに返る、狙い撃ち）

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
| 9 | ボールを増やすアイテム | `ABreakoutAddBallItem`（BP `AddBallItem`）の A。ブロックが壊れたとき `ItemDropRate`（0.6）の確率で落ち、パドルで受けると `GameManager->GenerateBall()` |
| 10 | ボールの持ち越し | `UBreakoutGameInstance::LeftBallNum`。`OpenNextLevel` で `LeftBallNum + InGameBallNum`（場のボールも含む）を入れ、次のレベルの `GameManager` が BeginPlay で受け取る。スコアも一緒に持ち越す |
| 11 | 創意工夫 | 下の「アレンジした箇所」を参照 |

## アレンジした箇所（創意工夫）

**ゲームの中身**
- **壊れないブロック**（Unbreakable）：灰色で数字なし。壊れず、クリア条件にも数えない。Level2 と Level3 で使用
- **アイテム 4 種**：A（ボール追加）／ S（分裂：ボール 1 個につき 2 個増える）／ P（貫通：約 5 秒、ブロックをすり抜けてダメージ）／ M（パドルから 5 個を扇状に発射）。ブロックを壊すと 60% の確率でランダムに落ちる
- **3 ボールの扇状発射**：Space で 3 個が扇状に飛び出す。ボールは最大 60 個
- **コンボと FEVER**：連続破壊でコンボ増加、10 コンボごとにスコア +1 倍。20 コンボで FEVER（虹色発光・背景の脈動・BGM の速度アップ・「FEVER!!」がドンと出て脈打つ）。ボールが全部なくなるか、約 3 秒壊さないとリセット（途切れるまで COMBO が薄くなる）。ステージ 1 を普通に遊んでも FEVER に届く
- **複数ステージ**：Level1（かんたん）／ Level2（壊れないブロックあり）／ Level3（Hp 最大 5、壊れないブロックあり）。最後は「ALL CLEAR!!」からタイトルへ戻る
- **ステージが進むほど速い**：ボール 1600 → 1800 → 2000、パドル 2000、アイテムの落下 900。進行を速くするため、演出やクリア後の待ち時間も短くしてある（ステージ開始 1.2 秒、クリア後 0.5 秒で次へ）
- **スコア**：ブロックに当たると +10、壊すと +100 × 倍率、アイテムで +50。ステージをまたいで持ち越す。残りボールの持ち越しは最大 30 個
- **タイトル画面**：「BREAKOUT」と点滅する「Push SPACE」。Space ですぐ開始

**見た目・演出（エフェクト）**
- **ネオン調のデザイン**：暗い背景に、下へ流れる格子、水色の壁、発光するマテリアル（`BlockMaterial` の `BaseColor` × `Glow`）。ポストプロセスで強いブルームとビネット
- **ボールの残像**（トレイル）と、跳ね返るたびの**火花**。パドルは当たるとぷるんと潰れて白く光る
- **ブロックが壊れる**：白いフラッシュ → 色つきのかけらが飛び散る → 外へ広がる衝撃波のリング → 「+100」のポップアップが浮かぶ。画面が揺れ、色収差がパルスし、一瞬だけヒットストップ（時間が止まりかける）
- **コンボ表示**：右側の「COMBO xN」が増えるたびにぽんと弾み、コンボが多いほど大きくなる。FEVER 中は画面上部の中央に、白と虹色を行き来する大きな縁取りつきの「FEVER!!」（始まった瞬間は大きく出て、すぐ落ち着いて脈打つ）。スコアには倍率（x2 など）を表示
- **ステージ開始**：「STAGE n」がドンと出て、「READY」、「GO!!」
- **クリア**：「GameClear!!」がズームバウンスし、紙吹雪・揺れ・色収差。最終ステージは「ALL CLEAR!!」と連続の紙吹雪（下に「Push SPACE to Next Stage」を点滅表示）
- **GameOver**：ポップ・揺れ・白から赤へ・Push SPACE の点滅に加えて、画面が暗く色が抜ける
- **効果音**：ヒット音は毎回ピッチが変わる。ブロック破壊音はコンボが増えるほど高くなる。ほかにアイテム取得・発射・FEVER 開始・クリア・ゲームオーバー。BGM はテンポ 150 の速いチップチューン（`Tools/gen_sounds.py` で自作）
- 画面表示：左上に `LeftBall`、上中央に `STAGE`、右上に `SCORE`

## 技術メモ

- C++ 17、UE 5.8、Enhanced Input。ウィジェットは Widget Blueprint を使わず、すべて C++（`RebuildWidget` と `NativeTick`）で作っている
- 移動はすべて Tick での Sweep 移動（物理は使わない）。Ball は `AddActorWorldOffset(..., Sweep=true, &Hit)` で当たりを取り、`Bounce` の中で Block・Paddle を直接呼ぶ（Hit イベントに頼らない）
- 主なクラス：`ABreakoutPaddle` / `ABreakoutBall` / `ABreakoutBlock` / `ABreakoutGameManager` / `AMissArea` / `ABreakoutAddBallItem` / `ABreakoutDebris`（かけら・火花・紙吹雪）/ `ABreakoutPopup`（+100 の文字）/ `UBreakoutGameInstance` / `ABreakoutTitleManager` と、`UBreakout*Widget`（Clear / GameOver / GameInfo / Title）。スライドの名前の Blueprint（`Paddle` `Ball` `Block` `GameManager` `AddBallItem` `BreakoutGame` `BreakoutGameInstance`）はこれらの子クラス
- コリジョン：オブジェクトチャンネル `Item` / `Ball` / `MissArea` とプリセットを `Config/DefaultEngine.ini` に定義。ボール同士・ボールとアイテムはぶつからない。ブロックは `WorldDynamic`、壁は `WorldStatic` にして、貫通ボールがブロックだけをすり抜けるようにしている
- スライドとの違い：レベルブループリントの代わりに、GameManager の BeginPlay でウィジェットを作る／タイトルは `ABreakoutTitleManager` が Space を受ける。BGM は SoundCue ではなく、ループ設定した wav を `SpawnSound2D` で再生。アイテムの落下率はスライドの 0.5 ではなく 0.6
- `Tools/gen_content.py`：エディタの Python で Blueprint・マテリアル・サウンド取り込み・入力アセット・全レベルを作り直す（`Tools/run_python.ps1` で実行）。ステージの配置は先頭の `STAGES`（数字 = Hp、`#` = 壊れないブロック）
- `Tools/gen_sounds.py`：標準ライブラリだけで BGM と効果音の wav を生成（`py -I Tools/gen_sounds.py`、出力は `Tools/SourceAudio`）
- 録画用の起動オプション（配布版では使わない）：`-autoplay`（パドルの自動操作）、`-automiss`（わざとミスする）、`-autoseed=N`（自動操作とアイテム抽選の乱数を固定）、`-startballs=N`（最初の残りボール数）、`-noitems`（アイテムを落とさない）、`-perflog`（フレームレートのログ）、`-uiframes`（UI も写る連番画像を保存）、`-debugnormals`（パドル天面の法線の矢印）。`Tools/record.ps1` と `Tools/mkgif.sh` で GIF にしている
- パッケージ化：`Config/DefaultGame.ini` に Shipping・配布用・フルリビルド・エディタコンテンツ除外・クックするマップ（TitleLevel / Level1〜3）を設定。`Tools/package.ps1` が `RunUAT BuildCookRun` を呼ぶ
- ビルド（エディタ）：`build.ps1`。PowerShell のスクリプトは PowerShell 7（`pwsh`）で実行する

## 開発の流れ（ブランチ）

`01_01`（Paddle と Ball）→ `01_02`（Block・GameManager・クリア/ゲームオーバー）→ `01_03`（リスポーン・残ボール・リセット）→ `01_04`（Hp・色・音・次のレベル・持ち越し）→ `01_05`（狙い撃ち）→ `01_06`（アイテム・コリジョン）→ `01_07`（タイトル）→ `評価課題01`（仕上げ・アレンジ・パッケージ化）。
