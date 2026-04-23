---
name: architecture
description: アーキテクチャに関するスキル
    ファイルの作成、移動、削除する際は、必ずこのスキルを呼び出すこと
    クラス、構造体、関数、モジュールを定義する際は、必ずこのスキルを呼び出すこと
---

# アーキテクチャ
- データ指向
- ECSアーキテクチャを採用する
- Entity、Component、Systemの3層構造にする
- Entityは、レンダラ内のあらゆる実体のIDのみを持つ
- Entityの特徴は、データもロジックも持たない
- Entityの役割は、コンポーネントを紐づけるためのインデックス
- Componentは、エンティティが持つデータの集まり
- Componentの特徴は、ロジックを持たず構造体として定義する
- Componentの役割は、位置、角度、色など、特定の性質を保持する
- Systemは、特定のコンポーネントの組み合わせを持つエンティティ群を一括して処理するロジック
- Systemの特徴は、状態を持たず、毎フレーム実行されるフィルターのような役割を果たす
- Systemの役割は、データの更新を行う
- 変換マトリックスは連続したメモリ領域に配置し、GPUで一括処理できるようにする
- MVPアーキテクチャを採用する
- coreはgeom, io, render, ui, appに依存しないようにする
- geomはio, render, ui, appに依存しないようにする
- ioはrender, ui, appに依存しないようにする
- renderはui, appに依存しないようにする
- uiはappに依存しないようにする
- appはあらゆるモジュールに依存してもよい

# ファイル構成
- asset : 3Dモデルやテクスチャなどのアセットを格納する
- asset/model : 3Dモデルを格納する
- asset/texture : テクスチャを格納する
- asset/shader : シェーダープログラムを格納する
- build : ビルド成果物を格納する。コミットしない
- docs : ドキュメントを格納する
- doc/spec : 仕様書を格納する
- src : ソースコードを格納する
- src/app : アプリケーションのコードを格納する
- src/core : コアライブラリのコードを格納する。Entity, Component, System, Registryなど
- src/geom : 数学ライブラリのコードを格納する
- src/io : 入出力ライブラリのコードを格納する
- src/render : レンダリングのコードを格納する
- src/render/component : 具象コンポーネントのコードを格納する
- src/render/pipeline : レンダリングパイプラインのコードを格納する
- src/render/resource : レンダリングリソースのコードを格納する
- src/redner/surface : レンダリングサーフェスのコードを格納する
- src/render/system : レンダリングシステムのコードを格納する
- src/ui : UIのコードを格納する
- tool : ツールプログラムのコードを格納する