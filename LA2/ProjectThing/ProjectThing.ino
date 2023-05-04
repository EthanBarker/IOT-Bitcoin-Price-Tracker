// ProjectThing
#include "unPhone.h"
#include <lvgl.h>
#include <TFT_eSPI.h>

unPhone u = unPhone();
long my_mapper(long, long, long, long, long);

static const uint16_t screenWidth  = 480;
static const uint16_t screenHeight = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight);

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

long my_mapper(long x, long in_min, long in_max, long out_min, long out_max) {
  long probable =
  (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  if(probable < out_min) return out_min;
  if(probable > out_max) return out_max;
  return probable;
}

void my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data) {
  uint16_t touchX, touchY;
  bool touched = u.tsp->touched();

  if(!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;

    TS_Point p(-1, -1, -1);
    p = u.tsp->getPoint();

    long xMin = 320;
    long xMax = 3945;
    long yMin = 420;
    long yMax = 3915;

    long xscld = my_mapper((long)p.x, xMin, xMax, 0, (long)screenWidth);
    long yscld = (long)screenHeight - my_mapper((long)p.y, yMin, yMax, 0, (long)screenHeight);
    touchX = (uint16_t)xscld;
    touchY = (uint16_t)yscld;

    data->point.x = touchX;
    data->point.y = touchY;
  }
}

void setup() {
  Serial.begin(115200);

  u.begin();
  u.tftp = (void *)&tft;
  u.tsp->setRotation(1);
  u.backlight(true);

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino +=
      String('V') + lv_version_major() + "." +
      lv_version_minor() + "." + lv_version_patch();

  Serial.println(LVGL_Arduino);

  lv_init();
  tft.begin();
  tft.setRotation(1);

  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  /* Create a blank screen with a white background */
  lv_obj_t *blank_screen = lv_obj_create(lv_scr_act());
  lv_obj_set_size(blank_screen, screenWidth, screenHeight);
  lv_obj_set_style_bg_color(blank_screen, lv_color_hex(0xFFFFFF), 0);

  // Create a label for displaying "hello"
  lv_obj_t *label = lv_label_create(blank_screen);
  lv_label_set_text(label, "hello");
  lv_obj_center(label); // Align the label to the center of its parent (blank_screen)

  Serial.println("Setup done");
}

void loop() {
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
