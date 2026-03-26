#include QMK_KEYBOARD_H
#include "quantum.h"

// カスタムキーコード（Kb 16〜）
enum custom_keycodes {
    TB_VOL = KEYBALL_SAFE_RANGE, // Kb 16: 押している間トラックボールが音量操作
    TB_ZOOM,                     // Kb 17: 押している間トラックボールがズーム操作
    TB_BRIGHT,                   // Kb 18: 押している間トラックボールが明るさ操作
};

// トラックボールモード
typedef enum {
    TB_MODE_DEFAULT,
    TB_MODE_VOLUME,
    TB_MODE_ZOOM,
    TB_MODE_BRIGHT,
} tb_mode_t;

static tb_mode_t tb_mode = TB_MODE_DEFAULT;

static int16_t tb_accumulated = 0;
#define TB_THRESHOLD 10

static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_universal(
    KC_ESC   , KC_Q     , KC_W     , KC_E     , KC_R     , KC_T     ,                                        KC_Y     , KC_U     , KC_I     , KC_O     , KC_P     , KC_DEL   ,
    KC_TAB   , KC_A     , KC_S     , KC_D     , KC_F     , KC_G     ,                                        KC_H     , KC_J     , KC_K     , KC_L     , KC_SCLN  , S(KC_7)  ,
    KC_LSFT  , KC_Z     , KC_X     , KC_C     , KC_V     , KC_B     ,                                        KC_N     , KC_M     , KC_COMM  , KC_DOT   , KC_SLSH  , KC_INT1  ,
              KC_LALT,KC_LGUI,LCTL_T(KC_LNG2),LT(1,KC_SPC),LT(3,KC_LNG1),             KC_BSPC,LT(2,KC_ENT),RCTL_T(KC_LNG2),KC_RALT,KC_PSCR
  ),
  [1] = LAYOUT_universal(
    AML_TO   , KC_F1    , KC_F2    , KC_F3    , KC_F4    , KC_F5    ,                                        KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10   , KC_F11   ,
    _______  , _______  , _______  , KC_UP    , KC_ENT   , KC_DEL   ,                                        KC_PGUP  , KC_BTN1  , KC_UP    , KC_BTN2  , KC_BTN3  , KC_F12   ,
    _______  , _______  , KC_LEFT  , KC_DOWN  , KC_RGHT  , KC_BSPC  ,                                        KC_PGDN  , KC_LEFT  , KC_DOWN  , KC_RGHT  , _______  , _______  ,
              _______  , _______  , _______   ,            _______  , _______  ,             _______  , _______  , _______  , _______  , _______
  ),
  [2] = LAYOUT_universal(
    _______  ,S(KC_QUOT), KC_7     , KC_8     , KC_9     , S(KC_8)  ,                                        S(KC_9)  , S(KC_1)  , S(KC_6)  , KC_LBRC  , S(KC_4)  , _______  ,
    _______  ,S(KC_SCLN), KC_4     , KC_5     , KC_6     , KC_RBRC  ,                                        KC_NUHS  , KC_MINS  , S(KC_EQL), S(KC_3)  , KC_QUOT  , S(KC_2)  ,
    _______  ,S(KC_MINS), KC_1     , KC_2     , KC_3     ,S(KC_RBRC),                                       S(KC_NUHS),S(KC_INT1), KC_EQL   ,S(KC_LBRC),S(KC_SLSH),S(KC_INT3),
              KC_0     , KC_DOT   , _______   ,            _______  , _______  ,             KC_DEL   , _______  , _______  , _______  , _______
  ),
  [3] = LAYOUT_universal(
    _______  , AML_TO   , AML_I50  , AML_D50  , _______  , _______  ,                                        _______  , _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  , SCRL_DVI ,                                        _______  , CPI_D100 , CPI_I100 , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  , SCRL_DVD ,                                        CPI_D1K  , _______  , _______  , CPI_I1K  , _______  , KBC_SAVE ,
              QK_BOOT  , KBC_RST  , _______   ,            _______  , _______  ,             _______  , _______  , _______  , KBC_RST  , QK_BOOT
  ),
  [4] = LAYOUT_universal(
    _______  , _______  , _______  , _______  , _______  , _______  ,                                        _______  , _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  , _______  ,                                        _______  , _______  , _______  , _______  , _______  , _______  ,
    _______  , _______  , _______  , _______  , _______  , _______  ,                                        _______  , _______  , _______  , _______  , _______  , _______  ,
                  _______  , _______  , _______  ,        _______  , _______  ,                   _______  , _______  , _______       , _______  , _______
  ),
};
// clang-format on

// トラックボールモード切り替え
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case TB_VOL:
            tb_mode = record->event.pressed ? TB_MODE_VOLUME : TB_MODE_DEFAULT;
            return false;
        case TB_ZOOM:
            tb_mode = record->event.pressed ? TB_MODE_ZOOM : TB_MODE_DEFAULT;
            return false;
        case TB_BRIGHT:
            tb_mode = record->event.pressed ? TB_MODE_BRIGHT : TB_MODE_DEFAULT;
            return false;
        default:
            return true;
    }
}

// pointing_device_task_user
report_mouse_t pointing_device_task_user(report_mouse_t report) {
    if (tb_mode != TB_MODE_DEFAULT) {
        tb_accumulated += report.x;
        if (tb_accumulated > TB_THRESHOLD) {
            switch (tb_mode) {
                case TB_MODE_VOLUME: tap_code(KC_VOLU); break;
                case TB_MODE_ZOOM:   tap_code16(C(KC_EQL)); break;
                case TB_MODE_BRIGHT: tap_code(KC_BRMU); break;
                default: break;
            }
            tb_accumulated = 0;
        } else if (tb_accumulated < -TB_THRESHOLD) {
            switch (tb_mode) {
                case TB_MODE_VOLUME: tap_code(KC_VOLD); break;
                case TB_MODE_ZOOM:   tap_code16(C(KC_MINS)); break;
                case TB_MODE_BRIGHT: tap_code(KC_BRMD); break;
                default: break;
            }
            tb_accumulated = 0;
        }
        report.x = 0;
        report.y = 0;
    }
    return report;
}

// keyball_on_apply_motion_to_mouse_move
void keyball_on_apply_motion_to_mouse_move(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
    r->x = clip2int8(m->y);
    r->y = clip2int8(m->x);
    if (is_left) {
        r->x = -r->x;
        r->y = -r->y;
    }
    m->x = 0;
    m->y = 0;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    keyball_set_scroll_mode(get_highest_layer(state) == 3);
    return state;
}

#ifdef OLED_ENABLE
#    include "lib/oledkit/oledkit.h"
void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif
