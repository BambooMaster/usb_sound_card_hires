# これなに
Raspberry Pi Picoを使ったマスタークロック付きのi2sを出力するusbスピーカーです。pico-playgroundのusb_sound_cardをベースにしています。

# build
vscodeの拡張機能(Raspberry Pi Pico)でインポートし、ビルドしてください。
pico-sdkは2.1.1を使っています。

# i2s
https://github.com/BambooMaster/pico-i2s-pio.git を使っています。

|name|pin|
|----|---|
|DATA|GPIO2|
|LRCLK|GPIO3|
|BCLK|GPIO4|
|MCLK|GPIO21|

# 対応機種
Windows11とAndroid(Pixel6a Android15)で動作確認をしています。