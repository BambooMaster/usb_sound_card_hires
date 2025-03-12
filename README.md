# これなに
Raspberry Pi Picoを使ったマスタークロック付きのi2sを出力するusbスピーカーです。pico-playgroundのusb_sound_cardをベースにしています。

# build
vscodeの拡張機能(Raspberry Pi Pico)でインポートし、ビルドしてください。
pico-sdkは2.1.0を使っています。

# i2s
https://github.com/BambooMaster/pico-i2s-pio.git を使っています。

# i2sのピン
|name|pin|
|----|---|
|DATA|GPIO18|
|LRCLK|GPIO19|
|BCLK|GPIO20|
|MCLK|GPIO21|

# 対応機種
Windows11とAndroid(Pixel6a Android1５)で動作確認をしています。