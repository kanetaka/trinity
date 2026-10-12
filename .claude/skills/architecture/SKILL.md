---
name: architecture
description: trinityのアーキテクチャ（モジュール構成・依存方向・ディレクトリ構成・Object/Component設計）に関するスキル。ファイルの作成・移動・削除を行う際、およびクラス・構造体・関数・モジュールを定義する際は必ずこのスキルを使用すること。
---

# アーキテクチャ
- オブジェクト指向 + コンポーネント指向。
- Objectは名前、階層（親子）、トランスフォーム、コンポーネントを持つ。
- 振る舞いはObjectに付与するコンポーネントとして実装する。コンポーネントはIComponentを実装する。
- 継承は原則インターフェイス（純粋仮想関数のみを持つ抽象クラス）の実装のみで行う。具象クラス同士の実装継承は避け、コンポジション（コンポーネントの合成）で再利用する。
- モジュールはcore,geometry,graphics,scene,renderer,appの6つとする。
- coreはgeometry,graphics,scene,renderer,appに依存しない。
- geometryはgraphics,scene,renderer,appに依存しない。
- graphicsはgeometry,scene,renderer,appに依存しない。
- sceneはrenderer,appに依存しない。
- rendererはappに依存しない。

# ファイル構成
- asset:3Dモデルやテクスチャなどのアセットを格納
- asset/model:3Dモデルを格納
- asset/shader:シェーダープログラム（Slang）を格納
- asset/texture:テクスチャを格納
- build:ビルド成果物を格納
- docs:ドキュメントを格納
- docs/spec:仕様書を格納
- src:ソースコードを格納
- src/app:アプリケーションのコードを格納
- src/core:コアライブラリのコードを格納。AssetPathなど
- src/geometry:数学ライブラリのコードを格納
- src/graphics:レンダリング(Vulkan)のコードを格納
- src/graphics/pipeline:レンダリングパイプラインのコードを格納
- src/graphics/resource:レンダリングリソースのコードを格納
- src/graphics/surface:レンダリングサーフェスのコードを格納
- src/scene:シーングラフ,オブジェクト,コンポーネントのコードを格納
- src/scene/component:コンポーネントのコードを格納
- src/scene/io:入出力ライブラリのコードを格納
- src/renderer:レンダラーのコードを格納
- src/renderer/ui:UIのコードを格納
- tools:ツールプログラムのコードを格納

# ファイル追加・削除時の注意
- ソースファイルを追加・削除した場合は `CMakeLists.txt` のソース一覧も更新する
- シェーダーを追加した場合は `CMakeLists.txt` のslangcコンパイル設定も追加する

# 禁止事項
- シングルトンは使わない
- 上記の依存方向に反するincludeを追加しない
