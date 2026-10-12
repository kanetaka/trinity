# trinity

リアルタイムレンダリングのための高速3Dレンダリングエンジン（C++ / Vulkan / ImGui / CMake）。

## 常に従うルール
- [rules/project.md](rules/project.md) のコード規約・命名規則・禁止事項に必ず従うこと

## スキル
- `architecture`: ファイルの作成・移動・削除、クラス・構造体・関数・モジュールの定義時に必ず参照する
- `graphics`: 座標・行列・クォータニオン・カメラ・シェーダー等、計算を伴う処理の実装時に参照する
- `trinity-review`: コードレビュー時に参照する

## ビルド
- 依存ライブラリはvcpkgで管理（`vcpkg.json`）
- 構成: `cmake --preset default`（出力先は `build/`）
- ビルド: `cmake --build build --config Debug`
- シェーダーはSlang（`asset/shader/**/*.slang`）で、`slangc` によりビルド時にSPIR-Vへコンパイルされる
