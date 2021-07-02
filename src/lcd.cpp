#include "lcd.h"

static int scaline_counter;
static uint8_t status_lcd;
static uint8_t mode;
static uint8_t scrolly, scrollx;
static uint8_t windowy, windowx;
static uint8_t lcdcontrol;
static uint16_t background_memory;
// スクリーンデータ(M5Stack表示用)
static uint16_t *screen;
static uint8_t currentLine, currentMode;

// LCD初期化
void lcd_init() {
    scaline_counter = 456;
    screen = (uint16_t *)calloc(GAMEBOY_HEIGHT*GAMEBOY_WIDTH,sizeof(uint16_t));
}
// LCDステーテス確認
void lcd_status() {
    status_lcd = get_memory_value(0xFF41);
    // LCDの表示が無効かチェック
    if ((get_memory_value(0xFF40) & 0x80) != 0x80) {
        // 転送中のデータの、垂直方向の座標を0にする
        set_lcd_memory(0xFF44, 0);
        scaline_counter = 456;
        status_lcd &= 252;
        status_lcd = ~(status_lcd & (1 << 0));
        status_lcd = ~(status_lcd & (1 << 1));
        set_memory_value(0xFF41, status_lcd);
        return;
    }
    // 現在のLCDに転送中のデータの、垂直方向の座標取得
    currentLine = get_memory_value(0xFF44);
    // モードフラグの取得
    currentMode = status_lcd & 0x3;
    bool reqInt = false;
    // モード1の場合(V-Blank 期間中)
    if (currentLine >= 144) {
        mode = 1;
        status_lcd = status_lcd | (1 << 0);
        status_lcd = ~(status_lcd & (1 << 1));
        reqInt = (((status_lcd) & (1 << 4)) > 0);
    }
    else {
        // モード2(OAM-RAM 読み込み中)
        if (scaline_counter >= 376) {
            mode = 2;
            status_lcd = status_lcd | (1 << 1);
            status_lcd = ~(status_lcd & (1 << 0));
            reqInt = (((status_lcd) & (1 << (5))) > 0);
        }
        // モード3(LCD ドライバへのデータ転送中)
        else if (scaline_counter >= 204) {
            mode = 3;
            status_lcd = status_lcd | (1 << 0);
            status_lcd = status_lcd | (1 << 1);
        }
        // モード0(H-Blank 期間中)
        else {
            mode = 0;
            status_lcd = ~(status_lcd & (1 << 0));
            status_lcd = ~(status_lcd & (1 << 1));
            reqInt = (((status_lcd) & (1 << (3))) > 0);
        }
    }
    // LYC と LY が一致した時、LCDステータスレジスタの2ビット目をセット
    if (currentLine == get_memory_value(0xFF45)) {
        status_lcd = status_lcd | (1 << 2);
    }
    else {
        status_lcd = ~(status_lcd & (1 << 2));
    }
    // LCDステータスレジスタのセット
    set_memory_value(0xFF41, status_lcd);
}
// グラフィックの更新
void update_graphics(int cycle) {
    // LCDステータス更新
    lcd_status();
    // LCDが有効かチェック
    if ((((get_memory_value(0xFF40)) & (1 << 7)) > 0)) {
        scaline_counter -= cycle;
    }
    else {
        return;
    }
    // LCDの走査は456クロックが必要
    if (scaline_counter <= 0) {
        // 現在転送中のデータの、垂直座標(LY)を取得
        uint16_t currentline = get_memory_value(0xFF44) + 1;
        // 現在転送中のデータの、垂直座標を+1加算して0xFF44番地にセット
        set_lcd_memory(0xFF44, currentline);
        scaline_counter += 456;
        // LYが144(LCDディスプレイ描画可能の最後の位置)
        if (currentline == 144) {
            draw_scanline();
            uint16_t req = get_memory_value(0xFF0F);
            req = req |= 1 << 0;
            set_memory_d16(0xFF0F, req);
        }
        // 垂直座標(LY)が153を超えた場合、垂直座標(LY)を0に戻す
        else if (currentline > 153) {
            set_lcd_memory(0xFF44, 0);
        }
        // LCDの描画可能範囲であれば描画する
        else if (currentline < 144) {
            draw_scanline();
        }
    }
}

// LCDの1ラインの描画
void draw_scanline() {
    // LCDコントローラーレジスタ
    lcdcontrol = get_memory_value(0xFF40);

    if ((lcdcontrol & 0x01) == 0x01){
        // LCDスクロールレジスタ取得
        scrolly = get_memory_value(0xFF42);
        scrollx = get_memory_value(0xFF43);
        // LCDウィンドウレジスタ取得
        windowy = get_memory_value(0xFF4A);
        windowx = get_memory_value(0xFF4B) - 7;

        static uint16_t title_data = 0;
        background_memory = 0;
        static bool using_window = false;
        static bool unsig = true;
        // 現在 LCD ドライバによって転送中のデータの、垂直方向の座標取得
        int offset = get_memory_value(0xFF44)*160;
        // ウィンドウ表示が有効かチェック
        if ((lcdcontrol & 0x20) == 0x20) {
            if (windowy <= get_memory_value(0xFF44)) {
                using_window = true;
            }
        }
        // タイルデータの場所チェック
        if ((lcdcontrol & 0x10) == 0x10) {
            title_data = 0x8000;
        }
        else {
            title_data = 0x8800;
            unsig = false;
        }

        if (!using_window) {
            // 背景タイルマップの場所チェック
            if ((lcdcontrol & 0x08) == 0x08) {
                background_memory = 0x9C00;
            }
            else {
                background_memory = 0x9800;
            }
        }
        else {
            if ((lcdcontrol & 0x40) == 0x40) {
                background_memory = 0x9C00;
            }
            else {
                background_memory = 0x9800;
            }
        }
        static uint8_t y_pos = 0;
        if (!using_window) {
            y_pos = scrolly + get_memory_value(0xFF44);
        }
        else {
            y_pos = get_memory_value(0xFF44) - windowy;
        }

        uint16_t tile_row = ((uint16_t(y_pos / 8)) * 32);
        // LCDの1ライン(160ドット)分ループ
        for (int pixel = 0; pixel < 160; pixel++,offset++) {
            // タイルマップのx座標計算
            uint8_t x_pos = pixel + scrollx;

            if (using_window)
            {
                if (pixel >= windowx)
                {
                    x_pos = pixel - windowx;
                }
            }
            // タイルマップのx座標計算
            uint16_t tile_col = uint16_t(x_pos / 8);
            int16_t tile_num=0;
            // タイルマップのアドレス計算
            uint16_t tileAddrss = background_memory + tile_row + tile_col;
            // タイルマップのアドレスからタイル番号を取得
            if (unsig) {
                tile_num = int16_t(get_memory_value(tileAddrss));
            }
            else {
                tile_num = int16_t(int8_t(get_memory_value(tileAddrss)));
            }

            uint16_t tile_location = title_data;
            //タイル番号からタイルのアドレス計算
            if (unsig){
                tile_location += uint16_t(tile_num * 16);
            }
            else{
                tile_location = uint16_t(int32_t(tile_location) + int32_t((tile_num + 128) * 16));
            }
            // y座標取得
            uint8_t line = y_pos % 8;
            line *= 2;
            // タイルの横一列のドットデータ取得
            uint8_t data1 = get_memory_value(tile_location + uint16_t(line));
            uint8_t data2 = get_memory_value(tile_location + uint16_t(line) + 1);
            // ドットのx座標
            int color_bit = int(x_pos % 8);
            color_bit -= 7;
            color_bit *= -1;
            // カラー番号計算
            uint8_t color_num = (data2 >> color_bit) & 1;
            color_num = (color_num << 1);
            color_num = color_num | (data1 >> color_bit) & 1;
            // カラーパレットを使って色を取得する
            int color = get_color(color_num, 0xFF47);
            // 座標チェック
            int finary = int(get_memory_value(0xFF44));
            if ((finary < 0) || (finary > 143) || (pixel < 0) || (pixel > 159)) {
                continue;
            }
            // M5Stack描画用screenにカラーを入れる
            screen[offset] = color;
        }
    }
}
int get_color(uint8_t color_num, uint16_t address) {
    uint8_t palette = get_memory_value(address);
    uint8_t hi, lo;
    // カラーパレットのどの位置か
    switch (color_num) {
    // 白
    case 0:
        hi = 1;
        lo = 0;
        break;
    // 薄いグレー
    case 1:
        hi = 3;
        lo = 2;
        break;
    // 濃いグレー
    case 2:
        hi = 5;
        lo = 4;
        break;
    //　黒
    case 3:
        hi = 7;
        lo = 6;
        break;
    default:
        hi = 1;
        lo = 0;
        break;
    }
    //パレットから色を決める
    uint8_t color = (((palette >> hi) & 1) << 1);
    color = color | ((palette >> lo) & 1);
    switch (color) {
    case 0:
        return 0xFFFF;
        break;
    case 1:
        return 0xEF;
        break;
    case 2:
        return 0xE0;
        break;
    case 3:
        return 0x0000;
        break;
    default:
        return 0;
        break;
    }
    return 0;
}
// M5StackにLCD表示の内容をbitmapで表示
void drawscreen() {
    M5.Lcd.drawBitmap(CENTER_X, CENTER_Y, GAMEBOY_WIDTH, GAMEBOY_HEIGHT, (uint8_t*)screen);
}