# usb_sound_card_hires
Raspberry Pi Pico(RP2040, RP2350)を使ったマスタークロック付きのi2sを出力するusbスピーカーです。pico-playgroundの[usb_sound_card](https://github.com/raspberrypi/pico-playground/tree/master/apps/usb_sound_card)をベースにしています。

## build
### vscodeの拡張機能を使う場合
```
https://github.com/BambooMaster/usb_sound_card_hires.git
cd usb_sound_card_hires
git submodule update --init
```
を実行した後、vscodeの拡張機能(Raspberry Pi Pico)でインポートし、ビルドしてください。

### vscodeの拡張機能を使わない場合
```
https://github.com/BambooMaster/usb_sound_card_hires.git
cd usb_sound_card_hires
git submodule update --init
mkdir build && cd build
cmke .. && make -j4
```

## i2s
[pico-i2s-pio](https://github.com/BambooMaster/pico-i2s-pio.git)を使っています。RP2040/RP2350のシステムクロックをMCLKの整数倍に設定し、pioのフラクショナル分周を使わないlowジッタモードを搭載しています。

|name|pin|
|----|---|
|DATA|GPIO18|
|LRCLK|GPIO19|
|BCLK|GPIO20|
|MCLK|GPIO21|

## 対応機種
Windows11とAndroid(Pixel6a Android15)で動作確認をしています。