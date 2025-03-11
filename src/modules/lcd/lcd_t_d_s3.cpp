#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <span>
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef ESP_PLATFORM
#define ESP_PLATFORM
#endif

#include "lvgl.h" 

#define LCD_H_RES 320   // Горизонтальное разрешение
#define LCD_V_RES 170   // Вертикальное разрешение
#define LCD_BUS_WIDTH 8 // Ширина шины I8080

// Пины для T-Display-S3 (взято из документации LilyGo)
#define PIN_NUM_BK_LIGHT 38 // Подсветка
#define PIN_NUM_WR 8        // WR
#define PIN_NUM_RD 9        // RD ???
#define PIN_NUM_CS 6        // CS
#define PIN_NUM_DC 7        // DC
#define PIN_NUM_RST 5       // RST
#define PIN_NUM_DATA0 39    // D0
#define PIN_NUM_DATA1 40    // D1
#define PIN_NUM_DATA2 41    // D2
#define PIN_NUM_DATA3 42    // D3
#define PIN_NUM_DATA4 45    // D4
#define PIN_NUM_DATA5 46    // D5
#define PIN_NUM_DATA6 47    // D6
#define PIN_NUM_DATA7 48    // D7

// #define LCD_MAX_TRANSFER_BYTES (LCD_H_RES * 10 * sizeof(uint16_t)) // 10 строк = 6400 байт // LCD_H_RES * LCD_V_RES * sizeof(uint16_t)
// #define LCD_MAX_TRANSFER_BYTES (LCD_H_RES * LCD_V_RES * sizeof(uint16_t)) // 10 строк = 6400 байт //
#define LVGL_LCD_BUF_SIZE (LCD_H_RES * LCD_V_RES)
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ (6528000) //(10 * 1000 * 1000)

static const char *TAG = "T-Display-S3";

// ESP
static esp_lcd_panel_handle_t panel_handle = NULL;
static esp_lcd_panel_io_handle_t io_handle = NULL;

// LVGL
static lv_display_t *disp; //
static lv_color_t *lv_disp_buf;

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
// Full buffer
//#define DRAW_BUF_SIZE (LCD_H_RES * LCD_V_RES * (LV_COLOR_DEPTH / 8))
#define DRAW_BUF_SIZE (LCD_H_RES * 5  * (LV_COLOR_DEPTH / 8)) // 5 can change
//uint32_t draw_buf[DRAW_BUF_SIZE / 4];
uint32_t *draw_buf1;//[DRAW_BUF_SIZE / 4];
uint32_t *draw_buf2;//[DRAW_BUF_SIZE / 4];

#if LV_USE_LOG != 0
void my_print(lv_log_level_t level, const char *buf)
{
    LV_UNUSED(level);
    ESP_LOGI("LVGL ***", "%s", buf);
}
#endif

// Функция рисования линии (алгоритм Брезенхэма)
void draw_line(int x0, int y0, int x1, int y1, uint16_t color)
{
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        // Устанавливаем пиксель
        esp_lcd_panel_draw_bitmap(panel_handle, x0, y0, x0 + 1, y0 + 1, &color);

        if (x0 == x1 && y0 == y1)
            break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

/* LVGL calls it when a rendered image needs to copied to the display*/
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    /*Copy `px map` to the `area`*/

    /*For example ("my_..." functions needs to be implemented by you)
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);

    my_set_window(area->x1, area->y1, w, h);
    my_draw_bitmaps(px_map, w * h);
     */

    int x1 = area->x1;
    int y1 = area->y1;
    int x2 = area->x2;
    int y2 = area->y2;

    ESP_LOGI("LVGL ***", " my_disp_flush xy xy %d-%d %d-%d", x1, y1, x2, y2);

    // Отправляем блок пикселей на дисплей
    esp_lcd_panel_draw_bitmap(panel_handle, x1, y1, x2 + 1, y2 + 1, px_map);

    /*Call it to tell LVGL you are ready*/
    lv_display_flush_ready(disp);
}

void lv_task(void *pvParameter)
{
    while (1)
    {
        lv_timer_handler();            // Обработка LVGL
        vTaskDelay(pdMS_TO_TICKS(10)); // Задержка 10 мс (оптимально)
    }
}

void lcd_main(void)
{

    ESP_LOGI("LCD----------", "Init");

    // Настройка подсветки
    gpio_config_t bk_gpio_config = {
        .pin_bit_mask = (1ULL << PIN_NUM_BK_LIGHT) | (1ULL << PIN_NUM_RD),
        .mode = GPIO_MODE_OUTPUT};

    gpio_config(&bk_gpio_config);
    gpio_set_level((gpio_num_t)PIN_NUM_BK_LIGHT, 1); // Включить подсветку
    gpio_set_level((gpio_num_t)PIN_NUM_RD, 1);       // RD =1

    // Настройка шины I8080
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = PIN_NUM_DC,
        .wr_gpio_num = PIN_NUM_WR,
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .data_gpio_nums = {
            PIN_NUM_DATA0, PIN_NUM_DATA1, PIN_NUM_DATA2, PIN_NUM_DATA3,
            PIN_NUM_DATA4, PIN_NUM_DATA5, PIN_NUM_DATA6, PIN_NUM_DATA7},
        .bus_width = LCD_BUS_WIDTH,
        .max_transfer_bytes = LVGL_LCD_BUF_SIZE * sizeof(uint16_t),
    };

    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = PIN_NUM_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ, // 5000000, // Частота 10 МГц// 10000000, // Частота 10 МГц
        .trans_queue_depth = 20,
        //.on_color_trans_done = example_notify_lvgl_flush_ready,
        //.user_ctx = &disp_drv,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));

    // Инициализация ST7789
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_NUM_RST,
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, true));       //?
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, true, false)); // ? Настройка ориентации
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));  // ?
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    // the gap is LCD panel specific, even panels with the same driver IC, can
    // have different gap value
    esp_lcd_panel_set_gap(panel_handle, 0, 35);

    ESP_LOGI("LCD----------", "Pre heap_caps_calloc"); 

    // Очистка экрана (черный фон)
    uint16_t *buffer = (uint16_t *)heap_caps_calloc(LVGL_LCD_BUF_SIZE, sizeof(uint16_t), MALLOC_CAP_DMA);
    std::span<uint16_t> s(buffer, LVGL_LCD_BUF_SIZE);

    ESP_LOGI("LCD----------", "Pre fill");

    //std::fill(s.begin(), s.end(), 0x002E);
    //ESP_LOGI(TAG, "fill %d %d %x", s.size(), s.size_bytes(), s[0]);

    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_H_RES, LCD_V_RES, buffer));
    free(buffer);

    // Рисуем линию (красная линия от (10,10) до (100,100))
    draw_line(40, 40, 150, 150, 0xF800); // 0xF800 - красный цвет в формате RGB565

    ESP_LOGI(TAG, "Line drawn successfully");

    lv_init();

    /* register print function for debugging */
#if LV_USE_LOG != 0
    // lv_log_register_print_cb(my_print);

#endif

    disp = lv_display_create(LCD_H_RES, LCD_V_RES);

    lv_display_set_flush_cb(disp, my_disp_flush);

    draw_buf1 = (uint32_t *)heap_caps_calloc(DRAW_BUF_SIZE, sizeof(uint8_t), MALLOC_CAP_DMA);
    draw_buf2 = (uint32_t *)heap_caps_calloc(DRAW_BUF_SIZE, sizeof(uint8_t), MALLOC_CAP_DMA);

    lv_display_set_buffers(disp, draw_buf1, draw_buf2,DRAW_BUF_SIZE /*sizeof(draw_buf)*/, LV_DISPLAY_RENDER_MODE_PARTIAL);




    lv_obj_t *label = lv_label_create(lv_screen_active());
    
    static lv_style_t style;
    lv_style_init(&style);
    //lv_style_set_text_font(&style, &lv_font_montserrat_24);
    lv_style_set_text_font(&style, &lv_font_unscii_16);
    lv_obj_add_style(label, &style, 0);
    
    lv_label_set_text(label, "Hello KOE, I'm LVGL!");

    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    xTaskCreate(lv_task, "LVGL", 4096, NULL, 5, NULL);

    // lv_demo_transform();

    // lv_disp_buf = (lv_color_t *)heap_caps_malloc(LVGL_LCD_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    //
    // lv_display_set_buffers(&disp_buf, lv_disp_buf, NULL, LVGL_LCD_BUF_SIZE);
    // lv_display_set_draw_buffers(&disp_buf, lv_disp_buf, NULL, LVGL_LCD_BUF_SIZE);

    ESP_LOGI("LCD----------", "+");
}

/*


static void lvgl_timer_callback(void *arg) {
    lv_timer_handler();
}

void init_lvgl_timer() {
    const esp_timer_create_args_t timer_args = {
        .callback = &lvgl_timer_callback,
        .name = "lvgl_timer"
    };

    esp_timer_handle_t lvgl_timer;
    esp_timer_create(&timer_args, &lvgl_timer);
    esp_timer_start_periodic(lvgl_timer, 10 * 1000);  // 10 мс (в микросекундах)
}

*/