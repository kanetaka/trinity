---
name: architecture
description: アーキテクチャに関するスキル
    ファイルの作成、移動、削除する際は、必ずこのスキルを呼び出すこと
    クラス、構造体、関数、モジュールを定義する際は、必ずこのスキルを呼び出すこと
---

# アーキテクチャ
- オブジェクト指向 + コンポーネント指向。
- Objectは名前、階層（親子）、トランスフォーム、コンポーネントを持つ。
- 振る舞いはObjectに付与するコンポーネントとして実装する。コンポーネントはIComponentを実装する。
- 継承は原則インターフェイス（純粋仮想関数のみを持つ抽象クラス）の実装のみで行う。具象クラス同士の実装継承は避け、コンポジション（コンポーネントの合成）で再利用する。
- モジュールはcore,geometry,graphics,scene,renderer,ui,appの7つとする。
- coreはgeometry,graphics,scene,renderer,ui,appに依存しない。
- geometryはgraphics,scene,renderer,ui,appに依存しない。
- graphicsはgeometry,scene,renderer,ui,appに依存しない。
- sceneはrenderer,ui,appに依存しない。
- rendererはui,appに依存しない。
- uiはappに依存しない。

# ファイル構成
- asset:3Dモデルやテクスチャなどのアセットを格納
- asset/model:3Dモデルを格納
- asset/shader:シェーダープログラムを格納
- asset/texture:テクスチャを格納
- build:ビルド成果物を格納
- docs:ドキュメントを格納
- doc/spec:仕様書を格納
- src:ソースコードを格納
- src/app:アプリケーションのコードを格納
- src/core:コアライブラリのコードを格納。AssetPathなど
- src/geometry:数学ライブラリのコードを格納
- src/graphics:レンダリング(Vulkan)のコードを格納
- src/graphics/pipeline:レンダリングパイプラインのコードを格納
- src/graphics/resource:レンダリングリソースのコードを格納
- src/graphics/surface:レンダリングサーフェスのコードを格納
- src/scene:シーングラフ,オブジェクト,コンポーネントのコードを格納
- src/scene/io:入出力ライブラリのコードを格納
- src/renderer:レンダラーのコードを格納
- src/ui:UIのコードを格納
- tool:ツールプログラムのコードを格納
